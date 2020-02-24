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

  Food() { //Food constructor
    //auto rc = [&]() { return HSV(rnd::uniform(), rnd::uniform(), rnd::uniform()); };
    auto rv = [&]() { return Vec3f(rnd::uniformS() * 0.1, rnd::uniformS()* 0.1, rnd::uniformS()* 0.1); };
    
    color = Color(1.0f, 0.0f, 0.0f, 1.0f);
    size = 1 + rnd::uniform() * 10; //at least a size of 1
    position = rv();
    velocity = rv();
    cout << " size: " << size << " position: " << position << " velocity: " << velocity << endl;
  }

  void setSize(int s) { size = s; }

  Vec3f getPosition() { return position; }
  Color getColor() { return color; }
  int getSize() { return size; }
};

struct Field {
  const static int foodNum = 1000;
  Food food[foodNum];

  void resetField() {
    initFood();
  }

  //Food
  void initFood() {
    for (int i = 0; i < foodNum; i++) {
      Food f;
      food[i] = f;
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

