#version 460 core
#extension GL_GOOGLE_include_directive : enable

#include "globals.glslinc"
#include "disney.glslinc"

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out uint gMaterialTex;
layout (location = 4) out vec4 gEmissive;

in vec3 frag_pos; // fragment position in view space
in vec3 vert_normal;
in vec3 vert_color;
in vec2 tex_coord;

uniform uint materialId;
uniform float vertex_color_weight;

uniform sampler2D diffuse_tex;

void main() {
    vec4 tex = texture(diffuse_tex, tex_coord);
    if (tex.a < 0.05)
        discard;
    gPosition = frag_pos;
    gNormal = normalize(vert_normal);
    gAlbedoSpec.rgb = vert_color * vertex_color_weight + tex.rgb;
    gAlbedoSpec.a = 0;
    gMaterialTex = materialId;
    gEmissive = vec4(materials[materialId].emissive, 0.0);
}