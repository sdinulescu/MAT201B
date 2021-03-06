#include "al/app/al_App.hpp"
#include "al/math/al_Random.hpp"
#include "al/ui/al_ControlGUI.hpp"  // gui.draw(g)

using namespace al;

#include <fstream>
#include <vector>
using namespace std;

int partNum = 1000;

Vec3f rv(float scale) {
  return Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS()) * scale;
} //assigns randomness in three dimensions, multiplies it by scale

string slurp(string fileName); //forward declaration

struct AlloApp : App {
  Parameter pointSize{"/pointSize", "", 1.0, "", 0.0, 2.0};
  Parameter timeStep{"/timeStep", "", 0.03, "", 0.01, 0.6}; //simplest way to not get NANs, keep timestep small
  Parameter gravConst{"/gravConst", "", 0.002, "", 0, 1};
  Parameter dragFactor{"/dragFactor", "", 0.07, "", 0.01, 0.99};
  Parameter maxAccel{"/maxAccel", "", 30, "", 0, 100};
  //add GUI params here
  ControlGUI gui;

  ShaderProgram pointShader;
  Mesh mesh; //simulation state position is located in the mesh (positions are the direct simulation states that we use to draw)

  //simulation state
  vector<Vec3f> velocity;
  vector<Vec3f> acceleration;
  vector<float> mass;

  void reset() { //empty all containers
    mesh.reset();
    velocity.clear();
    acceleration.clear();

    // c++11 "lambda" function
    // seed random number generators to maintain determinism
    rnd::Random<> rng;
    rng.seed(42);
    auto rc = [&]() { return HSV(rng.uniform(), 1.0f, 1.0f); };
    auto rv = [&](float scale) -> Vec3f {
      return Vec3f(rng.uniformS(), rng.uniformS(), rng.uniformS()) * scale;
    };

    mesh.primitive(Mesh::POINTS);
    for (int r = 0; r < partNum; r++) { //create 1000 points, put it into mesh
      mesh.vertex(rv(5));
      mesh.color(rc());

      float m = 1;
      if (r==0) { //
         m = 19885.0; //push the sun into the first array spot
       } else if (r > 0 && r <= 9) {
         m = rnd::uniform(10000.0, 3300.1); //push the planets
       } else if (r > 9 && r <= 50) {
         m = rnd::uniform(330.1, 100.0); //push the moons
       } else {
         m = rnd::uniform(10.0, 0.1); //push everything else
       }
      mass.push_back(m);
     
      //set texture coordinate to be the size of the point (related to the mass)
      //using a simplified volume size relationship -> V = 4/3 * pi * r^3
                    //pow is power -> m^(1/3)
      mesh.texCoord((4/3) * 3.14 * pow(m, 1.0f / 3), 0); 
      //pass in an s, t (like x, y)-> where on an image do we want to grab the color from for this pixel (2D texture)
      //normalized between 0 and 1

      // separate state arrays
      velocity.push_back(rv(0.1)); //start with some small velocity
      acceleration.push_back(rv(0)); //start with no acceleration
    }
  }

  void onCreate() override {
    // add more GUI here
    gui << pointSize << timeStep << dragFactor << maxAccel; //stream operator
    gui.init();
    navControl().useMouse(false);

    // compile shaders
    pointShader.compile(slurp("../point-vertex.glsl"),
                        slurp("../point-fragment.glsl"),
                        slurp("../point-geometry.glsl"));

    // set initial conditions
    reset();
    nav().pos(-1, -1, 20); //push camera back
  }

  bool freeze = false; //state that freezes simulation -> doesn't run onAnimate if frozen
  void onAnimate(double dt) override {
    if (freeze) return;

    // ignore the real dt (real number of seconds that have passed since the last call) and set the time step
    // numerical simulation stability is due to small, regular timesteps
    dt = timeStep; //simulation time, not wall time

    // *********** Calculate forces ***********

    // gravity
    float G = 6.674e-4; //gravitational constant
    for (int i = 0; i < partNum; i++) { //nested for loops (for each particle, calculate force with all other particles but itself one at a time)
      for (int j = 1+i; j < partNum; j++) {
          Vec3f distance(mesh.vertices()[j] - mesh.vertices()[i]); //calculate distances between particles -> b-a = c
          Vec3f gravityVal = G * mass[i] * mass[j] * distance.normalize() / pow(distance.mag(), 2); // F = G * m1 * m2 / r^2
          //multiply by r hat -> only direction, normalized magnitude

          acceleration[i] += gravityVal/mass[i];
          acceleration[j] -= gravityVal/mass[j];
      }
    }

    //limit acceleration
    for (int i = 0; i < acceleration.size(); i++) {
      float m = acceleration[i].mag();
      if (m > maxAccel) {
        acceleration[i].normalize(maxAccel);
        cout << "Limiting Acceleration: " << m << " -> " << (float)maxAccel
             << endl;
      }
    }

    // drag -> stabilizes simulation
    for (int i = 0; i < partNum; i++) {
      // force of drag is proportional to the opposite of velocity * small amount
      // normally, it is v^2 -> can change the statement to vector[i].mag() * velocity[i] * 0.07;
      // take a bit of acceleration away proportional to what the velocity is
      acceleration[i] -= velocity[i] * dragFactor;
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

  bool onKeyDown(const Keyboard& k) override {
    if (k.key() == ' ') {
      freeze = !freeze;
    }

    if (k.key() == '1') {
      // introduce some "random" forces
      for (int i = 0; i < partNum; i++) {
        acceleration[i] = rv(1) / mass[i]; // rv gives a force -> divide by mass to get acceleration
        //cout << acceleration[i] << endl;
      }
    }

     if (k.key() == 'r') {
        reset();
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
