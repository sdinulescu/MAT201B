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

    const char* filename = "../colorful2.png";
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

    if (k.key() == '3') { //hsv cylinder -> conversions taken from https://en.wikipedia.org/wiki/HSL_and_HSV
        // find max vals for conversions
        double minR, maxR = 0; 
        //mesh.colors()[0].r;
        double minG, maxG = 0;
        //mesh.colors()[0].g;
        double minB, maxB = 0;
        //mesh.colors()[0].b;
        for (auto& c : mesh.colors()) { 
            if (c.r >= maxR) { maxR = c.r; }
            if (c.g >= maxG) { maxG = c.g; }
            if (c.b >= maxB) { maxB = c.b; }
            
            if (c.r <= minR) { minR = c.r; } 
            if (c.g <= minG) { minG = c.g; } 
            if (c.b <= minB) { minB = c.b; }
        }
        
        //calculate total max vals
        double totMax = (maxR + maxG + maxB) / 3;
        double totMin = (minR + minG + minB)  /  3;

        std::cout << minR << " " << maxR << " " << minG << " " << maxG << " " << minB << " " << maxB << " " << totMax << " " << totMin << std::endl;
        
        for (int i = 0; i < mesh.colors().size(); i++) {
            Vec3f col;
            //handle x (hue)
            if (mesh.colors()[i].r == mesh.colors()[i].g == mesh.colors()[i].b == 0) {
                std::cout << "black" << endl;
                col = (0, 0, 0);
            } else if (mesh.colors()[i].r == totMax) {
                std::cout << "maxRed" << endl;
                col.x = 60 * (0 + (mesh.colors()[i].g - mesh.colors()[i].b)/(totMax - totMin));
            } else if (mesh.colors()[i].g == totMax) {
                std::cout << "maxGreen" << endl;
                col.x = 60 * (2 + (mesh.colors()[i].b - mesh.colors()[i].r)/(totMax - totMin));
            } else if (mesh.colors()[i].b == totMax) {
                std::cout << "maxBlue" << endl;
                col.x = 60 * (4 + (mesh.colors()[i].r - mesh.colors()[i].g)/(totMax - totMin));
            } else { std::cout << "nothing" << endl; }

            // if (mesh.colors()[i].r == mesh.colors()[i].g == mesh.colors()[i].b == 0) {
            //     //std::cout << "black" << endl;
            //     col = (0, 0, 0);
            // } else if (mesh.colors()[i].r > mesh.colors()[i].g && mesh.colors()[i].r > mesh.colors()[i].b) {
            //     //std::cout << "maxRed" << endl;
            //     col.x = 60 * (0 + (mesh.colors()[i].g - mesh.colors()[i].b)/(totMax - totMin));
            // } else if (mesh.colors()[i].g > mesh.colors()[i].r && mesh.colors()[i].g > mesh.colors()[i].b) {
            //     //std::cout << "maxGreen" << endl;
            //     col.x = 60 * (2 + (mesh.colors()[i].b - mesh.colors()[i].r)/(totMax - totMin));
            // } else if (mesh.colors()[i].b > mesh.colors()[i].r && mesh.colors()[i].b > mesh.colors()[i].g) {
            //     //std::cout << "maxBlue" << endl;
            //     col.x = 60 * (4 + (mesh.colors()[i].r - mesh.colors()[i].g)/(totMax - totMin));
            // } else { std::cout << "nothing" << endl; }

            if (col.x < 0) {
                col.x = col.x + 360;
            }

            //handle y (s) and z (v)
            if (! mesh.colors()[i].r == mesh.colors()[i].g == mesh.colors()[i].b == 0 ) {
                col.y = (totMax - totMin)/totMax;
                col.z = totMax;
            }


            //std::cout << col.x << " " << col.y << " " << col.z << " ";
            // if (mesh.colors()[i].r == mesh.colors()[i].g == mesh.colors()[i].b == 0) {
            //     col.y = 0;
            // } else if (mesh.colors()[i].r == maxR){
            //     col.y = (maxR - minR)/maxR ;
            //     //col.y = (totMax - totMin)/totMax ;
            // } else if (mesh.colors()[i].g == maxG){
            //     col.y = (maxG - minG)/maxG ;
            // } else if (mesh.colors()[i].b == maxB){
            //     col.y = (maxB - minB)/maxB ;
            // }

            //handle z (v)
            // if (mesh.colors()[i].r == mesh.colors()[i].g == mesh.colors()[i].b == 0) {
            //     col.z = 0;
            // } else if (mesh.colors()[i].r == maxR){
            //     col.z = maxR;
            //     //col.y = (totMax - totMin)/totMax ;
            // } else if (mesh.colors()[i].g == maxG){
            //     col.z = maxG;
            // } else if (mesh.colors()[i].b == maxB){
            //     col.z = maxB;
            // }

            mesh.vertices()[i] = col;
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