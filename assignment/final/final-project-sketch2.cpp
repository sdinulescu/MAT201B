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
#include "field.cpp"
//namespaces
using namespace al;
using namespace std;

// forward declarations of some functions
string slurp(string fileName); 

//global vars/containers
//const int AGENT_NUM = 1000;
//const int FOOD_NUM = 500;
vector<Agent> agents;

class MyApp : public DistributedAppWithState<SharedState>  {
  bool freeze = false;

  //Gui params
  //flocking params
  Parameter rate{"/rate", "", 0.01, "", 0.01, 1.0};
  Parameter localRadius{"/localRadius", "", 0.8, "", 0.01, 1.0};
  Parameter separationDistance{"/separationDistance", "", 0.03, "", 0.01, 0.9};
  Parameter size{"/size", "", 1.5, "", 0.0, 4.0};
  Parameter ratio{"/ratio", "", 1.5, "", 0.0, 4.0};
  //evolution params
  Parameter reproductionDistanceThreshold{"/reproductionDistanceThreshold", "", 0.2, "", 0.0, 1.0};
  Parameter foodDistanceThreshold{"/foodDistanceThreshold", "", 0.2, "", 0.0, 1.0}; // have to be this far away to eat food
  Parameter decreaseLifespanAmount{"/decreaseLifespanAmount", "", 0.01, "", 0.0, 1.0};
  Parameter reproductionProbabilityThreshold{"/reproductionProbabilityThreshold", "", 0.4, "", 0.0, 1.0};
  Parameter framesPerSecond{"/framesPerSecond", "", 0, "", 0, 100};

  ControlGUI gui;

  std::shared_ptr<CuttleboneStateSimulationDomain<SharedState>>
      cuttleboneDomain;

  ShaderProgram agentShader;
  ShaderProgram foodShader;

  Mesh agentMesh;
  Mesh foodMesh;

  Field field;

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
    gui << localRadius << separationDistance << size 
        << ratio << reproductionDistanceThreshold << foodDistanceThreshold 
        << decreaseLifespanAmount << reproductionProbabilityThreshold 
        << framesPerSecond;
    gui.init();
  }

  void initAgents() {
    for (int i = agents.size(); i < MAX_AGENT_NUM; i++) {
      Agent a;
      agents.push_back(a);
      
      agentMesh.vertex(a.pos());
      agentMesh.normal(a.uf());
      const Vec3f& up(a.uu());
      agentMesh.color(up.x, up.y, up.z, a.colorTransparency);
    }
  }

  void initFoodMesh() {
    for(int i = 0; i < field.getAmountOfFood(); i++) {
      foodMesh.vertex(field.food[i].getPosition());
      foodMesh.color(field.food[i].getColor());
      foodMesh.texCoord(field.food[i].getSize(), 0);
    }
  }

  //***********************************************************************
  //onCreate

  void onCreate() override {
    initCuttlebone();
    initGuiAndPassParams();
    
    navControl().useMouse(false);

    // compile shaders
    agentShader.compile(slurp("../tetrahedron-vertex.glsl"),
                   slurp("../tetrahedron-fragment.glsl"),
                   slurp("../tetrahedron-geometry.glsl"));
    foodShader.compile(slurp("../point-vertex.glsl"),
                   slurp("../point-fragment.glsl"),
                   slurp("../point-geometry.glsl"));

    //mesh
    agentMesh.primitive(Mesh::POINTS);
    foodMesh.primitive(Mesh::POINTS);

    field.resetField(); //initializes the field (fills the food array)
    initFoodMesh(); //init the mesh with food vector

    initAgents();
    nav().pos(0, 0, 3);
  }

  //***********************************************************************
  //Everything needed for onAnimate()

  void eatFood() { // if the agent is at a specific location in the environment and finds food, then increase it's lifespan
    for (int i = 0; i < agents.size(); i++) { //check each of the agents
      for (int j = 0; j < field.food.size(); j++) { //chech each of the food particles
        float distance = Vec3f(agents[i].pos() - field.food[j].getPosition()).mag();
        if (distance < foodDistanceThreshold) { //if the agent is this close to the food particle
          //cout << "food @ index " << i << " consumed!" << endl;
          field.food[j].isConsumed = true;
          agents[i].increaseLifespan(field.food[j].getSize()); //increase agent's lifespan by the food size
        }
      }
    }
  }

  // **************** CHANGE ****************
  void respawnFood() { // TO DO: only respawn food if something is triggered in the environment
    int foodThreshold = 100;
    if (field.getAmountOfFood() < foodThreshold) { //if there are less than 50 food particles in the field
    //cout << "food is under " << foodThreshold << endl;
      field.addFood();
    }
  }

  //TO DO!!
  //reproduce between two boids
  void reproduce() { 
    //go through and roll for all agents -> reproduction
    int boidReproductionCount = 0;
    for (int i = 0; i < agents.size(); i++) {
      agents[i].checkReproduction(reproductionProbabilityThreshold);
      if (agents[i].canReproduce) {
        boidReproductionCount++;
      }
    }
    //cout << "boids that can reproduce: " << boidReproductionCount << endl;
    int newAgentCount = 0;

    for (int i = 0; i < agents.size(); i++) {
      if (agents[i].canReproduce) {
        //check nearest neighbor
        for (int j = i + 1; j < agents.size(); j++) {
          if (agents[j].canReproduce) {
            float distance = Vec3f(  agents[j].pos() - agents[i].pos()  ).mag(); //check their distance
            if (distance < reproductionDistanceThreshold) { //if they are close, reproduce
              if (newAgentCount < (MAX_AGENT_NUM - agents.size())) {
                newAgentCount++;
                //cout << "reproduced!" << endl;
                Vec3f p = (  agents[i].pos() + agents[j].pos()  ) / 2;
                Vec3f o = (  agents[i].uf() + agents[j].uf()  ) / 2;
                Vec3f h = (  agents[i].heading + agents[j].heading  ) / 2;
                Vec3f c = (  agents[i].center + agents[j].center  ) / 2;
                Vec3f m = (  agents[i].moveRate + agents[j].moveRate  ) / 2;
                Vec3f t = (  agents[i].turnRate + agents[j].turnRate  ) / 2;
                Vec3f r = (  agents[i].randomFlocking + agents[j].randomFlocking  )/2;
                float l = (  agents[i].getLifespan() + agents[j].getLifespan()  ) / 2;
                Agent a(p, o, h, c, m, t, r, l);
                agents.push_back(a);
              }
            }
          }
        }
      }
    }

    //reset reproduction boolean
    for (int i = 0; i < agents.size(); i++) {
      agents[i].canReproduce = false;
    }
    //cout << "new agents this cycle: " << newAgentCount << endl;
    //cout << "--------------------" << endl;
  }

  void fitnessFunction() {
    
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

  // respawn agents if they go too far
  void respawn() {
    for (unsigned i = 0; i < agents.size(); i++) {
        if (agents[i].pos().mag() > 1.1) {
            agents[i].reset();
        }
    }
  }
  void wrapAgentPositions() {
    for (unsigned i = 0; i < agents.size(); i++) {

    }
  }

  //flocking
  void calcFlockingAndSeparation() {
    for (unsigned i = 0; i < agents.size(); i++) {
      agents[i].decreaseLifespan(decreaseLifespanAmount);
      Vec3f avgHeading(0, 0, 0);
      Vec3f centerPos(0, 0, 0);
      agents[i].flockCount = 0; //reset flock count
      for (unsigned j = 1 + i; j < agents.size(); j++) {
        float distance = (agents[j].pos() - agents[i].pos()).mag();
          
        if (distance < localRadius) { 
          //calculate alignment and cohesion vals if flock mates are far enough away from each other
          agents[i].flockCount++; //increase the flockmate count for that specific agent
          avgHeading += agents[j].uf() + agents[j].randomFlocking;
          centerPos += agents[j].pos();
          if (distance < separationDistance) {
              //cout << "separate" << endl;
              agents[i].pos() -= agents[j].uf().normalize() * agents[i].moveRate * rate;
          } else {
              // avgHeading += agents[j].uf();
              // centerPos += agents[j].pos();
          }
        } 
      }

      if (agents[i].flockCount > 0) {
          agents[i].heading = avgHeading.normalize()/agents[i].flockCount;
          agents[i].center = centerPos.normalize()/agents[i].flockCount;
      }
    }
  }

  void alignmentAndCohesion() {
    for (unsigned i = 0; i < agents.size(); i++) {
      agents[i].pos().lerp(agents[i].center.normalize() + agents[i].uf(), agents[i].moveRate.mag() * rate);
      agents[i].faceToward( (agents[i].heading + agents[i].center + agents[i].uf()).normalize() + agents[i].turnRate); // point agents in the direction of their heading
    }
  }

  //set the states for rendering
  void setState() {
    //copy simulation agents into drawable agents for rendering
    //cout << agents.size() << endl;
    for (unsigned i = 0; i < agents.size(); i++) {
      DrawableAgent a(agents[i].pos(), agents[i].uf(), agents[i].uu(), agents[i].colorTransparency);
      state().dAgents[i] = a;
    }
    if (agents.size() < MAX_AGENT_NUM) { // if the vector is smaller than the Drawable Agent array capacity
      for (int i = agents.size(); i < MAX_AGENT_NUM; i++) {
        DrawableAgent a(Vec3f(0, 0, 0), Vec3f(0, 0, 0), Vec3f(0, 0, 0), 0.0f);
        state().dAgents[i] = a;
      }
    }

    //set the environment
    //cout << field.food.size() << endl;
    for (unsigned i = 0; i < field.food.size(); i++) {
      //copy all the new food positions
      //cout << field.food[i].getPosition() << " ";
      DrawableFood f(field.food[i].getPosition(), field.food[i].getSize(), field.food[i].getColor());
      state().dFood[i] = f;
    }
    if (field.food.size() < MAX_FOOD_NUM) { // if the vector is smaller than the Drawable Agent array capacity
      //cout << "less food" << endl;
      for (int i = field.food.size(); i < MAX_FOOD_NUM; i++) {
        DrawableFood f(Vec3f(0, 0, 0), 0, Color(0, 0, 0, 0));
        state().dFood[i] = f;
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
    for (unsigned i = 0; i < agents.size(); i++) {
      agentMesh.vertices()[i] = state().dAgents[i].position;
      agentMesh.normals()[i] = state().dAgents[i].forward;
      const Vec3d& up(state().dAgents[i].up);
      agentMesh.colors()[i].set(up.x, up.y, up.z, state().dAgents[i].colorTransparency);
      //cout << "color " << state().dAgents[i].colorTransparency << endl;
    }
    if (agents.size() < MAX_AGENT_NUM) { // if the vector is smaller than the Drawable Agent array capacity
      for (int i = agents.size(); i < MAX_AGENT_NUM; i++) {
        agentMesh.colors()[i].set(0.0f, 0.0f, 0.0f, 0.0f); //don't draw that agent
      }
    }
  }

  void visualizeFood() { //update the mesh
    //foodMesh.reset();
    for (unsigned i = 0; i < MAX_FOOD_NUM; i++) {
      foodMesh.vertices()[i] = state().dFood[i].position;
      foodMesh.colors()[i].set(state().dFood[i].color.r, state().dFood[i].color.g, state().dFood[i].color.b);
      foodMesh.texCoord(state().dFood[i].size, 0);
      //cout << foodMesh.vertices()[i] << endl;
    }

    
    //foodMesh.reset();
    //initFoodMesh();
  }

 
  

  //***********************************************************************
  //update loop
   //frame counter 
  int frameCount{0};
  float timer{0};

  void onAnimate(double dt) override {
    timer += dt;
    frameCount++;
    if (timer > 1) {
      timer -= 1;
      framesPerSecond = frameCount;
      frameCount = 0;
    }
    if (freeze == false) {
      if (cuttleboneDomain->isSender()) {
        //update the food
        respawnFood();

        //update agents
        calcFlockingAndSeparation();
        alignmentAndCohesion();

        reproduce();

        //respawn(); // make this a GUI toggle potentially

        checkAgentDeath();
        eatFood();

        //update field
        field.moveFood();
        field.updateFood(); //do this after we eat food

        //state
        setState();
      } else {  nav().set(state().cameraPose);  }

      visualizeAgents();
      visualizeFood();
    }
  }

  //***********************************************************************
  // key pressed

  void reset() { //reset agents
    for (int i = 0; i < agents.size(); i++) {
      agents[i].reset();
    }
    for (int i = agents.size(); i < MAX_AGENT_NUM; i++) {
      Agent a; 
      agents.push_back(a);
    }

    foodMesh.reset();
    field.resetField();
    initFoodMesh();
  }

  bool onKeyDown(const Keyboard& k) override {
    if (k.key() == 'r') {
      reset();
    }
    if (k.key() == ' ') {
      freeze = !freeze;
    }
    return true;
  }

  //***********************************************************************
  // draw loop

  void renderAgents(Graphics& g) {
    //agent shader
    g.shader(agentShader);
    g.shader().uniform("size", state().size * 0.03);
    g.shader().uniform("ratio", state().ratio * 0.2);
    g.draw(agentMesh);
  }

  void renderFood(Graphics& g) {
    //food shader
    g.shader(foodShader);
    g.shader().uniform("pointSize", state().size * 0.005);
    g.draw(foodMesh);
  }

  void onDraw(Graphics& g) override {
    g.clear(state().background, state().background, state().background);
    gl::depthTesting(true);  // or g.depthTesting(true);
    gl::blending(true);      // or g.blending(true);
    gl::blendTrans();        // or g.blendModeTrans();

    renderFood(g);
    renderAgents(g);

    if (cuttleboneDomain->isSender()) {
      gui.draw(g);
    }
  }
};

//***********************************************************************
// main

int main() {
  MyApp app;
  app.start();
}

//***********************************************************************
// random methods

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
