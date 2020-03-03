/* Agent.cpp
 * This file describes the properties and functionality of an "agent" -> this is for the simulation
 * This file also describes the properties of a "drawable agent" -> this is for the renderers ONLY
 */

#pragma once

#include "al/app/al_DistributedApp.hpp"
using namespace al;

Vec3f randomVec3f(){ 
  return Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS());
}

struct Agent : Pose {
  // Agent attributes

  //Agent's have a position, which is inherited from Pose -> .pos()
  //Agents have a unit forward vector, which is inherited from Pose -> .uf()
  
  float lifespan; // agents die after a certain point
  bool canReproduce;
  float colorTransparency;

  //flocking attributes
  Vec3f heading; //heading from POV of agent (sum of all the headings, then divide by the count)
  Vec3f center;  //position in worldspace (sum of all the positions, then divide by the count)
  
  //these get inherited
  //adds individuality to each agent, fitness is evaluated
  Vec3f randomFlocking; 
  Vec3f moveRate;
  Vec3f turnRate;

  float fitnessValue;
  float startCheckingFitness;

  unsigned flockCount{1};

  //methods
  Agent() { reset(); } //constructor, initialize with a position and a forward
  Agent(Vec3f p, Vec3f o, Vec3f h, Vec3f c, Vec3f m, Vec3f t, Vec3f r, float l) {
    pos(p);
    faceToward(o);
    heading = h;
    center = c;
    moveRate = m;
    turnRate = t;
    randomFlocking = r;
    lifespan = l;
    colorTransparency = lifespan * 0.1;
    canReproduce = false;
    fitnessValue = 0.0f;
    randomFlocking = Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS());
  }
  void reset() { //give agents a pos and a forward
    pos(randomVec3f());
    faceToward(randomVec3f());
    randomFlocking = Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS());
    moveRate = Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS());
    turnRate = Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS());
    //heading = Vec3f(rnd::uniform(), rnd::uniform(), rnd::uniform());
    //center = Vec3f(rnd::uniform(), rnd::uniform(), rnd::uniform());
    lifespan = rnd::uniform() * 10.0f;
    colorTransparency = lifespan * 0.1;
    fitnessValue = 0.0f;
    startCheckingFitness = rnd::uniformS()*10;
    canReproduce = false;
  }

  //getters and setters
  float getLifespan() {
    return lifespan;
  }

  void incrementLifespan(float amount) {
    lifespan += amount;
    colorTransparency = lifespan * 0.1;
  }
  
  void incrementFitness(float value) {
    fitnessValue += value;
  }

  bool checkReproduction(float reproductionProbabilityThreshold) {
    if (lifespan < (rnd::uniform() * 10)) {
      //roll for probability of reproduction
      float reproductionProbability = rnd::uniform();
      reproductionProbability += fitnessValue; // boids with a greater fitness value have a higher reproductive chance
      
      //cout << reproductionProbability << " " << reproductionThreshold << endl;
      if (reproductionProbability > reproductionProbabilityThreshold) { //random chance to reproduce
        canReproduce = true;
      }
    }
  }

  void evaluateFitness(float fitnessCutoff) {
    if (lifespan < startCheckingFitness) {
      //cout << "start checking fitness" << endl;
      if (fitnessValue < fitnessCutoff)  {
        cout << "kill it, it is not fit" << endl;
        lifespan = 0;
      }
    }
  }
};

// This is only what we need to draw on the GPU
struct DrawableAgent {
  Vec3f position, forward; //agents have a position and a forward
  Vec3f up; //part of orientation -> which way is up?
  float colorTransparency;

  DrawableAgent() {}

  DrawableAgent(Vec3f p, Vec3f f, Vec3f u, float c) {
    position = p;
    forward = f;
    up = u;
    colorTransparency = c;
  }
};