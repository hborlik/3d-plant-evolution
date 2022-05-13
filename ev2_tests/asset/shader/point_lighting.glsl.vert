#include "globals.glslinc"

in vec3 VertPos;

uniform mat4 M;

void main() {
    gl_Position = P * View * M * vec4(VertPos, 1.0);
}