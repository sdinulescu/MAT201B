#include "al/app/al_App.hpp"
#include "al/graphics/al_Image.hpp"
using namespace al;

struct MyApp : App {
  Mesh mesh;

  void onCreate() override {
    mesh.primitive(Mesh::POINTS);

    const char* filename = "../Drake-Equation.jpg";
    // working directory that code runs in is "bin" folder -> need to back up when accessing the image
    auto imageData = Image(filename);
    if (imageData.array().size() == 0) {
      std::cout << "failed to load image" << std::endl;
    }

    Image::RGBAPix pixel; // make sure image is less than 1000*1000 pixels
    for (int c = 0; c < imageData.width(); c++) {
      for (int r = 0; r < imageData.height(); r++) {
        imageData.read(pixel, c, r);
        mesh.vertex(c * 0.1, r * 0.1, 0);
        mesh.color(pixel.r / 255.0, pixel.g / 255.0, pixel.b / 255.0);
      }
    }

    nav().pos(0, 0, 5);
  }

  void onAnimate(double dt) override {

  }

  void onDraw(Graphics& g) override {
      g.clear(0.1);
      g.meshColor();
      g.draw(mesh);
  }

};

// 1. Returns the points to the row/column positions of each pixel in the original image
// 2. Places each point in an [RGB color space "cube" according to its color
// 3. Builds an HSV color space "cylinder" with the points
// 4. Animation -> this is a linear interpolation in space

int main() {
  MyApp app;
  app.start();
}
