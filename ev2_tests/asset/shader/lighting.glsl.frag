#version 460 core
out vec4 frag_color;

in vec2 tex_coord;

layout (std140) uniform Globals {
    mat4 V;
    mat4 P;
    vec3 CameraPos;
};

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

void main() {
    // retrieve data from G-buffer
    vec3 FragPos = texture(gPosition, tex_coord).rgb;
    vec3 Normal = texture(gNormal, tex_coord).rgb;
    vec3 Albedo = textureLod(gAlbedoSpec, tex_coord, 0).rgb;
    float Specular = texture(gAlbedoSpec, tex_coord).a;

    frag_color = vec4(Albedo + vec3(0.1, 0, 0), 1.0);
    //frag_color = vec4(tex_coord, 0, 1.0);
}