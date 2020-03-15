#version 400

layout(points) in;
layout(triangle_strip, max_vertices = 9) out;

in Vertex {
  vec3 position;
  vec3 forward;
  vec3 up;
  vec4 color;
  int numFaces;
}
vertex[];

out Fragment {  //
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

void main() {
  mat4 pm = al_ProjectionMatrix * al_ModelViewMatrix;
  vec3 position = vertex[0].position;
  vec3 forward = vertex[0].forward * size / (0.1 + vertex[0].color.w); 
  vec3 up = vertex[0].up;
  vec4 color = vertex[0].color;

  vec3 over = cross(up, forward) * ratio;

  if (vertex[0].numFaces == 3) {
    vec4 a = pm * vec4(position + forward, 1.0);
    vec4 b =
        pm *
        vec4(position +
                (rotationMatrix(forward, radians(60)) * vec4(over, 0.0)).xyz,
            1.0f);
    vec4 c =
        pm *
        vec4(position +
                (rotationMatrix(forward, radians(180)) * vec4(over, 0.0)).xyz,
            1.0f);
    vec4 d =
        pm *
        vec4(position +
                (rotationMatrix(forward, radians(300)) * vec4(over, 0.0)).xyz,
            1.0f);

    gl_Position = a;
    fragment.color = color;
    EmitVertex();

    gl_Position = b;
    fragment.color = color;
    EmitVertex();

    gl_Position = c;
    fragment.color = color;
    EmitVertex();

    EndPrimitive();

    gl_Position = a;
    fragment.color = color;
    EmitVertex();

    gl_Position = c;
    fragment.color = color;
    EmitVertex();

    gl_Position = d;
    fragment.color = color;
    EmitVertex();

    EndPrimitive();

    gl_Position = a;
    fragment.color = color;
    EmitVertex();

    gl_Position = d;
    fragment.color = color;
    EmitVertex();

    gl_Position = b;
    fragment.color = color;
    EmitVertex();

    EndPrimitive();
  } else if (vertex[0].numFaces == 4) {
    vec4 a = pm * vec4(position + forward, 1.0);
    vec4 b =
        pm *
        vec4(position +
                (rotationMatrix(forward, radians(30)) * vec4(over, 0.0)).xyz,
            1.0f);
    vec4 c =
        pm *
        vec4(position +
                (rotationMatrix(forward, radians(120)) * vec4(over, 0.0)).xyz,
            1.0f);
    vec4 d =
        pm *
        vec4(position +
                (rotationMatrix(forward, radians(210)) * vec4(over, 0.0)).xyz,
            1.0f);

    vec4 e =
    pm *
    vec4(position +
              (rotationMatrix(forward, radians(300)) * vec4(over, 0.0)).xyz,
          1.0f); 

    gl_Position = a;
    fragment.color = color;
    EmitVertex();

    gl_Position = b; 
    fragment.color = color;
    EmitVertex();

    gl_Position = c;
    fragment.color = color;
    EmitVertex();

    EndPrimitive();

    gl_Position = a;
    fragment.color = color;
    EmitVertex();

    gl_Position = c; 
    fragment.color = color;
    EmitVertex();

    gl_Position = d;
    fragment.color = color;
    EmitVertex();

    EndPrimitive();

    gl_Position = a;
    fragment.color = color;
    EmitVertex();

    gl_Position = d; 
    fragment.color = color;
    EmitVertex();

    gl_Position = e;
    fragment.color = color;
    EmitVertex();

    EndPrimitive();

    gl_Position = a;
    fragment.color = color;
    EmitVertex();

    gl_Position = b; 
    fragment.color = color;
    EmitVertex();

    gl_Position = e;
    fragment.color = color;
    EmitVertex();

    EndPrimitive();
  } else if (vertex[0].numFaces == 5) {
    vec4 a = pm * vec4(position + forward, 1.0);
    vec4 b =
        pm *
        vec4(position +
                (rotationMatrix(forward, radians(0)) * vec4(over, 0.0)).xyz,
            1.0f);
    vec4 c =
        pm *
        vec4(position +
                (rotationMatrix(forward, radians(108)) * vec4(over, 0.0)).xyz,
            1.0f);
    vec4 d =
        pm *
        vec4(position +
                (rotationMatrix(forward, radians(216)) * vec4(over, 0.0)).xyz,
            1.0f);
    vec4 e =
    pm *
    vec4(position +
              (rotationMatrix(forward, radians(324)) * vec4(over, 0.0)).xyz,
          1.0f); 

    vec4 f =
    pm *
    vec4(position +
              (rotationMatrix(forward, radians(432)) * vec4(over, 0.0)).xyz,
          1.0f); 

    gl_Position = a;
    fragment.color = color;
    EmitVertex();

    gl_Position = b; 
    fragment.color = color;
    EmitVertex();

    gl_Position = c;
    fragment.color = color;
    EmitVertex();

    EndPrimitive();

    gl_Position = a;
    fragment.color = color;
    EmitVertex();

    gl_Position = c; 
    fragment.color = color;
    EmitVertex();

    gl_Position = d;
    fragment.color = color;
    EmitVertex();

    EndPrimitive();

    gl_Position = a;
    fragment.color = color;
    EmitVertex();

    gl_Position = d; 
    fragment.color = color;
    EmitVertex();

    gl_Position = e;
    fragment.color = color;
    EmitVertex();

    EndPrimitive();

    gl_Position = a;
    fragment.color = color;
    EmitVertex();

    gl_Position = e; 
    fragment.color = color;
    EmitVertex();

    gl_Position = f;
    fragment.color = color;
    EmitVertex();

    EndPrimitive();

    gl_Position = a;
    fragment.color = color;
    EmitVertex();

    gl_Position = f; 
    fragment.color = color;
    EmitVertex();

    gl_Position = b;
    fragment.color = color;
    EmitVertex();

    EndPrimitive();
  } else {
    vec4 a = pm * vec4(position + forward, 1.0);
    vec4 b =
        pm *
        vec4(position +
                (rotationMatrix(forward, radians(0)) * vec4(over, 0.0)).xyz,
            1.0f);
    vec4 c =
        pm *
        vec4(position +
                (rotationMatrix(forward, radians(120)) * vec4(over, 0.0)).xyz,
            1.0f);
    vec4 d =
        pm *
        vec4(position +
                (rotationMatrix(forward, radians(240)) * vec4(over, 0.0)).xyz,
            1.0f);
    vec4 e =
    pm *
    vec4(position +
              (rotationMatrix(forward, radians(360)) * vec4(over, 0.0)).xyz,
          1.0f); 
    vec4 f =
    pm *
    vec4(position +
              (rotationMatrix(forward, radians(480)) * vec4(over, 0.0)).xyz,
          1.0f); 
    vec4 g =
    pm *
    vec4(position +
              (rotationMatrix(forward, radians(600)) * vec4(over, 0.0)).xyz,
          1.0f); 

    gl_Position = a;
    fragment.color = color;
    EmitVertex();

    gl_Position = b; 
    fragment.color = color;
    EmitVertex();

    gl_Position = c;
    fragment.color = color;
    EmitVertex();

    EndPrimitive();

    gl_Position = a;
    fragment.color = color;
    EmitVertex();

    gl_Position = c; 
    fragment.color = color;
    EmitVertex();

    gl_Position = d;
    fragment.color = color;
    EmitVertex();

    EndPrimitive();

    gl_Position = a;
    fragment.color = color;
    EmitVertex();

    gl_Position = d; 
    fragment.color = color;
    EmitVertex();

    gl_Position = e;
    fragment.color = color;
    EmitVertex();

    EndPrimitive();

    gl_Position = a;
    fragment.color = color;
    EmitVertex();

    gl_Position = e; 
    fragment.color = color;
    EmitVertex();

    gl_Position = f;
    fragment.color = color;
    EmitVertex();

    EndPrimitive();

    gl_Position = a;
    fragment.color = color;
    EmitVertex();

    gl_Position = f; 
    fragment.color = color;
    EmitVertex();

    gl_Position = g;
    fragment.color = color;
    EmitVertex();

    EndPrimitive();

    gl_Position = a;
    fragment.color = color;
    EmitVertex();

    gl_Position = g; 
    fragment.color = color;
    EmitVertex();

    gl_Position = b;
    fragment.color = color;
    EmitVertex();

    EndPrimitive();
  }
}
