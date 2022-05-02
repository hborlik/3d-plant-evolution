#include "globals.glslinc"
out vec4 frag_color;

in vec2 tex_coord;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

uniform vec3 lightDir;

void main() {
    // retrieve data from G-buffer
    vec3 FragPos = texture(gPosition, tex_coord).rgb;
    vec3 Normal = texture(gNormal, tex_coord).rgb;
    vec3 Albedo = texture(gAlbedoSpec, tex_coord).rgb;
    float Specular = texture(gAlbedoSpec, tex_coord).a;

    // lighting

    frag_color = vec4(Albedo, 1.0);
}