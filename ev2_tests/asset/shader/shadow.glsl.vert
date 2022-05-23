#include "globals.glslinc"

in vec3 VertPos;


uniform mat4 M;
uniform mat4 LV;
uniform mat4 LP;


out vec3 frag_pos;      // fragment position in view space

void main() {
    vec4 vertV = LV * M * vec4(VertPos, 1.0);
    frag_pos = vertV.xyz;
    gl_Position = LP * vertV;
}