#version 460 core
out vec4 color;

in vec3 vert_normal;
in vec3 world_pos;
in vec3 vert_color;
in vec2 tex_pos;

layout (std140) uniform Globals {
    mat4 V;
    mat4 P;
    vec3 CameraPos;
};

void main() {
    vec3 normal = normalize(vert_normal);

    float dC = clamp(dot(normal, vec3(0, 1, 0)), 0.0, 1.0);
    color = vec4(vec3(0.04, 0.05, 0.02) + dC * vert_color, 1.0);
}