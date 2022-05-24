#include "globals.glslinc"

in layout(location = 0) vec3 VertPos;
in layout(location = 1) vec3 VertColor;

out vec3 color;

void main() {
    gl_Position = P * View * vec3(VertPos, 1.0f);
    color = VertColor;
}