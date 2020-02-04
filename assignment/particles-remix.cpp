/* This is Rodney's particles-p5.cpp.
 * File path: rodneydup/Assignment2/particles-p5.cpp. 
 * Rodney's original file shows a solar system in orbit around the sun
 * Stejara's changes: 1. mass relationships
 *                    2. textcoord
 *                    3. taking square root instead of distance cubed in gravity calculation
 *                    4. acceleration relationship between i and j ("gravity")
 */


#include "al/app/al_App.hpp"
#include "al/math/al_Random.hpp"
#include "al/ui/al_ControlGUI.hpp" // gui.draw(g)

using namespace al;

#include <fstream>
#include <vector>
using namespace std;

string slurp(string fileName); // forward declaration

struct AlloApp : App {
  // add more GUI here
  Parameter pointSize{"/pointSize", "", 0.6, "", 0.0, 2.0};
  Parameter timeStep{"/timeStep", "", 0.04, "", 0.001, 0.1};
  Parameter dragFactor{"/drag", "", 0.01, "", 0, 0.5};
  ParameterBool drawTrails{"/drawTrails", "", 1};
  Parameter asymmetry{"/asymmetry", "", 0.3, "", 0, 1};

  ControlGUI gui;

  ShaderProgram pointShader;
  Mesh mesh; // vector<Vec3f> position is inside mesh
  Texture trail;

  // typedef al::Vec<float, 3> Vec3f;
  // typedef std::vector<Vec3f> Vertices;

  //  simulation state
  vector<Vec3f> velocity;
  vector<Vec3f> acceleration;
  vector<float> mass;
  const float GM =
      1.190588106 * pow(10, -5); // Geocentric gravitational constant
                                 // (gravity per earth mass) in AU^3/s^2

  const float planetDistance[10] = {0, 5.20,  9.58, 19.2,  30.1,
                                    1, 0.722, 1.52, 0.387, 39.5};

  void onCreate() override {
    // add more GUI here
    gui << pointSize << timeStep << dragFactor << drawTrails;
    gui.init();
    navControl().useMouse(false);
    // compile shaders
    pointShader.compile(slurp("../point-vertex.glsl"),
                        slurp("../point-fragment.glsl"),
                        slurp("../point-geometry.glsl"));

    trail.filter(Texture::LINEAR);

    // set initial conditions of the simulation
    //

    mesh.primitive(Mesh::POINTS);
    // does 1000 work on your system? how many can you make before you get a low
    // frame rate? do you need to use <1000?
    // void reset() {

    reset();

    nav().pos(3, 0, 13);
  }

  void reset() {

    mesh.reset();
    velocity.clear();
    acceleration.clear();

    rnd::Random<> rng;
    rng.seed(42);

    auto rc = [&]() { return HSV(rng.uniform(), 1.0f, 1.0f); };
    auto rv = [&](float scale) -> Vec3f {
      return Vec3f(rng.uniformS(), rng.uniformS(), rng.uniformS()) * scale;
    };

    for (int i = 0; i < 1000; i++) {
      mesh.color(rc());

      float m = 0;

      if (i < 34) {
        m = rnd::uniform(33000);
      } else if (34 <= i < 100) {
        m = rnd::uniform(100) + 0.1;
      } else if (100 <= i < 500) {
        m = rnd::uniform(0.1) + 0.01;
      } else if (500 <= i < 1000) {
        m = rnd::uniform(0.00001) + 0.0000001;
      }

      mass.push_back(m);

      if (i == 0) {
        mesh.vertex(0);
        velocity.push_back(rv(0));
        acceleration.push_back(0);
      } else {
        float initialDistance =
            sqrt(i < 10 ? planetDistance[i] : rnd::uniform(30.0) + 1);
        mesh.vertex(initialDistance, 0, 0);
        float initialVelocity = pow(GM * mass[0] / initialDistance, 1.0 / 2);
        velocity.push_back(Vec3f(0, initialVelocity, 0));
        acceleration.push_back(0);
      }

        mesh.texCoord(pow(m, 1.0f / 3), 0); 
     }
  }

  bool freeze = false;
  void onAnimate(double dt) override {
    if (freeze)
      return;

    // ignore the real dt and set the time step;
    dt = timeStep;

    // Calculate gravitational force
    for (int i = 0; i < velocity.size(); i++) {
      for (int j = i + 1; j < velocity.size(); j++) {
        Vec3f r = mesh.vertices()[j] - mesh.vertices()[i];
        float distcubed = pow(r.mag(), -2.0);
        if (distcubed == 0)
          continue;
        Vec3f F = GM * mass[j] * mass[i] * r.normalize() / distcubed;
        acceleration[i] += F * float(asymmetry * 0.1);
        acceleration[j] -= F * float(asymmetry);
      }
    }

    // Integration
    vector<Vec3f> &position(mesh.vertices());
    for (int i = 0; i < velocity.size(); i++) {
      // "backward" Euler integration
      velocity[i] += acceleration[i] / mass[i] * dt;
      position[i] += velocity[i] * dt;

      // Explicit (or "forward") Euler integration would look like this:
      // position[i] += velocity[i] * dt;
      // velocity[i] += acceleration[i] / mass[i] * dt;
    }

    // clear all accelerations (IMPORTANT!!)
    for (auto &a : acceleration)
      a.zero();
  }

  bool onKeyDown(const Keyboard &k) override {
    if (k.key() == ' ') {
      freeze = !freeze;
    }

    if (k.key() == 'r') {
      reset();
    }

    return true;
  }

  void onDraw(Graphics &g) override {
    g.clear(0);
    if (drawTrails) {
      // 1. Match texture dimensions to window
      trail.resize(fbWidth(), fbHeight());
      // 2. Draw feedback texture.
      g.tint(0.98);
      g.quadViewport(trail, -1, -1, 2, 2); // non-transformed
      g.tint(1);                           // set tint back to 1
    }
    g.shader(pointShader);
    g.shader().uniform("pointSize", pointSize / 100);
    g.blending(true);
    g.blendModeTrans();
    g.depthTesting(true);
    g.draw(mesh);

    if (drawTrails) {
      g.depthTesting(false);
      trail.copyFrameBuffer(); // 3. Copy current (read) frame buffer to texture
    }

    imguiBeginFrame();
    ParameterGUI::beginPanel("Control");
    ParameterGUI::drawParameter(&pointSize);
    ParameterGUI::drawParameter(&timeStep);
    ParameterGUI::drawParameter(&dragFactor);
    ParameterGUI::drawParameterBool(&drawTrails);
    ParameterGUI::drawParameter(&asymmetry);

    ImGui::Text("Framerate %.3f", ImGui::GetIO().Framerate);

    ParameterGUI::endPanel();
    imguiEndFrame();
    imguiDraw();
  }
};

int main() { AlloApp().start(); }

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