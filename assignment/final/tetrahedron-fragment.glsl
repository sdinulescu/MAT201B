#version 400
// Written by Karl Yerkes

in Fragment {
  vec4 color;
}
fragment;

layout(location = 0) out vec4 fragmentColor;

void main() {
  fragmentColor = fragment.color;
}
