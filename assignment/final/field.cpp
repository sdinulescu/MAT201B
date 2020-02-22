/* State.cpp
 * This file describes the properties of the environmental field
 */
#pragma once 

#include "al/app/al_DistributedApp.hpp"
#include "al/math/al_Random.hpp"
#include <fstream>
#include <vector>
using namespace al;
using namespace std;

struct Food {
  Color c;
  int size; //size is proportional to amount of lifespan the creature gains when it is consumed
  Vec3f position;
  Vec3f velocity;

  Food() { //Food constructor
    rnd::Random<> rng;
    rng.seed(42);
    auto rc = [&]() { return HSV(rng.uniform(), 1.0f, 1.0f); };
    auto rv = [&]() { return Vec3f(rnd::uniformS() * 0.1, rnd::uniformS()* 0.1, rnd::uniformS()* 0.1);};
    
    c = rc();
    //size = rnd::uniform() * 10;
    position = rv();
    velocity = rv();
  }

  Vec3f getPosition() {
    return position;
  }
  Color getColor() {
    return c;
  }
};

struct Field {
  int foodNum;
  vector<Food> food;

  void resetField() {
    initFood();
  }

  //Food
  void initFood() {
    foodNum = rnd::uniform() * 1000;
    for (int i = 0; i < foodNum; i++) {
      Food f;
      food.push_back(f);
    }
  }

  void moveFood() {
    for (int i = 0; i < foodNum; i++) {
      food[i].position += food[i].velocity; //moving at constant vel
    }
  }

  int getFoodNum() {
    return foodNum;
  }


  //Forces
  void applyResistance() {

  }

  void applyAttraction() {

  }
};

