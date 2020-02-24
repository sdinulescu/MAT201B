/* State.cpp
 * This file describes the properties of the shared state -> for the renderers
 */

#pragma once 

#include "al/app/al_DistributedApp.hpp"
#include "agent.cpp"
#include "field.cpp"

using namespace al;

const int MAX_AGENT_NUM = 1000;

// Only share the state that needs to be shared for sending
// Everything that is simulated
struct SharedState {
    Pose cameraPose;
    DrawableAgent dAgents[MAX_AGENT_NUM];
    //DrawableField field;
    float background;
    float size, ratio;
};