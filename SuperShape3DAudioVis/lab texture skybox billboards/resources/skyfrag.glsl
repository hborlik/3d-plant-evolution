#version 330 core
out vec4 color;
in vec3 vertex_normal;
in vec3 vertex_pos;
in vec2 vertex_tex;
uniform vec3 campos;

uniform sampler2D tex;
uniform sampler2D tex2;

uniform float dn;
void main()
{
vec4 tcol = texture(tex, vertex_tex);
vec4 tcoln = texture(tex2, vertex_tex);
color = dn * tcoln + (1.-dn) * tcol;
}
