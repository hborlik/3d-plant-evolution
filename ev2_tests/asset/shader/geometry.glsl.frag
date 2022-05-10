#version 460 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out uint gMaterialTex;

in vec3 frag_pos; // fragment position in view space
in vec3 vert_normal;
in vec3 vert_color;
in vec2 tex_coord;

uniform uint materialId;

void main() {
    gPosition = frag_pos;
    gNormal = normalize(vert_normal);
    gAlbedoSpec.rgb = vec3(0);
    gAlbedoSpec.a = 0;
    gMaterialTex = materialId;
}