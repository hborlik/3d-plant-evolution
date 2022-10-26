#include "globals.glslinc"
#include "disney.glslinc"
#include "point_lighting.glslinc"

out vec4 frag_color;

flat in uint instance_id;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform usampler2D gMaterialTex;

layout(std140, binding = 3) buffer lights_in {
    PointLight lights[];
};

// BRDF shader interface

void main() {
    const vec2 tex_coord = gl_FragCoord.xy / textureSize(gPosition, 0);
    vec3 FragPos = texture(gPosition, tex_coord).rgb;
    if (FragPos == vec3(0, 0, 0)) // no rendered geometry
        discard;
    const vec3 vLightPos = vec3(View * vec4(lights[instance_id].position, 1.0f));
    float lD = length(vLightPos - FragPos);
    if (lD > lights[instance_id].radius)
        discard;


    vec3 Normal = texture(gNormal, tex_coord).rgb;
    vec3 Albedo = texture(gAlbedoSpec, tex_coord).rgb;
    float Specular = texture(gAlbedoSpec, tex_coord).a;
    uint MaterialId = texture(gMaterialTex, tex_coord).r;

    // constant tangent spaces
    vec3 X = vec3(1, 0, 0);
    vec3 Y = normalize(cross(Normal, X));
    X = normalize(cross(Y, Normal));

    vec3 lightDir = normalize(vLightPos - FragPos);
    float attenuation = 1.0 / (lights[instance_id].k_c + lights[instance_id].k_l * lD + lights[instance_id].k_q * lD*lD);

    vec3 viewDir = normalize(-FragPos);

    vec3 color = attenuation * lights[instance_id].lightColor * BRDF(lightDir, viewDir, Normal, X, Y, Albedo, materials[MaterialId]);
    // fake hdr
    // color = color / (color + vec3(1.0)); // function asymptote y = 1 (maps to LDR range of [0, 1])
    // gamma
    // color = pow(color, vec3(1.0/2.2));

    frag_color = vec4(color, 1.0);
}