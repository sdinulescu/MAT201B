#version 400

layout(points) in;
layout(triangle_strip, max_vertices = 31) out;

in Vertex {
  vec3 position;
  vec3 forward;
  vec3 up;
  vec4 color;
  int faceCount;
  float spikiness;
}
vertex[];

out Fragment {
  vec4 color;
}
fragment;

uniform mat4 al_ProjectionMatrix;
uniform mat4 al_ModelViewMatrix;

uniform float size;
uniform float ratio;

mat4 rotationMatrix(vec3 axis, float angle) {
  axis = normalize(axis);
  float s = sin(angle);
  float c = cos(angle);
  float oc = 1.0 - c;

  return mat4(
      oc * axis.x * axis.x + c, oc * axis.x * axis.y - axis.z * s,
      oc * axis.z * axis.x + axis.y * s, 0.0, oc * axis.x * axis.y + axis.z * s,
      oc * axis.y * axis.y + c, oc * axis.y * axis.z - axis.x * s, 0.0,
      oc * axis.z * axis.x - axis.y * s, oc * axis.y * axis.z + axis.x * s,
      oc * axis.z * axis.z + c, 0.0, 0.0, 0.0, 0.0, 1.0);
}

struct Base {
  vec4 position;
  vec4 color;
};

void make_triangle(vec4 nose, Base b1, Base b2, vec4 tail, float spikiness) {
  gl_Position = nose;
  fragment.color = vec4(1.0, 1.0, 1.0, 1.0);
  EmitVertex();

  gl_Position = b1.position;
  fragment.color = b1.color;
  EmitVertex();

  gl_Position = mix(b2.position, tail, spikiness);
  fragment.color = mix(b2.color, vec4(1.0), spikiness);
  EmitVertex();
}

const int N = 7;

void main() {
  mat4 pm = al_ProjectionMatrix * al_ModelViewMatrix;
  vec3 position = vertex[0].position;
  vec3 forward = vertex[0].forward * size;
  vec3 up = vertex[0].up;
  vec4 agentColor = vertex[0].color;

  vec3 over = cross(up, forward) * ratio;

  vec4 nose = pm * vec4(position + forward, 1.0);

  int n = vertex[0].faceCount > N ? N : vertex[0].faceCount;
  Base base[N];
  for (int i = 0; i < n; i++) {
    float angle = 360.0 * i / vertex[0].faceCount;
    base[i].position = pm * vec4(position + (rotationMatrix(forward, radians(angle)) * vec4(over, 0.0)).xyz * vertex[0].color.w, 1.0);
    base[i].color = agentColor;
  }

  vec4 tail = pm * vec4(position, 1.0);
  for (int i = 1; i < n; i++) {
    make_triangle(nose, base[i - 1], base[i], tail, vertex[0].spikiness);
  }
  make_triangle(nose, base[vertex[0].faceCount - 1], base[0], tail, vertex[0].spikiness);
}