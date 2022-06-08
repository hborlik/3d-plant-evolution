#version 330 core
uniform sampler2D normalTex;
uniform sampler2D colorTex;
uniform sampler2D shadowDepth;

uniform int normMapFlag;

out vec4 Outcolor;
in vec3 lightDir;


in OUT_struct {
   vec3 fPos;
   vec3 fragNor;
   vec2 vTexCoord;
   vec4 fPosLS;
   vec3 vColor;
} in_struct;

/* returns 1 if shadowed */
/* called with the point projected into the light's coordinate space */
float TestShadow(vec4 LSfPos) {

  float bias = 0.03;
  //1: shift the coordinates from -1, 1 to 0 ,1
  vec3 shift = (LSfPos.xyz + vec3(1.0))/2.0;  
  //2: read off the stored depth (.) from the ShadowDepth, using the shifted.xy 
  float curD = shift.z;
  float lightD = texture(shadowDepth, shift.xy).r;
  //3: compare to the current depth (.z) of the projected depth
  //4: return 1 if the point is shadowed
  if (curD - bias > lightD)
    return 1.0;
  else
    return 0.0;
}

void main() {

  float Shade;
  float amb = 0.3;
  float shine = 100.0;

  vec4 BaseColor = vec4(in_struct.vColor, 1);
  vec4 tColor = texture(colorTex, in_struct.vTexCoord);
  vec4 normal = texture(normalTex, in_struct.vTexCoord);

  Shade = TestShadow(in_struct.fPosLS);

  //TODO replace first set with normal mapped result when ready
  float dCoeff = BaseColor.x;
  if (normMapFlag < 1)
    dCoeff =  BaseColor.x;

  Outcolor = amb*tColor + (1.0-Shade)*(BaseColor*tColor);
}

