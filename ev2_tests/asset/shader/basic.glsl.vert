#version 460 core

in vec3 VertPos;
in vec3 Normal;
in vec2 TexPos;

layout (std140) uniform Globals {
    mat4 V;
    mat4 P;
    vec3 CameraPos;
    uint NumLights;
    vec3 LightPositions[10];
    vec3 LightColors[10];
};

uniform mat4 M;
uniform mat3 G;

out vec3 vert_nor;
out vec3 world_pos;
out vec2 tex_pos;

void main()
{
    gl_Position = P * V * M * vec4(Normal, 1.0);
    vert_nor = G * Normal;
    tex_pos = TexPos;
    world_pos = (M * vec4(Normal, 1.0)).xyz;
}