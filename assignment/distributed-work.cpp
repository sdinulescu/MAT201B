#include "al/app/al_DistributedApp.hpp"
#include "al/math/al_Random.hpp"
#include "al/ui/al_ControlGUI.hpp"  // gui.draw(g)

using namespace al;

#include <fstream>
#include <vector>
using namespace std;

Vec3f rv(float scale = 1.0f) {
  return Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()) * scale;
}

string slurp(string fileName);  // forward declaration

struct Agent : Pose {
  Vec3f heading; //heading from POV of agent (sum of all the headings, then divide by the count)
  Vec3f center;  // position in worldspace (sum of all the positions, then divide by the count)
  unsigned flockCount{1}; 
};

struct SharedState {
    // we need shared state to be contiguous in memory -> can't declare a vector of things because it is a pointer (int) and a size (int)
    // the renderer will interpret this information passed as a place in memory at that address and then it will crash
    // need contiguous memory -> declare a fixed array
    
    Agent agents[1000];
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

  ShaderProgram shader;
  Mesh mesh;

  int agentNum = 1000;

  void reset() {
    for (int i = 0; i < agentNum; i++) {
      state().agents[i].pos(rv());
      state().agents[i].faceToward(rv());
    }
  }

  void onCreate() override {
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
      state().agents[i] = a;
      
      mesh.vertex(a.pos());
      mesh.normal(a.uf());
      const Vec3f& up(a.uu());
      mesh.color(up.x, up.y, up.z);
    }
    nav().pos(0, 0, 10);
  }

  void onAnimate(double dt) override {
    //calculate stuff
    //if (isPrimary()) { // if it is the primary window
        for (unsigned i = 0; i < agentNum; i++) {
            Vec3f avgHeading(0, 0, 0);
            Vec3f centerPos(0, 0, 0);
            state().agents[i].flockCount = 0; //reset flock count
            for (unsigned j = 1 + i; j < agentNum; j++) {
                float distance = (state().agents[j].pos() - state().agents[i].pos()).mag();
                
                if (distance < localRadius) { 
                //calculate alignment and cohesion vals if flock mates are far enough away from each other
                state().agents[i].flockCount++; //increase the flockmate count for that specific agent
                if (distance < separationDistance) {
                    //cout << "separate" << endl;
                    state().agents[i].pos() -= state().agents[j].uf().normalize() * turnRate * 0.002;
                } else {
                    avgHeading += state().agents[j].uf();
                    centerPos += state().agents[j].pos();
                }
                } 
            }

            if (state().agents[i].flockCount > 0) {
                state().agents[i].heading = avgHeading.normalize()/state().agents[i].flockCount;
                state().agents[i].center = centerPos.normalize()/state().agents[i].flockCount;
            }
        }
        // only once the above loop is done do we have good data on average headings and centers
        // here is where we actually point them in the right direction and move them
    
        for (unsigned i = 0; i < agentNum; i++) {
            //alignment
            state().agents[i].faceToward(state().agents[i].heading.normalize()); // point agents in the direction of their heading
            //cohesion
            state().agents[i].pos().lerp(state().agents[i].center.normalize() + state().agents[i].uf(), moveRate * 0.02);
        }

        // respawn agents if they go too far (MAYBE KEEP)
        for (unsigned i = 0; i < agentNum; i++) {
            if (state().agents[i].pos().mag() > 1.1) {
                state().agents[i].pos(rv());
                state().agents[i].faceToward(rv());
            }
        }
    //}

    // visualize the agents, update meshes (for ALL screens)
    vector<Vec3f>& v(mesh.vertices());
    vector<Vec3f>& n(mesh.normals());
    vector<Color>& c(mesh.colors());
    for (unsigned i = 0; i < agentNum; i++) {
      v[i] = state().agents[i].pos();
      n[i] = state().agents[i].uf();
      const Vec3d& up(state().agents[i].uu());
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
    g.clear(0.1, 0.1, 0.1);
    // gl::depthTesting(true); // or g.depthTesting(true);
    // gl::blending(true); // or g.blending(true);
    // gl::blendTrans(); // or g.blendModeTrans();
    g.shader(shader);
    g.shader().uniform("size", size * 0.03);
    g.shader().uniform("ratio", ratio * 0.2);
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
