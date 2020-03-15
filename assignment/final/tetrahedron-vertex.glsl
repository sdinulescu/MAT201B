#version 400

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 tex;
layout(location = 3) in vec3 normal;
layout(location = 4) in int numFaces;

out Vertex {
  vec3 position;
  vec3 forward;
  vec3 up;
  vec4 color;
  int numFaces;
}
vertex;

void main() {
  vertex.position = position;
  vertex.forward = normal;
  vertex.up = color.rgb;
  vertex.color = color;
  vertex.numFaces = numFaces;
}
