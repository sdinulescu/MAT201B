#include "al/app/al_App.hpp"
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

// suggested structure for Agent
//
struct Agent : Pose {
  Vec3f heading; //heading from POV of agent (sum of all the headings, then divide by the count)
  Vec3f center;  // position in worldspace (sum of all the positions, then divide by the count)
  unsigned flockCount{1}; 
};

struct AlloApp : App {
  // add more GUI here
  Parameter moveRate{"/moveRate", "", 1.0, "", 0.0, 2.0};
  Parameter turnRate{"/turnRate", "", 1.0, "", 0.0, 2.0};
  Parameter localRadius{"/localRadius", "", 0.126, "", 0.01, 0.9};
  Parameter separationDistance{"/separationDistance", "", 0.720, "", 0.01, 0.9};
  Parameter size{"/size", "", 1.0, "", 0.0, 2.0};
  Parameter ratio{"/ratio", "", 1.0, "", 0.0, 2.0};
  ControlGUI gui;

  ShaderProgram shader;
  Mesh mesh;

  vector<Agent> agent;

  int agentNum = 1000;

  void reset() {
    for (int i = 0; i < agentNum; i++) {
      agent[i].pos(rv());
      agent[i].faceToward(rv());
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
      agent.push_back(a);
      
      mesh.vertex(a.pos());
      mesh.normal(a.uf());
      const Vec3f& up(a.uu());
      mesh.color(up.x, up.y, up.z);
    }
    nav().pos(0, 0, 10);
  }

  void onAnimate(double dt) override {
    //calculate stuff
    Vec3f avgHeading(0, 0, 0);
    Vec3f centerPos(0, 0, 0);
    for (unsigned i = 0; i < agentNum; i++) {
      agent[i].flockCount = 0; //reset flock count
      for (unsigned j = 1 + i; j < agentNum; j++) {
        float distance = (agent[j].pos() - agent[i].pos()).mag();
        
        if (distance < localRadius) { 
          //calculate alignment and cohesion vals if flock mates are far enough away from each other
          agent[i].flockCount++; //increase the flockmate count for that specific agent
          if (distance < separationDistance) {
            //cout << "separate" << endl;
            //avgHeading -= agent[j].uf();
            agent[i].pos() -= agent[j].uf().normalize() * moveRate * 0.002;
          } else {
            avgHeading += agent[j].uf();
            centerPos += agent[j].pos();
          }
        } 
      }

      //cout << agent[i].flockCount << endl;
      if (agent[i].flockCount > 0) {
        agent[i].heading = avgHeading.normalize()/agent[i].flockCount;
        agent[i].center = centerPos.normalize()/agent[i].flockCount;
      }

      //cout << agent[i].heading << " " << agent[i].center << endl;
    }
    // only once the above loop is done do we have good data on average headings and centers
    // here is where we actually point them in the right direction and move them
   
    for (unsigned i = 0; i < agentNum; i++) {
      //alignment
      agent[i].faceToward(agent[i].heading); // point agents in the direction of their heading
      //cohesion
      agent[i].pos().lerp(agent[i].center, moveRate * 0.02);
      //agent[i].pos() += agent[i].uf() * moveRate * 0.002; //move the agents along
    }

    // respawn agents if they go too far (MAYBE KEEP)
    // for (unsigned i = 0; i < agentNum; i++) {
    //   if (agent[i].pos().mag() > 1.1) {
    //     agent[i].pos(rv());
    //     agent[i].faceToward(rv());
    //   }
    // }

    // visualize the agents, update meshes
    vector<Vec3f>& v(mesh.vertices());
    vector<Vec3f>& n(mesh.normals());
    vector<Color>& c(mesh.colors());
    for (unsigned i = 0; i < agentNum; i++) {
      v[i] = agent[i].pos();
      n[i] = agent[i].uf();
      const Vec3d& up(agent[i].uu());
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
    gui.draw(g);
  }
};



int main() { AlloApp().start(); }

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
