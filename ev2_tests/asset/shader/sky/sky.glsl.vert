#include "globals.glslinc"
// https://github.com/shff/opengl_sky
out vec3 pos;
out vec3 fsun;

uniform float time = 0.0;

const vec2 data[4] = vec2[](
    vec2(-1.0,  1.0), vec2(-1.0, -1.0),
    vec2( 1.0,  1.0), vec2( 1.0, -1.0)
);

void main()
{
    float x = -1.0 + float((gl_VertexID & 1) << 2);
    float y = -1.0 + float((gl_VertexID & 2) << 1);
    gl_Position = vec4(x, y, 0, 1);
    pos = transpose(mat3(View)) * (PInv * gl_Position).xyz;
    fsun = vec3(0.0, sin(time * 0.01), cos(time * 0.01));
}