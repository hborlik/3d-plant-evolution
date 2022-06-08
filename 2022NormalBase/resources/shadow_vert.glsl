#version  330 core

layout(location = 0) in vec3 vertPos;
layout(location = 1) in vec3 vertNor;
layout(location = 2) in vec2 vertTex;
layout(location = 3) in vec3 vertTan;
layout(location = 4) in vec3 vertBN;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform mat4 LS;

uniform vec3 lightDir;

out OUT_struct {
	vec3 fPos;
  vec3 fragLightDir;
	vec3 fragNor;
  vec3 fragTan;
  vec3 fragBN;
	vec2 vTexCoord;
	vec4 fPosLS;
	vec3 vColor;
} out_struct;

void main() {

  /* First model transforms */
  gl_Position = P * V * M * vec4(vertPos.xyz, 1.0);

	/* the position in world coordinates */
  out_struct.fPos = (M*vec4(vertPos, 1.0)).xyz;
	/* the normal */
  out_struct.fragNor = (V *M*vec4(vertNor, 0.0)).xyz;
  out_struct.fragTan = (V *M*vec4(vertTan, 0.0)).xyz;
  out_struct.fragBN = (V *M*vec4(vertBN, 0.0)).xyz;
  out_struct.fragLightDir = (V * vec4(lightDir, 0.0)).xyz;
  /* pass through the texture coordinates to be interpolated */
  out_struct.vTexCoord = vertTex;
  /* The vertex in light space TODO: fill in appropriately*/
  out_struct.fPosLS = LS*vec4(out_struct.fPos, 1.0);
  /* a color that could be blended - or be shading */
  out_struct.vColor = vec3(max(dot(normalize(out_struct.fragNor), normalize(out_struct.fragLightDir)), 0));
}
