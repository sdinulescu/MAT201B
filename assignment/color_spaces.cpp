#include "al/app/al_App.hpp"
#include "al/graphics/al_Image.hpp"  // al::Image
#include "al/math/al_Random.hpp"
#include "al/ui/al_ControlGUI.hpp"  // gui.draw(g)

using namespace al;

#include <fstream>
#include <vector>
using namespace std;

// string slurp(string fileName) {
//   fstream file(fileName);
//   string returnValue = "";
//   while (file.good()) {
//     string line;
//     getline(file, line);
//     returnValue += line + "\n";
//   }
//   return returnValue;
// }

const char* vertexCode = R"(
#version 400
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec4 vertexColor;
uniform mat4 al_ModelViewMatrix;
uniform mat4 al_ProjectionMatrix;
out Vertex { vec4 color; }
vertex;
void main() {
  gl_Position = al_ModelViewMatrix * vec4(vertexPosition, 1.0);
  vertex.color = vertexColor;
}
)";

const char* fragmentCode = R"(
#version 400
in Fragment { vec4 color; }
fragment;
layout(location = 0) out vec4 fragmentColor;
void main() {
  fragmentColor = fragment.color;
}
)";

const char* geometryCode = R"(
#version 400
// take in a point and output a triangle strip with 4 vertices (aka a "quad")
layout(points) in;
layout(triangle_strip, max_vertices = 4) out;
uniform mat4 al_ProjectionMatrix;
uniform float pointSize;
in Vertex { vec4 color; }
vertex[];
out Fragment { vec4 color; }
fragment;
void main() {
  mat4 m = al_ProjectionMatrix;   // rename to make lines shorter
  vec4 v = gl_in[0].gl_Position;  // al_ModelViewMatrix * gl_Position
  float r = pointSize;
  gl_Position = m * (v + vec4(-r, -r, 0.0, 0.0));
  fragment.color = vertex[0].color;
  EmitVertex();
  gl_Position = m * (v + vec4(r, -r, 0.0, 0.0));
  fragment.color = vertex[0].color;
  EmitVertex();
  gl_Position = m * (v + vec4(-r, r, 0.0, 0.0));
  fragment.color = vertex[0].color;
  EmitVertex();
  gl_Position = m * (v + vec4(r, r, 0.0, 0.0));
  fragment.color = vertex[0].color;
  EmitVertex();
  EndPrimitive();
}
)";

struct AlloApp : App {
  Parameter pointSize{"/pointSize", "", 0.21, "", 0.0, 1.0};
  ControlGUI gui;
  ShaderProgram pointShader;
  Mesh mesh;
  vector<Vec3f> original;

  void onCreate() override {
    //
    gui << pointSize;
    gui.init();
    navControl().useMouse(false);
    
    pointShader.compile(vertexCode, fragmentCode, geometryCode);

    mesh.primitive(Mesh::POINTS);

    const char* filename = "../windows.jpg";
    auto imageData = Image(filename);
    if (imageData.array().size() == 0) {
      std::cout << "failed to load image" << std::endl;
      exit(1);
    }

    Image::RGBAPix pixel;
    const int W = imageData.width();
    const int H = imageData.height();
    for (int c = 0; c < W; c++)
      for (int r = 0; r < H; r++) {
        imageData.read(pixel, c, r);
        Vec3f position((c - W / 2) * 0.005, (r - H / 2) * 0.005, 0);
        original.push_back(position);
        mesh.vertex(position);
        mesh.color(pixel.r / 255.0, pixel.g / 255.0, pixel.b / 255.0);
      }

    nav().pos(0, 0, 10);
  }

  bool freeze = false;
  void onAnimate(double dt) override {
    if (freeze) return;

    // c++11 "lambda" function
    auto rv = []() {
      return Vec3f(rnd::uniformS(), rnd::uniformS(), rnd::uniformS());
    };

    // range-for to do a random walk for each vertex
    for (auto& v : mesh.vertices()) {
      //
      v += rv() * 0.01;
    }
  }

  bool onKeyDown(const Keyboard& k) override {
    if (k.key() == ' ') {
      freeze = !freeze;
    }

    if (k.key() == '1') { //back to original image
      for (int i = 0; i < mesh.vertices().size(); i++) {
        mesh.vertices()[i] = original[i];
      }
    }

    if (k.key() == '2') { //rgb cube
        // this doesn't look like a cube ... does it actually need to look like a cube?
        for (int i = 0; i < mesh.colors().size(); i++) { //loop through the colors
            //get the pixel color value
            Vec3f col(mesh.colors()[i].r, mesh.colors()[i].g, mesh.colors()[i].b);
            //assign a position based on the color
            mesh.vertices()[i] = col;
            //printf("%f", mesh.colors()[i].rgb());
        }
    }

    if (k.key() == '3') { //hsv cylinder
        for (int i = 0; i < mesh.colors().size(); i++) {

        }
    }

     if (k.key() == '4') { //own thing
        for (int i = 0; i < mesh.vertices().size(); i++) {

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