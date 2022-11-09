#version 460 core
out vec3 FragColor;

in vec2 tex_pos;

uniform sampler2D diffuseTex;

void main() {
    FragColor = texture(diffuseTex, tex_pos).rgb;
    gl_FragDepth = 1;
}