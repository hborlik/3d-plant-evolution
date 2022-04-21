#version 460 core

in vec3 VertPos;
in vec3 Normal;
in vec3 VertCol;
in vec2 TexPos;

layout (std140) uniform Globals {
    mat4 V;
    mat4 P;
    vec3 CameraPos;
};

uniform mat4 M;
uniform mat3 G;

out vec3 vert_normal;
out vec3 world_pos;
out vec3 vert_color;
out vec2 tex_pos;

out vec3 view_vec;

void main()
{
    vec4 vertV = V * M * vec4(VertPos, 1.0);
    vec4 vert = P * vertV;
    view_vec = vec3(vertV);
    gl_Position = vert;
    vert_normal = G * Normal;
    vert_color = VertCol;
    tex_pos = TexPos;
    world_pos = (M * vec4(VertPos, 1.0)).xyz;
}