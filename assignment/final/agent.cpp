/* Agent.cpp
 * This file describes the properties and functionality of an "agent" -> this is for the simulation
 * This file also describes the properties of a "drawable agent" -> this is for the renderers ONLY
 */

#pragma once
#include "Gamma/Oscillator.h"

using namespace al;

//inherit from Sound Source -> try to find an example
// could potentially have a Pose in it -> look at this

//humans can locally pinpoint clicks, repeating buzzing signals

struct Agent : Pose {
  // Agent attributes

  //Agent's have a position, which is inherited from Pose -> .pos()
  //Agents have a unit forward vector, which is inherited from Pose -> .uf()
  
  float lifespan; // agents die after a certain point
  bool canReproduce; //true if it can reproduce, false if it can't
  Color agentColor;
  bool isDead;

  //flocking attributes
  Vec3f heading; //heading from POV of agent (sum of all the headings, then divide by the count)
  Vec3f center;  //position in worldspace (sum of all the positions, then divide by the count)
  
  //these get inherited
  //adds individuality to each agent, fitness is evaluated
  Vec3f randomFlocking; //random direction parameter
  Vec3f moveRate; //how fast they move
  Vec3f turnRate; //how fast they turn to a new direction

  float fitnessValue; //keeps track of an agent's "fitness" -> higher is better
  float startCheckingFitness; //when do we start keeping track of fitness? when the lifespan of the agent is less than this

  unsigned flockCount{1}; //how many neighbors?

  //agent sound
  gam::Chirplet<> chirplet;  // a Gamma sine oscillator
  float startFrequency;
  float endFrequency;

  //constructors
  Agent() { reset(); } //constructor, initialize with a position and a forward
  Agent(Vec3f p, Vec3f o, Vec3f m, Vec3f t, Vec3f c) { //everything that gets inherited
    isDead = false;
    pos(p);
    faceToward(o);
    moveRate = m;
    turnRate = t;
    lifespan = rnd::uniform() * 10.0;
    agentColor = Color(c.x, c.y, c.z, lifespan);

    randomFlocking = Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS());

    fitnessValue = 0.0;
    startCheckingFitness = rnd::uniformS()*10;
    canReproduce = false;

    // // TO DO: CHANGE THESE VALUES TO MAKE THEM INHERITABLE
    startFrequency = rnd::uniform(220, 880); 
    endFrequency = rnd::uniform(220, 880);

    chirplet.freq(startFrequency, endFrequency);
  }
  void reset() { //give agents a pos and a forward
    isDead = false;
    pos(Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()));
    faceToward(Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()));
    randomFlocking = Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS());
    moveRate = Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS());
    turnRate = Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS());
    lifespan = rnd::uniform() * 10.0;
    agentColor = Color(rnd::uniform(), rnd::uniform(), rnd::uniform(), lifespan);
    fitnessValue = 0.0;
    startCheckingFitness = rnd::uniformS()*10.0;
    canReproduce = false;

    //sound
    startFrequency = rnd::uniform(220, 880);
    endFrequency = rnd::uniform(220, 880);

    chirplet.freq(startFrequency, endFrequency);
  }

  //getters and setters
  float getLifespan() {  return lifespan;  }
  void incrementLifespan(float amount) {
    lifespan += amount;
    agentColor.a = lifespan;
  }
  void incrementFitness(float value) {  fitnessValue += value;  }

  // methods
  bool checkReproduction(float reproductionProbabilityThreshold) {
    canReproduce = false; // always false to start the round
    if (lifespan < (rnd::uniform())) {
      //roll for probability of reproduction
      float reproductionProbability = rnd::uniform();
      //cout << "fitness value: " << fitnessValue << endl;
      reproductionProbability += fitnessValue; // boids with a greater fitness value have a higher reproductive chance
      
      //cout << reproductionProbability << " " << reproductionProbabilityThreshold << endl;
      if (reproductionProbability > reproductionProbabilityThreshold) { //random chance to reproduce
        canReproduce = true;
        fitnessValue = fitnessValue * 0.9; // need to cut their fitness a bit because they have to take care of a "child"
      }
    }
  }

  void setDeathState() {
    isDead = true;
    pos() = Vec3f(0, 0, 0);
    heading = Vec3f(0, 0 ,0);
    center = Vec3f(0, 0, 0);
    agentColor = Color(0 ,0 ,0, 0);
    moveRate = Vec3f(0, 0, 0);
    turnRate = Vec3f(0, 0, 0);
    randomFlocking = Vec3f(0, 0, 0);
    fitnessValue = 0;
    startCheckingFitness = 0;
    canReproduce = false;
  }

  void evaluateFitness(float fitnessCutoff) {
    if (lifespan < startCheckingFitness) {
      //cout << "start checking fitness" << endl;
      if (fitnessValue < fitnessCutoff)  {
        //cout << "kill it, it is not fit" << endl;
        lifespan = 0;
      }
    }
  }

  float nextSample() {
    if (chirplet.done()) {
      chirplet.freq(  startFrequency, endFrequency );
      chirplet.length(1); //length of chirp proportional to lifespan of agent
      startFrequency = endFrequency;
      endFrequency = rnd::uniform(220, 880);
    }
    return chirplet().r;
  }

  float randomCull(Vec3f cullPosition, float radius) {
    float cullingThreshold = 0.8;
    float distance = (pos() - cullPosition).mag();

    if (distance < radius) { //if within the culling radius, randomly kill
      if (rnd::uniform() > cullingThreshold) {
        lifespan = 0;
        //cout << "culled" << endl;
      }
    }
  }
};

//**************************
//to be passed to renderers
//**************************
struct DrawableAgent {
  Vec3f position, forward; //agents have a position and a forward
  Vec3f up; //part of orientation -> which way is up?
  Color agentColor;

  DrawableAgent() {}

  DrawableAgent(Vec3f p, Vec3f f, Vec3f u, Color c) {
    position = p;
    forward = f;
    up = u;
    agentColor = c;
  }
};