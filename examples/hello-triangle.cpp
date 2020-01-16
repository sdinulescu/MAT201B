#include "al/app/al_App.hpp"
using namespace al;

struct MyApp : App {
  Mesh triangle;

  void onCreate() override {
    triangle.vertex(0, 1, 0);
    triangle.color(1, 0, 0);
    triangle.vertex(1, 0, 0);
    triangle.color(0, 1, 0);
    triangle.vertex(-1, 0, 0);
    triangle.color(0, 0, 1);

    nav().pos(0, 0, 5);
  }

  void onAnimate(double dt) override {

  }

  void onDraw(Graphics& g) override {
      g.clear(0.1);
      g.meshColor();
      g.draw(triangle);
  }

};

int main() {
  MyApp app;
  app.start();
}
