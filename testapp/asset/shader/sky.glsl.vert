#version 460 core
in vec3 VertPos;
in vec2 TexPos;

out vec2 tex_pos;

layout (std140) uniform Globals {
    mat4 V;
    mat4 P;
    vec3 CameraPos;
    uint NumLights;
    vec3 LightPositions[10];
    vec3 LightColors[10];
};

uniform mat4 M;

void main() {
    tex_pos = TexPos;
    mat4 v_t = V;
    v_t[3] = vec4(0, 0, 0, 1);
    gl_Position = P * v_t * M * vec4(VertPos, 1);
}