#version 460 core
out vec4 frag_color;

in vec2 tex_coord;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

void main() {
    // retrieve data from G-buffer
    vec3 FragPos = texture(gPosition, tex_coord).rgb;
    vec3 Normal = texture(gNormal, tex_coord).rgb;
    vec3 Albedo = texture(gAlbedoSpec, tex_coord).rgb;
    float Specular = texture(gAlbedoSpec, tex_coord).a;

    frag_color = vec4(FragPos, 1.0);
}