#version 460 core

out layout(location = 0) vec4 frag_color;
in vec2 tex_coord;

uniform sampler2D hdrBuffer;
uniform sampler2D bloomBuffer;

uniform float exposure = 0.5;
uniform float gamma = 2.2;

void main() {
    const vec3 hdrColor = texture(hdrBuffer, tex_coord).rgb;
    const vec3 bColor = texture(bloomBuffer, tex_coord).rgb;
    const vec3 color = hdrColor + bColor;

    // exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-color * exposure);
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));

    frag_color = vec4(mapped, 1.0f);
}