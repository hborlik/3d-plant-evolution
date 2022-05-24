#include "globals.glslinc"

in layout(location = 0) vec3 VertPos;
in layout(location = 1) vec3 Normal;
in layout(location = 2) vec3 VertCol;
in layout(location = 3) vec2 TexPos;

uniform mat4 M;
uniform mat3 G;

out vec3 frag_pos;      // fragment position in view space
out vec3 vert_normal;   // normal in view space
out vec3 vert_color;    // passthrough
out vec2 tex_coord;     // passthrough

void main() {
    vec4 vertV = View * M * vec4(VertPos, 1.0);
    frag_pos = vertV.xyz;
    gl_Position = P * vertV;
    vert_normal = vec3(View * vec4(G * Normal, 0.0));
    vert_color = VertCol;
    tex_coord = TexPos;
}