/* Agent.cpp
 * This file describes the properties and functionality of an "agent" -> this is for the simulation
 * This file also describes the properties of a "drawable agent" -> this is for the renderers ONLY
 */

#pragma once

#include "al/app/al_DistributedApp.hpp"
using namespace al;

struct Agent : Pose {
  //Agent's have a position, which is inherited from Pose -> .pos()
  //Agents have a unit forward vector, which is inherited from Pose -> .uf()

  Vec3f heading; //heading from POV of agent (sum of all the headings, then divide by the count)
  Vec3f center;  // position in worldspace (sum of all the positions, then divide by the count)
  unsigned flockCount{1};

  Vec3f rv(){ 
    float scale = 1.0f;
    return Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()) * scale;
  }

  Agent() {
    reset();
  }

  void reset() {
    //give agents a pos and a forward
    pos(rv());
    faceToward(rv());
  }

  // //getters and setters
  // void setPosition() {
  //   pos(rv());
  // }

  // Vec3f getPosition() {
  //   return pos();
  // }
};

// This is only what we need to draw on the GPU
struct DrawableAgent {
  Vec3f position, forward; //agents have a position and a forward
  Vec3f up; //part of orientation -> which way is up?
};