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
#include "al/spatial/al_HashSpace.hpp"
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
HashSpace space(6, MAX_AGENT_NUM);

class MyApp : public DistributedAppWithState<SharedState>  {
  //global vars/containers
  const float FITNESS_CUTOFF = 10.0;
  Agent agents[MAX_AGENT_NUM];
  bool freeze = false;
  float timing = rnd::uniform(1,1000);
  unsigned counter = 0;
  //Gui params
  //flocking params
  Parameter backgroundColor{"/backgroundColor", "", 0.1, "", 0, 1};
  Parameter rate{"/rate", "", 0.015, "", 0.01, 0.1};
  Parameter localRadius{"/localRadius", "", 0.18, "", 0.01, 1.0};
  Parameter size{"/size", "", 1.5, "", 0.0, 4.0};
  Parameter ratio{"/ratio", "", 1.5, "", 0.0, 4.0};
  //evolution params
  Parameter reproductionDistanceThreshold{"/reproductionDistanceThreshold", "", 0.05, "", 0.0, 1.0};
  Parameter foodDistanceThreshold{"/foodDistanceThreshold", "", 0.05, "", 0.0, 1.0}; // have to be this far away to eat food
  Parameter decreaseLifespanAmount{"/decreaseLifespanAmount", "", 0.01, "", 0.0, 1.0};
  Parameter reproductionProbabilityThreshold{"/reproductionProbabilityThreshold", "", 200, "", 0, 500};
  ParameterInt k{"/k", "", 5, "", 1, 15};

  Parameter framesPerSecond{"/framesPerSecond", "", 0, "", 0, 100};
  Parameter aliveAgents{"/aliveAgents", "", 0, "", 0, 1000}; //TO DO: USE THIS TO KEEP TRACK OF HOW MANY ARE ALIVE

  ControlGUI gui;

  std::shared_ptr<CuttleboneStateSimulationDomain<SharedState>>
      cuttleboneDomain;

  ShaderProgram agentShader;
  ShaderProgram foodShader;

  Mesh agentMesh;
  Mesh foodMesh;
  Mesh cullMesh;

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
    gui << backgroundColor << rate << localRadius << k << size 
        << ratio << reproductionDistanceThreshold << foodDistanceThreshold 
        << decreaseLifespanAmount << reproductionProbabilityThreshold 
        << framesPerSecond << aliveAgents;
    gui.init();
  }

  void initAgents() {
    for (int i = MAX_AGENT_NUM; i < MAX_AGENT_NUM; i++) {
      Agent a;
      agents[i] = a;

      space.move(i, a.pos() * space.dim()); //push agents into the hash space
      
      agentMesh.vertex(a.pos());
      agentMesh.normal(a.uf());
      const Vec3f& up(a.uu());
      agentMesh.color(a.agentColor);
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
    gam::sampleRate(audioIO().framesPerSecond());

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
    cullMesh.primitive(Mesh::POINTS);

    field.resetField(); //initializes the field (fills the food array, initializes forces)
    initFoodMesh(); //init the food mesh with food vector
    initAgents(); //init the agent mesh with agent vector

    nav().pos(0, 0, 3);
  }

  //***********************************************************************
  //Everything needed for onAnimate()

  void eatFood() { // if the agent is at a specific location in the environment and finds food, then increase it's lifespan
    for (int i = 0; i < MAX_AGENT_NUM; i++) { //check each of the agents
      if (agents[i].isDead) { continue; }
      for (int j = 0; j < field.getAmountOfFood(); j++) { //check each of the food particles
        float distance = Vec3f(agents[i].pos() - field.food[j].getPosition()).mag();
        if (distance < foodDistanceThreshold) { //if the agent is this close to the food particle
          //cout << "food @ index " << i << " consumed!" << endl;
          field.food[j].isConsumed = true;
          agents[i].incrementLifespan(field.food[j].getSize()); //increase agent's lifespan by the food size
        }
      }
    }
  }

  // **************** CHANGE ****************
  void respawnFood() { // TO DO: only respawn food if something is triggered in the environment
    int foodThreshold = 250;
    if (field.getAmountOfFood() < foodThreshold) { //if there are less than X food particles in the field
    //cout << "food is under " << foodThreshold << endl;
      field.addFood();
    }
  }

  void applyForces() {
    //take an agent, find out the grid space that it is in
    for (int i = 0; i < MAX_AGENT_NUM; i++) {
      if (agents[i].isDead) { continue; }
      int index = field.findGridBlock(agents[i].pos()); //find the grid that it is in
      //cout << index << endl;
      Vec3f forceField = field.getForceVector(index); //get the force vector to apply to the agent
      agents[i].pos(agents[i].pos() + forceField * rate);
    }
    field.dampForces();
  }

  //reproduce between two boids
  void reproduce() { 
    //go through and roll for all agents -> reproduction
    int boidReproductionCount = 0;
    for (int i = 0; i < MAX_AGENT_NUM; i++) {
      if (agents[i].isDead) { continue; }
      agents[i].checkReproduction(reproductionProbabilityThreshold);
      if (agents[i].canReproduce) {
        boidReproductionCount++;
      }
    }
    //cout << "boids that can reproduce: " << boidReproductionCount << endl;
    int newAgentCount = 0;

    for (int i = 0; i < MAX_AGENT_NUM; i++) {
      if (agents[i].canReproduce) {
        //check nearest neighbor

        HashSpace::Query query(k);
        int results = query(space, agents[i].pos() * space.dim(),
                          space.maxRadius() * localRadius);
        for (int j = 0; j < results; j++) {
          int id = query[j]->id;
          if (agents[id].isDead) { continue; }
          if (agents[id].canReproduce == false) { continue; }
          //only reproduce if the nearest neighbor is alive AND can also reproduce
          
          float distance = Vec3f(  agents[id].pos() - agents[i].pos()  ).mag(); //check their distance
          if (distance < reproductionDistanceThreshold) { //if they are close enough, reproduce
            if (newAgentCount < MAX_AGENT_NUM - aliveAgents) {
              newAgentCount++;
              //cout << "reproduced!" << endl;
              Vec3f p = Vec3f(  (agents[i].pos() + agents[j].pos()) / 2  );
              Vec3f o = Vec3f(  (agents[i].uf() + agents[j].uf()) / 2  );
              //Vec3f h = Vec3f(  agents[i].heading + agents[j].heading  ) / 2;
              //Vec3f c = Vec3f(  agents[i].center + agents[j].center  ) / 2;
              Vec3f m = Vec3f(  (agents[i].moveRate + agents[j].moveRate) / 2  );
              Vec3f t = Vec3f(  (agents[i].turnRate + agents[j].turnRate) / 2  );
              Color col = (  agents[i].agentColor + agents[j].agentColor  ) / 2;
              Vec3f c = Vec3f(col.r, col.g, col.b);
              Agent a(p, o, m, t, c);
              agents[i] = a;
            }
          
          }
        }
      }
    }

    //reset reproduction boolean
    for (int i = 0; i < MAX_AGENT_NUM; i++) {
      if (agents[i].isDead) { continue; }
      agents[i].canReproduce = false;
    }
    //cout << "new agents this cycle: " << newAgentCount << endl;
    //cout << "--------------------" << endl;
  }

  void assignFitness() { //assign a fitness value to each agent based on specific rules
    for (int i = 0; i < MAX_AGENT_NUM; i++) {
      if (agents[i].isDead) { continue; }
      //first, what is it's fitness value??
      float valueScalar = 1.0f;
      //cout << agents[i].fitnessValue << endl;
      if (agents[i].fitnessValue > 500) {
        valueScalar *= 10;
      }

      float value = 0.0;
      //cout << "agent flockcount" << agents[i].flockCount << endl;
      //flock count
      if (agents[i].flockCount < 5 || agents[i].flockCount > 30) {
        value -= rnd::uniform() * agents[i].flockCount;
      } else {
        value += rnd::uniform() * agents[i].flockCount; //some random relationship, but also proportional to flockCount
      }
      //move rate
      if (agents[i].moveRate.mag() < 0.3 || agents[i].moveRate.mag() > 0.9) {
        value -= rnd::uniform() * agents[i].moveRate.mag(); //some random relationship, but also dependent on magnitude of moveRate
      } else {
        value += rnd::uniform() * agents[i].moveRate.mag();
      }
      //turn rate
      if (agents[i].turnRate.mag() < 0.3 || agents[i].turnRate.mag() > 0.9) {
        value -= rnd::uniform() * agents[i].turnRate.mag();
      } else {
        value += rnd::uniform() * agents[i].turnRate.mag();
      }

      value *= valueScalar;
      
      agents[i].incrementFitness(value); //change agent's fitness value
      agents[i].evaluateFitness(FITNESS_CUTOFF);
    }
  }

  //check if the agent is dead -> DO THIS FOR ALL AGENTS
  void checkAgentDeath() {
    int agentCounter = 0;
    for (int i = 0; i < MAX_AGENT_NUM; i++) {
      if (agents[i].lifespan <= 0) { agents[i].setDeathState(); } 
      else { agentCounter++; }
    }

    aliveAgents = agentCounter;
  }

  void cull() {
    if (counter % (int)timing == 0) { //cull
      //cout << "cull" << endl;
      Vec3f cullPosition = Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS());
      float radius = rnd::uniform() * 100.0;

      //add a cull to the mesh
      cullMesh.reset();
      cullMesh.vertex(cullPosition);
      cullMesh.color(Color(rnd::uniform(), rnd::uniform(), rnd::uniform(), 0.3));
      cullMesh.texCoord(radius, 0);

      for (int i = 0; i < MAX_AGENT_NUM; i++) {
        if (agents[i].isDead) { continue; }
        agents[i].randomCull(cullPosition, radius); //cull every second
      }

      timing = rnd::uniform(1,1000); //reset timing
    }
  }

  //flocking
  void calcFlocking() {
    for (unsigned i = 0; i < MAX_AGENT_NUM; i++) {
      if (agents[i].isDead) { continue; }
      agents[i].incrementLifespan(-1 * decreaseLifespanAmount);
      Vec3f avgHeading(0, 0, 0);
      Vec3f centerPos(0, 0, 0);
      agents[i].flockCount = 0; //reset flock count

      HashSpace::Query query(k);
      int results = query(space, agents[i].pos() * space.dim(),
                          space.maxRadius() * localRadius);
      for (int j = 0; j < results; j++) {
        int id = query[j]->id;
        if (agents[id].isDead) { continue; } // only look at the neighbors that are alive!
        avgHeading += agents[id].uf() + agents[id].randomFlocking;
        centerPos += agents[id].pos();
      }
      if (results > 0) {
        avgHeading = avgHeading.normalize() / results;
        centerPos = centerPos.normalize() / results;
      }
      agents[i].flockCount = results;
      agents[i].heading = avgHeading;
      agents[i].center = centerPos;
    }
  }

  void alignmentAndCohesion() { //agent update function
    //alignment and cohesion from boids algorithm
    for (unsigned i = 0; i < MAX_AGENT_NUM; i++) {
      if (agents[i].isDead) { continue; }
      agents[i].pos().lerp(agents[i].center.normalize() + agents[i].uf(), agents[i].moveRate.mag() * rate);
      space.move(i, agents[i].pos() * space.dim());
      agents[i].faceToward( (agents[i].heading + agents[i].center + agents[i].uf()).normalize() * agents[i].turnRate.mag() ); // point agents in the direction of their heading
    }
  }

  //set the states for rendering
  void setState() {
    //copy simulation agents into drawable agents for rendering
    //cout << agents.size() << endl;
    for (unsigned i = 0; i < MAX_AGENT_NUM; i++) { // set state for all the agents, no matter if they are dead or not
      DrawableAgent a(agents[i].pos(), agents[i].uf(), agents[i].uu(), agents[i].agentColor);
      state().dAgents[i] = a;
    }

    //set the environment
    //cout << field.food.size() << endl;
    for (unsigned i = 0; i < field.getAmountOfFood(); i++) {
      //copy all the new food positions
      //cout << field.food[i].getPosition() << " ";
      DrawableFood f(field.food[i].getPosition(), field.food[i].getSize(), field.food[i].getColor());
      state().dFood[i] = f;
    }

    //set the other state vars
    state().cameraPose.set(nav());
    state().background = backgroundColor;
    state().size = size.get();
    state().ratio = ratio.get();
  }

  //visualize
  void visualizeAgents() { // visualize the agents, update meshes using DrawableAgent in state (for ALL screens)
    agentMesh.reset();
    for (unsigned i = 0; i < MAX_AGENT_NUM; i++) {
      agentMesh.vertex(state().dAgents[i].position);
      agentMesh.normal(state().dAgents[i].forward);
      agentMesh.color(state().dAgents[i].agentColor.r, state().dAgents[i].agentColor.b, state().dAgents[i].agentColor.g, state().dAgents[i].agentColor.a);
    }
  }

  void visualizeFood() { // visualize the food, update meshes using DrawableFood in state (for ALL screens)
    foodMesh.reset();
    for (unsigned i = 0; i < field.getAmountOfFood(); i++) {
      //cout << state().dFood[i].size << endl;
      foodMesh.vertex(state().dFood[i].position);
      foodMesh.color(state().dFood[i].color.r, state().dFood[i].color.g, state().dFood[i].color.b);
      foodMesh.texCoord(state().dFood[i].size, 0);
    }
  }

  //***********************************************************************
  //update loop
  //frame counter 
  int frameCount{0};
  float timer{0};

  void onAnimate(double dt) override {
    counter++;
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
        calcFlocking();
        alignmentAndCohesion();

        assignFitness();
        reproduce();

        //respawn(); // make this a GUI toggle potentially

        checkAgentDeath();
        eatFood();

        cull();

        //update field
        field.moveFood(); //move the food
        field.updateFood(); //check what food was eaten and update the vector accordingly

        applyForces();

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
    for (int i = 0; i < MAX_AGENT_NUM; i++) {
      agents[i].reset();
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
  // onSound

  void onSound(AudioIOData& io) override {
    while (io()) {
      float sound = 0.0;

      // for (int i = 0; i < agents.size(); i++) {
      //   sound += agents[i].nextSample();
      // }
      // sound /= MAX_AGENT_NUM;
      // sound /= 3;
      
      // cap sound
      if (sound >  1) { sound = 1; }
      if (sound < -1) { sound = -1; }

      //cout << sound << endl;
      io.out(0) = io.out(1) = sound;    // write the signal to channels 0 and 1
    }
  }

  //***********************************************************************
  // draw loop

  void renderAgents(Graphics& g) {
    //agent shader
    g.shader(agentShader);
    g.shader().uniform("size", state().size * 0.03);
    g.shader().uniform("ratio", state().ratio * 0.2);
    //cout << "mesh size: " << agentMesh.vertices().size() << endl;
    g.draw(agentMesh);
  }

  void renderFood(Graphics& g) {
    //food shader
    g.shader(foodShader);
    g.shader().uniform("pointSize", state().size * 0.005);
    //cout << foodMesh.vertices().size() << endl;
    g.draw(foodMesh);
    //g.draw(cullMesh);
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
    // Enable audio with 2 channels of output.
  app.configureAudio(44100, 2048, 2, 0);
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