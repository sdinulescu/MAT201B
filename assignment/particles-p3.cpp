#include "al/app/al_App.hpp"
#include "al/math/al_Random.hpp"
#include "al/ui/al_ControlGUI.hpp"  // gui.draw(g)

using namespace al;

#include <fstream>
#include <vector>
using namespace std;

int partNum = 50;

Vec3f rv(float scale) {
  return Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()) * scale;
} //assigns randomness in three dimensions, multiplies it by scale

string slurp(string fileName); //forward declaration

struct AlloApp : App {
  Parameter pointSize{"/pointSize", "", 1.0, "", 0.0, 2.0};
  Parameter timeStep{"/timeStep", "", 0.1, "", 0.1, 0.6}; //simplest way to not get NANs, keep timestep small
  Parameter symmetry{"/symmetry", "", 1, "", 0.1, 1}; 
  //add GUI params here
  ControlGUI gui;

  ShaderProgram pointShader;
  Mesh mesh; //simulation state position is located in the mesh (positions are the direct simulation states that we use to draw)

  //simulation state
  vector<Vec3f> velocity;
  vector<Vec3f> acceleration;
  vector<float> mass;

  void onCreate() override {
    // add more GUI here
    gui << pointSize << timeStep << symmetry; //stream operator
    gui.init();
    navControl().useMouse(false);

    // compile shaders
    pointShader.compile(slurp("../point-vertex.glsl"),
                        slurp("../point-fragment.glsl"),
                        slurp("../point-geometry.glsl"));

    // set initial conditions

    // c++11 "lambda" function
    auto rc = []() { return HSV(rnd::uniform(), 1.0f, 1.0f); }; //picking a random hue with high saturation and brightness

    mesh.primitive(Mesh::POINTS);
    for (int r = 0; r < partNum; r++) { //create 1000 points, put it into mesh
      mesh.vertex(rv(5));
      mesh.color(rc());

      float m = 1; // mass = 1 -> initialize
      //      float m = rnd::uniform(3.0, 0.5);
      if (r==0) { //
        m = 1988.5; //push the sun into the first array spot
      } else if (r > 0 && r <= 9) {
        m = rnd::uniform(1898.2, 330.1); //push the planets
      } else if (r > 9 && r <= 50) {
        m = rnd::uniform(148.2, 1.0); //push the moons
      } else {
        m = rnd::uniform(1.0, 0.1); //push everything else
      }
      
      // float m = 3 + rnd::normal() * 0.1; //calculate mass and set it -> gaussian distribution of masses
      // if (m < 0.5) m = 0.5; //clamp the mass -> no smaller than 0.5
      mass.push_back(m);
     
      //set texture coordinate to be the size of the point (related to the mass)
      //using a simplified volume size relationship -> V = 4/3 * pi * r^3
                    //pow is power -> m^(1/3)
      float size = (4/3) * 3.14 * pow(m, 1.0f / 3);
      mesh.texCoord(size, 0); 
      //pass in an s, t (like x, y)-> where on an image do we want to grab the color from for this pixel (2D texture)
      //normalized between 0 and 1

      // separate state arrays
      velocity.push_back(rv(0.1)); //start with some small velocity
      acceleration.push_back(rv(0)); //start with no acceleration
    }

    nav().pos(0, 0, 10); //push camera back
  }

  bool freeze = false; //state that freezes simulation -> doesn't run onAnimate if frozen

  float checkMag(float value) {
    cout << "check mag" << endl;
    float v = 0.0;
    if (value > 5.0) {
        v = value/10;
        cout << "recalc " << v << endl;
        v = checkMag(v); //check recursively until it is in range
        cout << "HERE : " << v << endl;
    } else {
        v = value;
        cout << "value " << v << endl;
        return v; //return the final value
    }
  }
  
  void onAnimate(double dt) override {
    if (freeze) return;

    // ignore the real dt (real number of seconds that have passed since the last call) and set the time step
    // numerical simulation stability is due to small, regular timesteps
    dt = timeStep; //simulation time, not wall time

    // *********** Calculate forces ***********

    // gravity
    float G = 6.674; //gravitational constant
    for (int i = 0; i < partNum; i++) { //nested for loops (for each particle, calculate force with all other particles but itself one at a time)
      for (int j = 0; j < partNum; j++) {
        if (j!=i) {
          Vec3f distance(mesh.vertices()[i] - mesh.vertices()[j]); //calculate distances between particles
          Vec3f gravityVal = G * mass[i] * mass[j] / (distance.mag() * distance.mag()); // F = G * m1 * m2 / r^2
          //cout << gravityVal << endl;
          acceleration[i] += gravityVal;
          acceleration[j] -= (gravityVal*symmetry);
        }
      }
    }
    
    for (int i = 0; i < partNum; i++) {
        if (acceleration[i].mag() > 5) {
            float newVal = checkMag(acceleration[i].mag());
            cout << i << " " << newVal << endl;
            acceleration[i] = Vec3f(newVal);
        }
        //cout << acceleration[i] << endl;
    }

    if (keyOne == false) {
      for (int i = 0; i < partNum; i++) {
        acceleration[i] *= 100;
      }
    } else { keyOne = false; }

    //cout << acceleration[0] << endl;

    // drag -> stabilizes simulation
    for (int i = 0; i < partNum; i++) {
      // force of drag is proportional to the opposite of velocity * small amount
      // normally, it is v^2 -> can change the statement to vector[i].mag() * velocity[i] * 0.07;
      // take a bit of acceleration away proportional to what the velocity is
      acceleration[i] -= velocity[i] * 0.07;
    }

    // Integration -> don't mess with this
    vector<Vec3f>& position(mesh.vertices()); // reference (alias) to mesh.vertices()
    for (int i = 0; i < partNum; i++) {
      // "backward" Euler integration (semi-implicit, more stable)
      velocity[i] += acceleration[i] / mass[i] * dt;
      position[i] += velocity[i] * dt; // this is actually changing mesh.vertices() -> interchangeable

      // Explicit (or "forward") Euler integration would look like this:
      // position[i] += velocity[i] * dt;
      // velocity[i] += acceleration[i] / mass[i] * dt;
    }

    // clear all accelerations (IMPORTANT!!) -> accelerations have been used and counted already
    for (auto& a : acceleration) a.zero();
  }

  bool keyOne = false;

  bool onKeyDown(const Keyboard& k) override {
    if (k.key() == ' ') {
      freeze = !freeze;
    }

    if (k.key() == '1') {
      // introduce some "random" forces
      keyOne = true;
      for (int i = 0; i < partNum; i++) {
        acceleration[i] = rv(1) / mass[i]; // rv gives a force -> divide by mass to get acceleration
        //cout << acceleration[i] << endl;
      }
    }

    return true;
  }

  void onDraw(Graphics& g) override {
    g.clear(0.01);
    g.shader(pointShader);
    g.shader().uniform("pointSize", pointSize / 100);
    g.depthTesting(true);
    g.draw(mesh);
    gui.draw(g);
  }
};

int main() { AlloApp().start(); }

//slurp definition
string slurp(string fileName) {
  fstream file(fileName);
  string returnValue = "";
  while (file.good()) {
    string line;
    getline(file, line);
    returnValue += line + "\n";
  }
  return returnValue;
}
