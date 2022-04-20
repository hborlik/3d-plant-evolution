#version 460 core
out vec3 frag_color;

layout (std140) uniform Globals {
    mat4 V;
    mat4 P;
    vec3 CameraPos;
    uint NumLights;
    vec3 LightPositions[10];
    vec3 LightColors[10];
};

uniform ShaderData {
    float lightFarPlane;
    float lightNearPlane;
};

in vec3 world_pos;
in vec3 vert_nor;
in vec3 vert_tan;
in vec3 vert_bitan;
in vec2 tex_pos;

uniform sampler2D albedoMap;
uniform sampler2D normalMap;
uniform sampler2D metallicMap;
uniform sampler2D roughnessMap;
uniform sampler2D aoMap;

// one for each light
uniform samplerCube lightDepthTex[10];

const float PI = 3.14159265359;

// radius of disk in world space // 0.4 to 0.05
uniform float SamplingRadius;
// samples [4, 10]
const uint Samples = 5;
const float SampleStep = 2*PI/(Samples+1);
// disk of directions for sampling cubemap
const vec2 sampleOffsets[10] = vec2[](
    vec2(cos(SampleStep), sin(SampleStep)),
    vec2(cos(2*SampleStep), sin(2*SampleStep)),
    vec2(cos(3*SampleStep), sin(3*SampleStep)),
    vec2(cos(4*SampleStep), sin(4*SampleStep)),
    vec2(cos(5*SampleStep), sin(5*SampleStep)),
    vec2(cos(6*SampleStep), sin(6*SampleStep)),
    vec2(cos(7*SampleStep), sin(7*SampleStep)),
    vec2(cos(8*SampleStep), sin(8*SampleStep)),
    vec2(cos(9*SampleStep), sin(9*SampleStep)),
    vec2(cos(10*SampleStep), sin(10*SampleStep))
);

vec3 normalFromNormalMap(vec2 texPos, mat3 T);

float DistributionGGX(vec3 N, vec3 H, float roughness);
float GeometrySchlickGGX(float NdotV, float roughness);
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 F0);

// z in normal direction, x is tangent, y is bitangent
float checkShadow(uint light, vec3 normal, mat3 TanS);

void main() {
    vec3 N = normalize(vert_nor);
    vec3 view = normalize(CameraPos - world_pos);

    // tangent transform
    mat3 TanS = mat3(normalize(vert_tan), normalize(vert_bitan), N);

    // get PBR inputs
    vec3 albedo     = pow(texture(albedoMap, tex_pos).rgb, vec3(2.2));
    vec3 normal     = normalFromNormalMap(tex_pos, TanS);
    float metallic  = texture(metallicMap, tex_pos).r;
    float roughness = texture(roughnessMap, tex_pos).r;
    float ao        = texture(aoMap, tex_pos).r;

    // lighting
    vec3 Lo = vec3(0);
    for(uint i = 0; i < NumLights; ++i) {
        vec3 ldir = normalize(LightPositions[i] - world_pos);
        vec3 h = normalize(view + ldir);

        float ldist = length(LightPositions[i] - world_pos);
        float attenuation = 1.0 / (ldist*ldist);
        vec3 radiance = LightColors[i] * attenuation;

        vec3 F0 = vec3(0.04); 
        F0      = mix(F0, albedo, metallic);
        vec3 F  = fresnelSchlick(max(dot(h, view), 0.0), F0);

        float NDF = DistributionGGX(normal, h, roughness);       
        float G   = GeometrySmith(normal, view, ldir, roughness);  

        vec3 numerator    = NDF * G * F;
        float denominator = 4.0 * max(dot(normal, view), 0.0) * max(dot(normal, ldir), 0.0);
        vec3 specular     = numerator / max(denominator, 0.001);  

        vec3 kS = F;
        vec3 kD = vec3(1.0) - kS;
  
        kD *= 1.0 - metallic; // subtract metallic metalic surfaces do not have diffuse lighting

        float NdotL = max(dot(normal, ldir), 0.0);

        Lo += (kD * albedo / PI + specular) * radiance * NdotL * checkShadow(i, normal, TanS);
        //Lo += vec3(1, 1, 1) * checkShadow(i, normal, N);
        //Lo += (1 / PI + specular) * radiance * NdotL;
    }

    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color   = ambient + Lo;

    // hdr
    color = color / (color + vec3(1.0)); // function asymptote y = 1 (maps to LDR range of [0, 1])
    //color = normalize(TanS * vec3(1, 1, 1));
    //color = normal;
    // gamma
    color = pow(color, vec3(1.0/2.2));
    frag_color = color;
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a      = roughness*roughness;
    float a2     = a*a;
    float NdotH  = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;
	
    float num   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;
	
    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float num   = NdotV;
    float denom = NdotV * (1.0 - k) + k;
	
    return num / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2  = GeometrySchlickGGX(NdotV, roughness);
    float ggx1  = GeometrySchlickGGX(NdotL, roughness);
	
    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
} 

vec3 normalFromNormalMap(vec2 texPos, mat3 T) {
    vec3 tangent_space_nor = normalize(texture(normalMap, texPos).rgb * 2.0 - 1.0);
    return normalize(T * tangent_space_nor);
}

float checkShadow(uint light, vec3 normal, mat3 TanS) {
    vec3 L = world_pos - LightPositions[light];
    vec3 ldir = normalize(L);

    // normal facing actual surface
    vec3 diskdir = normal;//TanS[2];// (ndp)*surfnor + (1-ndp)*normal;
    float ndp = clamp(dot(normal, diskdir), 0, 1);

    float currentDepth_2 = 0; // depth squared

    // PCF in a disk around point
    float shadow = 0;
    for(uint i = 0; i < Samples; i++) {
        // resize the disk to be of size radius and scale for consistent sampling area
        vec3 sampleOff = TanS * vec3(sampleOffsets[i] * SamplingRadius / max(abs(dot(diskdir, ldir)), 0.5), 0);//-0.05 * (1-dot(normal, ldir)));
        // sampleV traces a circle along the polygon surface aligned to surface normal
        vec3 sampleV = L + sampleOff;
        // retrive light depth value -> actual depth
        float mapDepth_2 = texture(lightDepthTex[light], sampleV).r * lightFarPlane;

        currentDepth_2 = dot(sampleV, sampleV);
        mapDepth_2 *= mapDepth_2;
        shadow += currentDepth_2 < mapDepth_2 ? 1.0 : 0.0;
    }
    shadow /= Samples;

    //shadow =  texture(lightDepthTex[light], L).r * texture(lightDepthTex[light], L).r;
    
    return clamp(shadow, 0, 1);
}