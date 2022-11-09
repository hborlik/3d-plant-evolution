#version 460 core
#extension GL_GOOGLE_include_directive : enable

#include "globals.glslinc"
#include "disney.glslinc"

out vec4 frag_color;

in vec2 tex_coord;

uniform mat4 LS;
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
float TestShadow(vec3 LSfPos, vec3 Normal) {
    // pcss
    vec3 X = vec3(1, 0, 0);
    vec3 Y = normalize(cross(Normal, X));
    X = normalize(cross(Y, Normal));

    mat3 TBN = mat3(X, Y, Normal);

    vec2 texelScale = 1.0 / textureSize(shadowDepth, 0);
    float percentShadow = 0.0f;
    // 5x5 sampling
    for (int i = -2; i <= 2; i++)
        for (int j = -2; j <= 2; j++) {
            const vec3 spos = TBN * vec3(vec2(i, j) * texelScale, 0);
            const float lightDepth = texture(shadowDepth, LSfPos.xy + spos.xy).r;
            if (LSfPos.z - spos.z * 1.5f > lightDepth)
                percentShadow += 1.0f;
        }

    return percentShadow / 25.0f;
}

// BRDF shader interface

void main() {
    vec3 FragPos = texture(gPosition, tex_coord).rgb;
    if (FragPos == vec3(0, 0, 0)) // no rendered geometry
        discard;

    vec3 Normal = texture(gNormal, tex_coord).rgb;
    vec3 Albedo = texture(gAlbedoSpec, tex_coord).rgb;
    float Specular = texture(gAlbedoSpec, tex_coord).a;
    uint MaterialId = texture(gMaterialTex, tex_coord).r;
    float AO = texture(gAO, tex_coord).r;

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

    mat4 inv_pv = LS * VInv;
    float Shade = TestShadow((inv_pv * vec4(FragPos, 1.0)).xyz, (inv_pv * vec4(Normal, 0.0)).xyz);

    vec3 color = AO * lightAmbient * (Albedo + materials[MaterialId].diffuse) + (1.0 - Shade) * lightColor * BRDF(lightDirV, viewDir, Normal, X, Y, Albedo, materials[MaterialId]);
    
    frag_color = vec4(color, 1.0);
    // frag_color = vec4(AO, AO, AO, 1.0);
}