/* State.cpp
 * This file describes the properties of the shared state -> for the renderers
 */

#pragma once 

#include "al/app/al_DistributedApp.hpp"
#include "agent.cpp"
#include "field.cpp"

using namespace al;

const int MAX_AGENT_NUM = 200;
const int MAX_FOOD_NUM = 500;

// Only share the state that needs to be shared for sending
// Everything that is simulated
struct SharedState {
    Pose cameraPose; //where our camera is in space
    DrawableAgent dAgents[MAX_AGENT_NUM]; //visualize the agents
    DrawableFood dFood[MAX_FOOD_NUM]; //visualize the food
    float background; //of the window
    float size, ratio; //of agents
};