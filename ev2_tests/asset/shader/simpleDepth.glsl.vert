#version  330 core

layout(location = 0) in vec3 vertPos;

uniform mat4 LPV;
uniform mat4 M;

void main() {

  /* transform into light space */
  gl_Position = LPV * M * vec4(vertPos.xyz, 1.0);
}
