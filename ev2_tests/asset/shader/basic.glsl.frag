#version 460 core
out vec3 FragColor;

in vec3 vert_nor;
in vec3 world_pos;
in vec2 tex_pos;

uniform ShaderData {
    float a;
};

layout (std140) uniform Globals {
    mat4 V;
    mat4 P;
    vec3 CameraPos;
    uint NumLights;
    vec3 LightPositions[10];
    vec3 LightColors[10];
};

uniform float diffuse;
uniform sampler2D diffuseTex;

void main() {
    //FragColor = vec3(1, 0, 1) * diffuse;
    //FragColor = vec3(1, 0, tex_pos.x) * diffuse;
    vec3 N = normalize(vert_nor);
    float L = 0;

    for(uint i = 0; i < NumLights; ++i) {
        vec3 ldir = normalize(LightPositions[i] - world_pos);
        L += clamp(dot(N, ldir), 0, 1);
    }
    FragColor = texture(diffuseTex, tex_pos).rgb * (L + 0.05);
}