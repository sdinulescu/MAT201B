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
  Color color;
  int size; //size is proportional to amount of lifespan the creature gains when it is consumed
  Vec3f position;
  Vec3f velocity;
  bool isConsumed = false;

  Food() { //Food constructor
    color = Color(1.0f, 0.0f, 0.0f, 1.0f);
    size = 1 + rnd::uniform() * 10; //at least a size of 1
    position = Vec3f( rnd::uniformS(), rnd::uniformS(), rnd::uniformS()  );
    velocity = Vec3f(  rnd::uniformS(), rnd::uniformS(), rnd::uniformS()  ) * 0.001;
    //cout << " size: " << size << " position: " << position << " velocity: " << velocity << endl;
  }

  void reset() {
    Vec3f p{  rnd::uniformS(), rnd::uniformS(), rnd::uniformS()  };
    position = p;
  }

  void setSize(int s) { size = s; }

  Vec3f getPosition() { return position; }
  Color getColor() { return color; }
  int getSize() { return size; }
  bool isFoodConsumed() { return isConsumed; }
};

struct Field {
  const static int foodNum = 500;
  vector<Food> food;

  void resetField() {
    initializeFood();

  }

  //Food
  void initializeFood() {
    for (int i = 0; i < foodNum; i++) {
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

// struct DrawableFood {
//   Color color;
//   int size; //size is proportional to amount of lifespan the creature gains when it is consumed
//   Vec3f position;

//   DrawableFood() {}

//   DrawableFood(Vec3f p, int s, Color c) {
//     position = p;
//     size = s;
//     color = c;
//   }
// };

// struct DrawableField {
//   DrawableFood dFood[1000];

//   DrawableField(int arrSize, Food& f) {
     
//   }

// };

