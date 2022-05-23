#version 460 core

out vec2 tex_coord;
// from https://rauwendaal.net/2014/06/14/rendering-a-screen-covering-triangle-in-opengl/

void main()
{
    float x = -1.0 + float((gl_VertexID & 1) << 2);
    float y = -1.0 + float((gl_VertexID & 2) << 1);
    tex_coord.x = (x+1.0)*0.5;
    tex_coord.y = (y+1.0)*0.5;
    gl_Position = vec4(x, y, 0, 1);
}