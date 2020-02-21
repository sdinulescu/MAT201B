/* final.cpp
 * Written by Stejara Dinulescu
 * This is my final project for MAT201B, taught by Karl Yerkes.
 * This program is a simulated environment of flocking agents. 
 * Incorporating an evolutionary algorithm, as time progresses, we see changes in flocking shape.
 */

//allolib includes
#include "al/app/al_DistributedApp.hpp"
#include "al/math/al_Random.hpp"
#include "al/ui/al_ControlGUI.hpp"
//cuttlebone includes
#include "al_ext/statedistribution/al_CuttleboneStateSimulationDomain.hpp"
//c std library includes
#include <fstream>
#include <vector>
//my includes
#include "agent.cpp"
#include "state.cpp"
//namespaces
using namespace al;
using namespace std;

// forward declarations of some functions
string slurp(string fileName); 

//global vars/containers
const int AGENT_NUM = 1000;
vector<Agent> agents;

class MyApp : public DistributedAppWithState<SharedState>  {
  //Gui params
  Parameter moveRate{"/moveRate", "", 0.35, "", 0.0, 2.0};
  Parameter turnRate{"/turnRate", "", 0.15, "", 0.0, 2.0};
  Parameter localRadius{"/localRadius", "", 0.6, "", 0.01, 0.9};
  Parameter separationDistance{"/separationDistance", "", 0.15, "", 0.01, 0.9};
  Parameter size{"/size", "", 1.0, "", 0.0, 2.0};
  Parameter ratio{"/ratio", "", 1.0, "", 0.0, 2.0};
  ControlGUI gui;

  std::shared_ptr<CuttleboneStateSimulationDomain<SharedState>>
      cuttleboneDomain;

  ShaderProgram shader;
  Mesh mesh;

 //***********************************************************************
 //Everything needed for onCreate

  void initCuttlebone() {
    //cuttlebone
    cuttleboneDomain =
        CuttleboneStateSimulationDomain<SharedState>::enableCuttlebone(this);
    if (!cuttleboneDomain) {
      std::cerr << "ERROR: Could not start Cuttlebone. Quitting." << std::endl;
      quit();
    }
  }

  void initGuiAndPassParams() {
    //gui
    gui << moveRate << turnRate << localRadius << separationDistance << size << ratio;
    gui.init();
  }

  void initAgents() {
    for (int i = 0; i < AGENT_NUM; i++) {
      Agent a;
      agents.push_back(a);
      
      mesh.vertex(a.pos());
      mesh.normal(a.uf());
      const Vec3f& up(a.uu());
      mesh.color(up.x, up.y, up.z);
    }
  }

  void reset() {
    for (int i = 0; i < agents.size(); i++) {
      agents[i].reset();
    }
  }

 //***********************************************************************
 //onCreate

  void onCreate() override {
    initCuttlebone();
    initGuiAndPassParams();
    
    navControl().useMouse(false);

    // compile shaders
    shader.compile(slurp("../tetrahedron-vertex.glsl"),
                   slurp("../tetrahedron-fragment.glsl"),
                   slurp("../tetrahedron-geometry.glsl"));

    //mesh
    mesh.primitive(Mesh::POINTS);

    initAgents();
    nav().pos(0, 0, 10);
  }

//***********************************************************************
//Everything needed for onAnimate()

  //reproduce between two boids
  void reproduce() { 

  }

  //check if the agent is dead
  void checkAgentDeath() {
    for (int i = 0; i < agents.size(); i++) {
      if (agents[i].lifespan <= 0) {
        agents.erase(agents.begin()+i);
        //cout << agents.size() << endl;
      }
    }
  }

  void eat() { // if the agent is at a specific location in the environment and finds food, then increase it's lifespan

  }

  //set the states for rendering
  void setState() {
    //copy simulation agents into drawable agents for rendering
    //cout << agents.size() << endl;
    for (unsigned i = 0; i < agents.size(); i++) {
      DrawableAgent a(agents[i].pos(), agents[i].uf(), agents[i].uu());
      state().dAgents[i] = a;
    }
    if (agents.size() < AGENT_NUM) { // if the vector is smaller than the Drawable Agent array capacity
      for (int i = agents.size(); i < AGENT_NUM; i++) {
        DrawableAgent a(Vec3f(0, 0, 0), Vec3f(0, 0, 0), Vec3f(0, 0, 0));
        state().dAgents[i] = a;
      }
    }

    //set the other state vars
    state().cameraPose.set(nav());
    state().background = 0.1;
    state().size = size.get();
    state().ratio = ratio.get();
  }

  // visualize the agents, update meshes using DrawableAgent in state (for ALL screens)
  void visualizeAgents() {
    vector<Vec3f>& v(mesh.vertices());
    vector<Vec3f>& n(mesh.normals());
    vector<Color>& c(mesh.colors());
    for (unsigned i = 0; i < agents.size(); i++) {
      v[i] = state().dAgents[i].position;
      n[i] = state().dAgents[i].forward;
      const Vec3d& up(state().dAgents[i].up);
      c[i].set(up.x, up.y, up.z);
    }
    if (agents.size() < AGENT_NUM) { // if the vector is smaller than the Drawable Agent array capacity
      for (int i = agents.size(); i < AGENT_NUM; i++) {
        c[i].set(0, 0, 0); //don't draw that agent
      }
    }
  }

 //***********************************************************************
 //onAnimate

  void onAnimate(double dt) override {
    if (cuttleboneDomain->isSender()) {
      for (unsigned i = 0; i < agents.size(); i++) {
        agents[i].decreaseLifespan(0.1);
        Vec3f avgHeading(0, 0, 0);
        Vec3f centerPos(0, 0, 0);
        agents[i].flockCount = 0; //reset flock count
        for (unsigned j = 1 + i; j < agents.size(); j++) {
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

      for (unsigned i = 0; i < agents.size(); i++) {
        //alignment
        agents[i].faceToward(agents[i].heading.normalize()); // point agents in the direction of their heading
        //cohesion
        agents[i].pos().lerp(agents[i].center.normalize() + agents[i].uf(), moveRate * 0.02);
      }

      // respawn agents if they go too far
      for (unsigned i = 0; i < agents.size(); i++) {
          if (agents[i].pos().mag() > 1.1) {
              agents[i].reset();
          }
      }

      checkAgentDeath();
      setState();

    } else { }

    
    visualizeAgents();
  }

 //***********************************************************************

  bool onKeyDown(const Keyboard& k) override {
    if (k.key() == 'r') {
      reset();
    }
    return true;
  }

 //***********************************************************************

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

 //***********************************************************************

int main() {
  MyApp app;
  app.start();
}

 //***********************************************************************

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
