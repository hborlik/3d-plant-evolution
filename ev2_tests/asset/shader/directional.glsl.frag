#include "globals.glslinc"
#include "disney.glslinc"

out vec4 frag_color;

in vec2 tex_coord;
in vec4 fPosLS;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform usampler2D gMaterialTex;
uniform sampler2D gAO;
uniform sampler2D shadowDepth;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 lightAmbient;


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

// BRDF shader interface

void main() {
    vec3 FragPos = texture(gPosition, tex_coord).rgb;
    vec3 Normal = texture(gNormal, tex_coord).rgb;
    vec3 Albedo = texture(gAlbedoSpec, tex_coord).rgb;
    float Specular = texture(gAlbedoSpec, tex_coord).a;
    uint MaterialId = texture(gMaterialTex, tex_coord).r;
    float AO = texture(gAO, tex_coord).r;

    if (FragPos == vec3(0, 0, 0)) // no rendered geometry
        discard;

    // constant tangent spaces
    vec3 X = vec3(1, 0, 0);
    vec3 Y = normalize(cross(Normal, X));
    X = normalize(cross(Y, Normal));

    // const vec3 vLightPos = vec3(View * vec4(lightPos, 1.0f)); 
    // float lD = length(vLightPos - FragPos);
    // vec3 lightDir = normalize(vLightPos - FragPos);
    // float attenuation = 1.0 / (0.9 * lD + 0.1 * lD*lD);
    vec3 lightDirV = vec3(View * vec4(lightDir, .0f));

    vec3 viewDir = normalize(-FragPos);

    vec3 color = AO * lightAmbient * (Albedo + materials[MaterialId].diffuse) + 1.0 * lightColor * BRDF(lightDirV, viewDir, Normal, X, Y, Albedo, materials[MaterialId]);
    // fake hdr
    color = color / (color + vec3(1.0)); // function asymptote y = 1 (maps to LDR range of [0, 1])
    // gamma
    color = pow(color, vec3(1.0/2.2));

//    frag_color = vec4(color, 1.0);
    // frag_color = vec4(AO, AO, AO, 1.0);

  float Shade = TestShadow(in_struct.fPosLS);

  //TODO replace first set with normal mapped result when ready
//  float dCoeff = max(0.0, dot(normalize(lightDirV), 2*(Normal.xyz) - vec3(1.0))); //;
   Outcolor = vec4(color, 1.0) + (1.0 - Shade)*vec4(color, 1.0);

  dCoeff =  BaseColor.x;    

}