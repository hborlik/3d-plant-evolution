#version 460 core

layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;

in vec3 frag_pos; // fragment position in view space
in vec3 vert_normal;
in vec3 vert_color;
in vec2 tex_coord;

uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;
uniform vec3 transmittance;
uniform vec3 emission;
uniform float shininess;
uniform float ior;
uniform float dissolve;

void main() {
    gPosition = frag_pos;
    gNormal = normalize(vert_normal);
    gAlbedoSpec.rgb = diffuse;
    gAlbedoSpec.a = shininess;
}