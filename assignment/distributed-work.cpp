#include "al/app/al_DistributedApp.hpp"
#include "al/math/al_Random.hpp"
#include "al/ui/al_ControlGUI.hpp"  // gui.draw(g)
#include "al_ext/statedistribution/al_CuttleboneStateSimulationDomain.hpp"

using namespace al;

#include <fstream>
#include <vector>
using namespace std;

const int AgentNum = 1000;

Vec3f rv(float scale = 1.0f) {
  return Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()) * scale;
}

string slurp(string fileName);  // forward declaration

/*Critical: 
  - Pose is larger than it needs to be -> uses doubles and quat
  - We don't draw heading, center, and flockCount -> need for simulating, useless for drawing
*/

struct Agent : Pose {
  Vec3f heading; //heading from POV of agent (sum of all the headings, then divide by the count)
  Vec3f center;  // position in worldspace (sum of all the positions, then divide by the count)
  unsigned flockCount{1}; 
};

Agent agents[AgentNum]; //declare a global agents array

// This is only what we need to draw on the GPU
// Only share the state that needs to be shared for sending
struct DrawableAgent {
  Vec3f position, forward; //agents have a position and a forward
  Vec3f up; //part of orientation -> which way is up?
};

struct SharedState {
    // we need shared state to be contiguous in memory -> can't declare a vector of things because it is a pointer (int) and a size (int)
    // the renderer will interpret this information passed as a place in memory at that address and then it will crash
    // need contiguous memory -> declare a fixed array
    
    DrawableAgent agents[AgentNum];
    float background;
    float size, ratio;
    // everything that is simulated
};

class MyApp : public DistributedAppWithState<SharedState>  {
  Parameter moveRate{"/moveRate", "", 0.35, "", 0.0, 2.0};
  Parameter turnRate{"/turnRate", "", 0.15, "", 0.0, 2.0};
  Parameter localRadius{"/localRadius", "", 0.6, "", 0.01, 0.9};
  Parameter separationDistance{"/separationDistance", "", 0.15, "", 0.01, 0.9};
  Parameter size{"/size", "", 1.0, "", 0.0, 2.0};
  Parameter ratio{"/ratio", "", 1.0, "", 0.0, 2.0};
  ControlGUI gui;

   // You can keep a pointer to the cuttlebone domain
  // This can be useful to ask the domain if it is a sender or receiver
  std::shared_ptr<CuttleboneStateSimulationDomain<SharedState>>
      cuttleboneDomain;

  ShaderProgram shader;
  Mesh mesh;

  int agentNum = 1000;

  void reset() {
    for (int i = 0; i < agentNum; i++) {
      agents[i].pos(rv());
      agents[i].faceToward(rv());
    }
  }

  void onCreate() override {
    cuttleboneDomain =
        CuttleboneStateSimulationDomain<SharedState>::enableCuttlebone(this);
    if (!cuttleboneDomain) {
      std::cerr << "ERROR: Could not start Cuttlebone. Quitting." << std::endl;
      quit();
    }

    // add more GUI here
    gui << moveRate << turnRate << localRadius << separationDistance << size << ratio;
    gui.init();
    navControl().useMouse(false);

    // compile shaders
    shader.compile(slurp("../tetrahedron-vertex.glsl"),
                   slurp("../tetrahedron-fragment.glsl"),
                   slurp("../tetrahedron-geometry.glsl"));

    mesh.primitive(Mesh::POINTS);

    for (int i = 0; i < agentNum; i++) {
      Agent a;
      a.pos(rv());
      a.faceToward(rv());
      agents[i] = a;
      
      mesh.vertex(a.pos());
      mesh.normal(a.uf());
      const Vec3f& up(a.uu());
      mesh.color(up.x, up.y, up.z);
    }
    nav().pos(0, 0, 10);
  }

  void onAnimate(double dt) override {
    if (cuttleboneDomain->isSender()) {
      for (unsigned i = 0; i < agentNum; i++) {
        Vec3f avgHeading(0, 0, 0);
        Vec3f centerPos(0, 0, 0);
        agents[i].flockCount = 0; //reset flock count
        for (unsigned j = 1 + i; j < agentNum; j++) {
            float distance = (agents[j].pos() - agents[i].pos()).mag();
            
            if (distance < localRadius) { 
            //calculate alignment and cohesion vals if flock mates are far enough away from each other
            agents[i].flockCount++; //increase the flockmate count for that specific agent
            if (distance < separationDistance) {
                //cout << "separate" << endl;
                agents[i].pos() -= agents[j].uf().normalize() * turnRate * 0.002;
            } else {
                avgHeading += agents[j].uf();
                centerPos += agents[j].pos();
            }
            } 
        }

        if (agents[i].flockCount > 0) {
            agents[i].heading = avgHeading.normalize()/agents[i].flockCount;
            agents[i].center = centerPos.normalize()/agents[i].flockCount;
        }
      }
      // only once the above loop is done do we have good data on average headings and centers
      // here is where we actually point them in the right direction and move them


      for (unsigned i = 0; i < agentNum; i++) {
          //alignment
          agents[i].faceToward(agents[i].heading.normalize()); // point agents in the direction of their heading
          //cohesion
          agents[i].pos().lerp(agents[i].center.normalize() + agents[i].uf(), moveRate * 0.02);
      }

      // respawn agents if they go too far (MAYBE KEEP)
      for (unsigned i = 0; i < agentNum; i++) {
          if (agents[i].pos().mag() > 1.1) {
              agents[i].pos(rv());
              agents[i].faceToward(rv());
          }
      }

      //copy only happens in primary screen
      for (unsigned i = 0; i < AgentNum; i++) { //Agent array -> DrawableAgent array
        DrawableAgent a;
        a.position = agents[i].pos();
        a.forward = agents[i].uf();
        a.up = agents[i].uu();

        state().agents[i] = a;
      }

      state().background = 0.1;
      state().size = size.get();
      state().ratio = ratio.get();
    } else { }

    
    // visualize the agents, update meshes using DrawableAgent in state (for ALL screens)
    vector<Vec3f>& v(mesh.vertices());
    vector<Vec3f>& n(mesh.normals());
    vector<Color>& c(mesh.colors());
    for (unsigned i = 0; i < agentNum; i++) {
      v[i] = state().agents[i].position;
      n[i] = state().agents[i].forward;
      const Vec3d& up(state().agents[i].up);
      c[i].set(up.x, up.y, up.z);
    }
  }

  bool onKeyDown(const Keyboard& k) override {
    if (k.key() == 'r') {
      reset();
    }
    return true;
  }

  void onDraw(Graphics& g) override {
    g.clear(state().background, state().background, state().background);
    gl::depthTesting(true);  // or g.depthTesting(true);
    gl::blending(true);      // or g.blending(true);
    gl::blendTrans();        // or g.blendModeTrans();
    g.shader(shader);
    g.shader().uniform("size", state().size * 0.03);
    g.shader().uniform("ratio", state().ratio * 0.2);
    g.draw(mesh);

    if (isPrimary()) {
      gui.draw(g);
    }
  }
};



int main() {
  MyApp app;
  app.start();
}

string slurp(string fileName) {
  fstream file(fileName);
  string returnValue = "";
  while (file.good()) {
    string line;
    getline(file, line);
    returnValue += line + "\n";
  }
  return returnValue;
}
