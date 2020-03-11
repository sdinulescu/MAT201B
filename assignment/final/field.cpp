/* State.cpp
 * This file describes the properties of the environmental field
 */
#pragma once 

#include "al/math/al_Random.hpp"
#include <fstream>
#include <vector>
using namespace al;
using namespace std;

//complete sphere imposters for the food
//figure out how to render these guys... don't like that they are just particles right now, not sophisticated

struct Food { //food in the field
  Color color;
  float size; //size is proportional to amount of lifespan the creature gains when it is consumed
  Vec3f position;
  Vec3f velocity;
  bool isConsumed = false;

  //Food constructor
  Food() {  reset();  }

  void reset() {
    color = Color(rnd::uniform(), rnd::uniform(), rnd::uniform());
    size = rnd::uniform(); //at least a size of 1
    position = Vec3f( rnd::uniformS(), rnd::uniformS(), rnd::uniformS()  );
    velocity = Vec3f(  rnd::uniformS(), rnd::uniformS(), rnd::uniformS()  ) * 0.001;
  }

  void setSize(int s) { size = s; }

  Vec3f getPosition() { return position; }
  Color getColor() { return color; }
  float getSize() { return size; }
  bool isFoodConsumed() { return isConsumed; }
};

struct Field {
// TO DO -- flesh this out
// include some environmental effects that can be positive or negative for agents in the surrounding area

  int amountOfFood = 500;
  vector<Food> food;

  int side;
  vector<Vec3f> fluidForces;
  vector<float> dampingFactors;

  // initialize
  void make(int s) {
    side = s;
    for (int i = 0; i < side*side*side; i++) {
      Vec3f randomFluidForce = Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS());
      fluidForces.push_back(randomFluidForce);
      dampingFactors.push_back(rnd::uniform());
    }
    cout << "make is done!" << endl;
  }

  void initializeFood() {
    for (int i = 0; i < amountOfFood; i++) {
      Food f;
      food.push_back(f);
    }
  }

  void resetField() { // initialize all things in the field
    cout << "init field..." << endl;
    make(10);
    initializeFood();
    cout << "field is initialized!" << endl;
    //initializeForces();
    // TO DO: add forces and other things here
  }

  //Food
  void moveFood() {
    for (int i = 0; i < food.size(); i++) {
        food[i].position += food[i].velocity; //moving at constant vel
    }
  }

  void updateFood() {
    for (int i = 0; i < food.size(); i++) {
      if (food[i].isConsumed) { //if the food is consumed, remove it from the vector
        food.erase(food.begin() + i);
      }
    }
    amountOfFood = food.size();
  }

  void addFood() {
    int foodToAdd = rnd::uniform() * 100;
    //cout << "adding " << foodToAdd << " food!" << endl;
    for (int i = 0; i < foodToAdd; i++) {
      Food f;
      food.push_back(f);
    }
    //cout << "new food size: " << food.size() << endl;
  }

  int getAmountOfFood() {
    return amountOfFood;
  }

  //Fluid Simulation
  int findGridBlock(Vec3f position) {
    float index = abs(position.x + position.y + position.z);
    index *= 10;
    int i = index;
    //cout << i << endl;
    return i;
  }

  Vec3f getForceVector(int index) { return fluidForces[index]; }
  void dampForces() {
    for (int i = 0; i < fluidForces.size(); i++) {
      if (fluidForces[i].mag() > 0.001) {
        fluidForces[i] = fluidForces[i] * dampingFactors[i];
      } else {
        fluidForces[i] = Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()); //reset the fluid force
      }
    }
  }
};

//**************************
//to be passed to renderers
//**************************
struct DrawableFood {
  Color color;
  int size; //size is proportional to amount of lifespan the creature gains when it is consumed
  Vec3f position;

  DrawableFood() {}

  DrawableFood(Vec3f p, int s, Color c) {
    position = p;
    size = s;
    color = c;
  }
};