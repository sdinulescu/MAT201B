/* Agent.cpp
 * This file describes the properties and functionality of an "agent" -> this is for the simulation
 * This file also describes the properties of a "drawable agent" -> this is for the renderers ONLY
 */

#pragma once
#include "Gamma/Oscillator.h"

using namespace al;

//humans can locally pinpoint clicks, repeating buzzing signals

// Impulse Generator struct taken from Pedal by Aaron Anderson and Kee Youn
struct ImpulseGenerator {
  float frequency, phase, period;
  float maskChance;
  float deviation, randomOffset;//deviation from periodicity
  float currentSample;

  ImpulseGenerator() {
    setFrequency(1.0f);//one impulse per second
    setPhase(0.0f);//initialize phase to 0
    setDeviation(0.0f);//ensure periodicity
    setMaskChance(0.0f);//no missing impulses
    randomOffset = rnd::uniform(-period*0.5, period*0.5) * deviation;
  }

  ImpulseGenerator(float initialFrequency) {
    setFrequency(initialFrequency);
    setPhase(0.0f);//initialize phase to 0
    setDeviation(0.0f);//ensure periodicity
    setMaskChance(0.0f);//no missing impulses
    randomOffset = rnd::uniform(-period*0.5, period*0.5) * deviation;
  }

  float generateSample(){
    if(phase >= period+randomOffset){
      float test = rnd::uniform(0.0f, 1.0f);
      if(test > maskChance){
        currentSample = 1.0f;
        float halfPeriod = period*0.5f;
        randomOffset = rnd::uniform(-halfPeriod, halfPeriod) * deviation;
      }
      phase -= period;
    }else{
      currentSample = 0.0f;
      phase += 1.0f;//increase phase by 1 sample 
    }
    return currentSample;
  }

  void setFrequency(float newFrequency){
    frequency = fabs(newFrequency);//no need for negative frequencies for this
    period = 44100/frequency;//period in samples
  }
  void setPhase(float newPhase){
      phase = newPhase*period;//conver from 0 - 1 to 0 - period
  }
  void setMaskChance(float newMaskChance){maskChance = newMaskChance;}
  void setDeviation(float newDeviation){deviation = newDeviation;}

  float getSample(){return currentSample;}
  float getFrequency(){return frequency;}
  float getMaskChance(){return maskChance;}
  float getDeviation(){return deviation;}
};

struct Chirplet {
  // float centerFrequency;
  // float range;
  // float windowPosition;
  // float duration;
   float currentSample;
  // float* windowPtr = nullptr;
  // int durationInSamples;
  // double windowPositionIncrement;
  // int windowArraySize = 1024;
  // bool active;

  gam::Sine<float> osc;

  Chirplet() {
    osc.freq(rnd::uniform(100.0f, 800.0f));
    // centerFrequency = rnd::uniform(200, 1000);
    // range = centerFrequency * rnd::uniform(0.01, 0.5); //pick a range dependent on centerFrequency
    // //duration = 0.1;
    // duration = rnd::uniform(0.05, 0.2);
    // osc.freq((centerFrequency + range));
    // durationInSamples = duration * 44100;
    // windowPositionIncrement = 1.0/durationInSamples;
    // //cout << windowPositionIncrement << endl;
    // active = false;    
  }

  float generateSample() { 
    //if (active && windowPtr != nullptr) {
      currentSample = osc();
    //   //cout << "wp: " << windowPosition << " arrSize " << windowArraySize << endl;
    //   int index = int(windowPosition * windowArraySize);
    //   //cout << "index: " << index << endl;
    //   index = std::fmin(std::fmax(index, 0), 1024);
    //   currentSample *= windowPtr[index];
    //   //cout << "currentSample: " << currentSample << endl;
    //   windowPosition = windowPosition + windowPositionIncrement;
    //  //cout << "agent index: " << index << " windowPosition " << windowPosition << " wpIncrement " << windowPositionIncrement << endl;
    //   if (windowPosition >= 1.0f) {
    //     windowPosition = 0.0f;
    //     active = false;
    //   }
    //}
    return currentSample; 
  }

  //void setActive(bool a) { active = a; }
  //void setWindowPtr(float* wPtr) { windowPtr = wPtr; }
};

struct Agent : Pose {
  // Agent attributes

  //Agent's have a position, which is inherited from Pose -> .pos()
  //Agents have a unit forward vector, which is inherited from Pose -> .uf()
  
  float lifespan; // agents die after a certain point
  bool canReproduce; //true if it can reproduce, false if it can't
  Color agentColor;
  bool isDead;

  int cyclesBeforeAteFood = 0;

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
  // gam::Chirplet<> chirplet;  // a Gamma sine oscillator
  //ImpulseGenerator impulse;
  Chirplet chirp;
  float currentSample;

  //constructors
  Agent() { reset(); } //constructor, initialize with a position and a forward
  Agent(Vec3f p, Vec3f o, Vec3f m, Vec3f t, Vec3f c) { //everything that gets inherited
    isDead = false;
    pos(p);
    faceToward(o);
    moveRate = m;
    turnRate = t;
    lifespan = rnd::uniform() * 10.0;
    agentColor = Color(c.x, c.y, c.z, lifespan);
    randomFlocking = Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS());
    fitnessValue = 0.0;
    startCheckingFitness = rnd::uniformS()*10;
    canReproduce = false;

    currentSample = 0.0f;
  }
  void reset() { //give agents a pos and a forward
    isDead = false;
    pos(Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()));
    faceToward(Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()));
    randomFlocking = Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS());
    moveRate = Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS());
    turnRate = Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS());
    lifespan = rnd::uniform() * 10.0;
    agentColor = Color(rnd::uniform(), rnd::uniform(), rnd::uniform(), lifespan);
    fitnessValue = 0.0;
    startCheckingFitness = rnd::uniformS()*10.0;
    canReproduce = false;

    currentSample = 0.0f;
  }

  //getters and setters
  float getLifespan() {  return lifespan;  }
  void incrementLifespan(float amount) {
    lifespan += amount;
    agentColor.a = lifespan;
  }
  void incrementFitness(float value) {  fitnessValue += value;  }

  // methods
  bool checkReproduction(float reproductionProbabilityThreshold) {
    canReproduce = false; // always false to start the round
    if (lifespan < (rnd::uniform())) {
      //roll for probability of reproduction
      float reproductionProbability = rnd::uniform();
      //cout << "fitness value: " << fitnessValue << endl;
      reproductionProbability += fitnessValue; // boids with a greater fitness value have a higher reproductive chance
      
      //cout << reproductionProbability << " " << reproductionProbabilityThreshold << endl;
      if (reproductionProbability > reproductionProbabilityThreshold) { //random chance to reproduce
        canReproduce = true;
        fitnessValue = fitnessValue * 0.9; // need to cut their fitness a bit because they have to take care of a "child"
      }
    }
  }

  void setDeathState() {
    isDead = true;
    pos() = Vec3f(0, 0, 0);
    heading = Vec3f(0, 0 ,0);
    center = Vec3f(0, 0, 0);
    agentColor = Color(0 ,0 ,0, 0);
    moveRate = Vec3f(0, 0, 0);
    turnRate = Vec3f(0, 0, 0);
    randomFlocking = Vec3f(0, 0, 0);
    fitnessValue = 0;
    startCheckingFitness = 0;
    canReproduce = false;
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

  float nextSample() {
    currentSample = chirp.generateSample();
    // if(impulse.generateSample() == 1.0f) { // start new chirp
    //   chirp.setActive(true);
    // }
    // if (chirp.active) {
    //   currentSample = chirp.generateSample(index);
    // }
    // if (chirplet.done()) {
    //   chirplet.freq(  startFrequency, endFrequency );
    //   chirplet.length(1); //length of chirp proportional to lifespan of agent
    //   startFrequency = endFrequency;
    //   endFrequency = rnd::uniform(220, 880);
    // }
    // return chirplet().r;
    return currentSample;
  }

  float randomCull(Vec3f cullPosition, float radius) {
    float cullingThreshold = 0.8;
    float distance = (pos() - cullPosition).mag();

    if (distance < radius) { //if within the culling radius, randomly kill
      if (rnd::uniform() > cullingThreshold) {
        lifespan = 0;
        //cout << "culled" << endl;
      }
    }
  }
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