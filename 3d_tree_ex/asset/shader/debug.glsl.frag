#version 460 core
#extension GL_GOOGLE_include_directive : enable

#include "globals.glslinc"

out vec4 frag_color;

in vec3 color;

void main() {
    frag_color = vec4(color, 1);
}