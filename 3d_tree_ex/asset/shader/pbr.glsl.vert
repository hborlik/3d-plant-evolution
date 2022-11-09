#version 460 core
in layout(location = 0) vec3 VertPos;
in layout(location = 1) vec2 TexPos;
in layout(location = 2) vec3 BiTangent;
in layout(location = 3) vec3 Tangent;

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

out vec3 world_pos;
out vec3 vert_nor;
out vec3 vert_tan;
out vec3 vert_bitan;
out vec2 tex_pos;

void main() {
    vec4 wrldpos = M*vec4(VertPos, 1);
    world_pos = wrldpos.xyz;
    gl_Position = P*V*wrldpos;
    vert_nor = (M*vec4(normalize(cross(Tangent, BiTangent)), 0)).xyz;
    vert_tan = Tangent;
    vert_bitan = BiTangent;
    tex_pos = TexPos;
}