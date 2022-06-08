#version 330 core
uniform sampler2D Texture0;
uniform sampler2D shadowDepth;

out vec4 Outcolor;

in OUT_struct {
   vec3 fPos;
   vec3 fragNor;
   vec2 vTexCoord;
   vec4 fPosLS;
   vec3 vColor;
} in_struct;

/* returns 1 if shadowed */
/* called with the point projected into the light's coordinate space */
float TestShadow(vec4 LSfPos, vec3 nor) {

	//1: shift the coordinates from -1, 1 to 0 ,1
	//2: read off the stored depth (.) from the ShadowDepth, using the shifted.xy 
	//3: compare to the current depth (.z) of the projected depth
	//4: return 1 if the point is shadowed
  // vec3 fpos = 0.5*(LSfPos.xyz + vec3(1.0));
  vec2 texelScale = 1.0 / textureSize(shadowDepth, 0);
  float percentShadow = 0.0f;
  for (int i=-2; i <= 2; i++) { 
    for (int j=-2; j <= 2; j++) { 
      float lightDepth = texture(shadowDepth, LSfPos.xy+vec2(i, j)*texelScale).r; 
      if (LSfPos.z > lightDepth) 
        percentShadow += 1.0; 
    } 
  } 
  return percentShadow/25.0;
}

void main() {

  float Shade;
  float amb = 0.3;

  vec4 BaseColor = vec4(in_struct.vColor, 1);
  vec4 texColor0 = texture(Texture0, in_struct.vTexCoord);

  Shade = TestShadow(in_struct.fPosLS, in_struct.fragNor);

  Outcolor = amb*(texColor0) + (1.0-Shade)*texColor0*BaseColor;
}

