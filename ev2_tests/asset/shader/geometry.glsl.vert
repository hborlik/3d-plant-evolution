#version 460 core

layout (std140) uniform Globals {
    mat4 V;
    mat4 P;
    vec3 CameraPos;
};

in vec3 VertPos;
in vec3 Normal;
in vec3 VertCol;
in vec2 TexPos;

uniform mat4 M;
uniform mat3 G;

out vec3 frag_pos;      // fragment position in view space
out vec3 vert_normal;   // normal in view space
out vec3 vert_color;    // passthrough
out vec2 tex_coord;     // passthrough

void main() {
    vec4 vertV = V * M * vec4(VertPos, 1.0);
    frag_pos = vertV.xyz;
    gl_Position = P * vertV;
    vert_normal = vec3(V * vec4(G * Normal, 0.0));
    vert_color = VertCol;
    tex_coord = TexPos;
}