/* Agent.cpp
 * This file describes the properties and functionality of an "agent" -> this is for the simulation
 * This file also describes the properties of a "drawable agent" -> this is for the renderers ONLY
 */

#pragma once
#include "Gamma/Oscillator.h"

using namespace al;

//inherit from Sound Source -> try to find an example
// could potentially have a Pose in it -> look at this

//humans can locally pinpoint clicks, repeating buzzing signals

struct Agent : Pose {
  // Agent attributes

  //Agent's have a position, which is inherited from Pose -> .pos()
  //Agents have a unit forward vector, which is inherited from Pose -> .uf()
  
  float lifespan; // agents die after a certain point
  bool canReproduce; //true if it can reproduce, false if it can't
  Color agentColor;
  float colorTransparency; //sets the alpha value per agent

  //flocking attributes
  Vec3f heading; //heading from POV of agent (sum of all the headings, then divide by the count)
  Vec3f center;  //position in worldspace (sum of all the positions, then divide by the count)
  
  //these get inherited
  //adds individuality to each agent, fitness is evaluated
  Vec3f randomFlocking; //random direction parameter
  Vec3f moveRate; //how fast they move
  Vec3f turnRate; //how fast they turn to a new direction

  float fitnessValue; //keeps track of an agent's "fitness" -> higher is better
  float startCheckingFitness; //when do we start keeping track of fitness? when the lifespan of the agent is less than this

  unsigned flockCount{1}; //how many neighbors?

  //agent sound
  gam::Chirplet<> osc;  // a Gamma sine oscillator
  float frequency;
  float amplitude;

  //constructors
  Agent() { reset(); } //constructor, initialize with a position and a forward
  Agent(Vec3f p, Vec3f o, Vec3f m, Vec3f t, Vec3f r, Vec3f c) { //everything that gets inherited
    pos(p);
    faceToward(o);
    moveRate = m;
    turnRate = t;
    randomFlocking = r;
    lifespan = rnd::uniform() * 10.0;
    colorTransparency = lifespan * 0.1;
    agentColor = Color(c.x, c.y, c.z, colorTransparency);
    fitnessValue = 0.0;
    startCheckingFitness = rnd::uniformS()*10;
    canReproduce = false;

    // // TO DO: CHANGE THESE VALUES TO MAKE THEM INHERITABLE
    amplitude = 1;
    frequency = 500;
    // frequency = rnd::uniform() * 500.0; 
    // amplitude = rnd::uniform(); 
    // osc.freq(frequency);
    // osc.amp(amplitude);
    // osc.length(lifespan);
  }
  void reset() { //give agents a pos and a forward
    pos(Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()));
    faceToward(Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()));
    randomFlocking = Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS());
    moveRate = Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS());
    turnRate = Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS());
    lifespan = rnd::uniform() * 10.0;
    colorTransparency = lifespan * 0.1;
    agentColor = Color(rnd::uniform(), rnd::uniform(), rnd::uniform(), colorTransparency);
    fitnessValue = 0.0;
    startCheckingFitness = rnd::uniformS()*10.0;
    canReproduce = false;

    amplitude = 1;
    frequency = 500;

    // frequency = rnd::uniform() * 500.0;
    // amplitude = rnd::uniform();
    // osc.freq(frequency);
    // osc.amp(amplitude);
    // osc.length(0.8);
  }

  //getters and setters
  float getLifespan() {  return lifespan;  }
  void incrementLifespan(float amount) {
    lifespan += amount;
    colorTransparency = lifespan * 0.1;
    agentColor.a = colorTransparency;
  }
  void incrementFitness(float value) {  fitnessValue += value;  }

  // methods
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
        //cout << "kill it, it is not fit" << endl;
        lifespan = 0;
      }
    }
  }

  void updateAgentSound(int numberOfFlockmates, float avgFreq) {
    //set the amplitude
    amplitude = amplitude + numberOfFlockmates * 0.1; //set the amplitude proportional to the number of flockmates
    //set the frequency
    frequency = avgFreq + (pos().mag()); //SOMETHING ISN'T WORKING HERE
    //cout << amplitude << " " << frequency << endl;
  }

  float setChirp() {
    cout << "setting chirp" << endl;
    //these values will be individualized
    osc.amp(1);
    osc.freq(1000);
    osc.length(0.5);
  }

  float getSound() { return osc().norm(); }
};

//**************************
//to be passed to renderers
//**************************
struct DrawableAgent {
  Vec3f position, forward; //agents have a position and a forward
  Vec3f up; //part of orientation -> which way is up?
  Color agentColor;

  DrawableAgent() {}

  DrawableAgent(Vec3f p, Vec3f f, Vec3f u, Color c) {
    position = p;
    forward = f;
    up = u;
    agentColor = c;
  }
};