#include "globals.glslinc"
out float FragColor;
  
in vec2 tex_coord;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D texNoise;

uniform float radius;
uniform float bias;
uniform uint nSamples;

layout (std140, binding = 1) uniform Samples {
    vec3 samples[64];
};

void main() {
    const vec2 noiseScale = textureSize(gPosition, 0) * (1.0 / 4.0);
    vec3 fragPos   = texture(gPosition, tex_coord).xyz;
    vec3 normal    = texture(gNormal, tex_coord).rgb;

    vec3 randomVec = texture(texNoise, tex_coord * noiseScale).xyz;

    vec3 tangent   = normalize(randomVec - normal * dot(randomVec, normal)); // take perpendicular part of random vec
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN       = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    for(int i = 0; i < nSamples; ++i)
    {
        // get sample position
        vec3 samplePos = TBN * samples[i]; // from tangent to view-space
        samplePos = fragPos + samplePos * radius;
        
        vec4 offset = vec4(samplePos, 1.0);
        offset      = P * offset;    // from view to clip-space
        offset.xyz /= offset.w;               // perspective divide
        offset.xyz  = offset.xyz * 0.5 + vec3(0.5); // transform to range 0.0 - 1.0

        float sampleDepth = texture(gPosition, offset.xy).z;
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(fragPos.z - sampleDepth));
        occlusion       += (sampleDepth > samplePos.z + bias ? 1.0 : 0.0) * rangeCheck;
    }
    occlusion = 1.0 - (occlusion / nSamples);
    FragColor = occlusion;
}