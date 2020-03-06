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

struct Force { //forces in the field
  Vec3f position;
  Vec3f magnitude;
  Vec3f travelVelocity;

  float radius;

  //Force constructors
  Force() {  reset();  }
  void reset() {
    position = Vec3f( rnd::uniformS(), rnd::uniformS(), rnd::uniformS()  );
    magnitude = Vec3f(  rnd::uniform(), rnd::uniform(), rnd::uniform() * 10 );
    travelVelocity = Vec3f(  rnd::uniformS(), rnd::uniformS(), rnd::uniformS()  ) * 0.001;
    radius = rnd::uniform() * 100;
  }
};

struct Field {
// TO DO -- flesh this out
// include some environmental effects that can be positive or negative for agents in the surrounding area

  int amountOfFood = 500;
  vector<Food> food;
  const static int numberOfForces = 0;
  Force forces[numberOfForces];

  void resetField() { // initialize all things in the field
    initializeFood();
    initializeForces();
    // TO DO: add forces and other things here
  }

  //*********************Food*********************
  void initializeFood() {
    for (int i = 0; i < amountOfFood; i++) {
      Food f;
      food.push_back(f);
    }
  }

  void moveFood() {
    for (int i = 0; i < food.size(); i++) {
      if (food[i].position.mag() > 1.1) {
        food[i].reset();
      } else {
        food[i].position += food[i].velocity; //moving at constant vel
      }
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

  //*********************Forces*********************
  void initializeForces() {
    for (int i = 0 ; i < numberOfForces; i++) {
      Force f;
      forces[i] = f;
    }
  }
  void moveForces() {
    for (int i = 0; i < numberOfForces; i++) {
      forces[i].position += forces[i].travelVelocity;
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