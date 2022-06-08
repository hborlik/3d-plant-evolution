#version 330 core
uniform sampler2D normalTex;
uniform sampler2D colorTex;
uniform sampler2D shadowDepth;

uniform int normMapFlag;

out vec4 Outcolor;


in OUT_struct {
   vec3 fPos;
   vec3 fragLightDir;
    vec3 fragNor;
    vec3 fragTan;
    vec3 fragBN;
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
  vec3 normal = texture(normalTex, in_struct.vTexCoord).xyz;

  vec3 X = normalize(in_struct.fragTan);
  vec3 Y = normalize(in_struct.fragBN);
  vec3 Z = normalize(in_struct.fragNor);
  mat3 TBN = mat3(X, Y, Z);
  vec3 NMapped = TBN * normalize(2 * normal - vec3(1));

  Shade = TestShadow(in_struct.fPosLS);

  //TODO replace first set with normal mapped result when ready
  float dCoeff = 1.0;
  if (normMapFlag > 0)
    dCoeff = max(dot(NMapped, normalize(in_struct.fragLightDir)), 0.0);

  Outcolor = tColor*(amb) + (1.0-Shade)*(BaseColor * tColor * dCoeff);


  // figure 1
  // Outcolor = vec4(normal, 0.0);

  // figure 2
  // Outcolor = vec4(vec3(dCoeff), 1.0);

  // figure 3
  // Outcolor = vec4(normal, 1.0);

  // figure step 2
  // Outcolor = vec4(abs(in_struct.fragBN.x), abs(in_struct.fragBN.y), abs(in_struct.fragBN.z), 1.0);


  // Outcolor = vec4(X, 0.0);
  // Outcolor = BaseColor * (1.1-Shade);
  // Outcolor = (1.0-Shade)*vec4(dCoeff, 0, 0, 0.0);
  // Outcolor = vec4(normalize(in_struct.fragNor) * (normMapFlag + 0.1), 0.0);
}

