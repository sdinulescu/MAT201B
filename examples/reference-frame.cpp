#include "al/app/al_App.hpp"
using namespace al;

struct MyApp : App {
    Mesh frame;
    void onCreate() override {
        frame.primitive(Mesh::LINES); // this primitive is pairwise (two points)

        //line from origin to red/x
        frame.vertex(0, 0, 0);
        frame.color(0, 0, 0);
        frame.vertex(1, 0, 0);
        frame.color(1, 0, 0);

        //line from origin to green/y
        frame.vertex(0, 0, 0);
        frame.color(0, 0, 0);
        frame.vertex(0, 1, 0);
        frame.color(0, 1, 0);

        //line from origin to blue/z
        frame.vertex(0, 0, 0);
        frame.color(0, 0, 0);
        frame.vertex(0, 0, 1);
        frame.color(0, 0, 1);

        //start the viewer somewhere more useful
        // nav().pos(0, 0, 5);

        //remember a good vantage point
        nav().pos(Vec3d(2.122773, 2.163705, 5.800607));
        nav().quat(Quatd(0.972931, -0.149372, 0.170585, 0.044662));
    }
    
    void onAnimate(double dt) override {
        // nav().print();
    }

    void onDraw(Graphics& g) override {
        g.clear(0.1);
        g.meshColor(); // use the color from the mesh, not the default color
        g.draw(frame);
    }

};

int main() {
    MyApp app;
    app.start();
}