#version 460 core

// threshold bright areas of the hdr texture for bloom
// also add emissive brightness to final hdr texture

out layout(location = 0) vec4 hdr_combined;
out layout(location = 1) vec4 overbright_color;
in vec2 tex_coord;

uniform sampler2D hdrBuffer;
uniform sampler2D emissiveBuffer;
uniform float bloom_threshold = 1.0f;

void main() {
    const vec3 hdrColor = texture(hdrBuffer, tex_coord).rgb;
    const vec3 eColor = texture(emissiveBuffer, tex_coord).rgb;
    hdr_combined = vec4(hdrColor + eColor, 1.0);

    float brightness = dot(hdr_combined.rgb, vec3(0.2126, 0.7152, 0.0722));
    if(brightness > bloom_threshold) {
        overbright_color = vec4(hdr_combined.rgb, 1.0);
    } else
        overbright_color = vec4(0.0, 0.0, 0.0, 1.0);
}