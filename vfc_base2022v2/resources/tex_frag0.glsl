#version 330 core

uniform sampler2D Texture0;
uniform float MatShine;

uniform int flip;

in vec2 vTexCoord;

out vec4 Outcolor;

//interpolated normal and light vector in camera space
in vec3 fragNor;
in vec3 lightDir;
//position of the vertex in camera space
in vec3 EPos;

void main() {
  vec4 texColor0 = texture(Texture0, vTexCoord);

  vec3 normal = normalize(fragNor);
  if (flip < 1) {
  	normal *= -1.0f;
  }
  vec3 light = normalize(lightDir);
  float dC = max(0, dot(normal, light));
  Outcolor = vec4(dC*texColor0.xyz, 1.0);

  //to confirm texture coordinates
  //Outcolor = vec4(vTexCoord.x, vTexCoord.y, 0, 0);
}

