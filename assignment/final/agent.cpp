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
  bool dead;

  //flocking attributes
  Vec3f heading; //heading from POV of agent (sum of all the headings, then divide by the count)
  Vec3f center;  // position in worldspace (sum of all the positions, then divide by the count)
  unsigned flockCount{1};

  //methods
  Agent() { reset(); } //constructor, initialize with a position and a forward

  void reset() { //give agents a pos and a forward
    pos(randomVec3f());
    faceToward(randomVec3f());
    lifespan = rnd::uniform() * 10.0f;
    cout << "lifespan: " << lifespan << endl;
    dead = false; //agent is alive to start with
  }

  //getters and setters
  float getLifespan() {
    return lifespan;
  }

  void decreaseLifespan(float l) {
    lifespan -=l;
  }

  void kill(bool d) {
    dead = d;
  }

  bool isDead() {
    return dead;
  }
};

// This is only what we need to draw on the GPU
struct DrawableAgent {
  Vec3f position, forward; //agents have a position and a forward
  Vec3f up; //part of orientation -> which way is up?

  DrawableAgent() {
    position = Vec3f(0, 0, 0);
    forward = Vec3f(0, 0, 0);
    up = Vec3f(0, 0, 0);
  }

  DrawableAgent(Vec3f p, Vec3f f, Vec3f u) {
    position = p;
    forward = f;
    up = u;
  }
};