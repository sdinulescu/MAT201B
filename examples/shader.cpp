#include "al/app/al_App.hpp"
#include "al/graphics/al_Image.hpp"  // al::Image
#include "al/math/al_Random.hpp"
#include "al/ui/al_ControlGUI.hpp"

using namespace al;

#include <fstream>
#include <vector>
using namespace std;

//this is an example of basic or boilerplate vertex shader
const char* vertexCode = R"(
#version 400
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec4 vertexColor;
layout(location = 2) in vec2 vertexTexture;
uniform mat4 al_ModelViewMatrix;
uniform mat4 al_ProjectionMatrix;
void main() {
  gl_Position = al_ProjectionMatrix * al_ModelViewMatrix * vec4(vertexPosition, 1.0);
}
)";

//pixel by pixel color decisions
const char* fragmentCode = R"(
#version 400
layout(location = 0) out vec4 fragmentColor;
uniform vec3 color;
uniform float time;
void main() {
  // https://people.freedesktop.org/~idr/OpenGL_tutorials/03-fragment-intro.html
  float distance = 350 + sin(time * 3) * 500;
  vec2 pos = mod(gl_FragCoord.xy, vec2(50.0)) - vec2(25.0);
  float dist_squared = dot(pos, pos);
  fragmentColor = (dist_squared < distance) ? vec4(color, 1.0) : vec4(1 - color.r, color.g * color.g, 1 - color.b * color.b, 1.0);
}
)";

struct AlloApp : App {
  ParameterColor color{"/color"}; //acts as a chameleon
  ControlGUI gui;
  ShaderProgram pointShader;
  Mesh mesh;

  void onCreate() override {
    gui << color;
    gui.init();
    navControl().useMouse(false);

    mesh.primitive(Mesh::TRIANGLE_STRIP);
    mesh.vertex(-1, 1);
    mesh.vertex(-1, -1);
    mesh.vertex(1, 1);
    mesh.vertex(1, -1);

    pointShader.compile(vertexCode, fragmentCode);
    nav().pos(0, 0, 5);
  }

  double time = 0;
  void onAnimate(double dt) override {
      time += dt;
  }

  void onDraw(Graphics& g) override {
    g.clear(color);
    g.shader(pointShader);
    Color c(color);
    g.shader().uniform("color", Vec3f(1 - c.r, 1 - c.g, 1 - c.b));
    g.shader().uniform("time", (float)time);
    g.draw(mesh);
    gui.draw(g);
  }
};

int main() { AlloApp().start(); }
