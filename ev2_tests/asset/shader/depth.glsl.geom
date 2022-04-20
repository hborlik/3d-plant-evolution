#version 460 core

layout(triangles) in;
layout(triangle_strip, max_vertices=18) out;

uniform ShaderData {
    mat4 SM[6];
    float lightFarPlane;
    float lightNearPlane;
};

out vec4 FragPos;
out vec4 ClipPos;

void main() {
    for(int f = 0; f < 6; f++) {
        gl_Layer = f; // select cube map face to draw
        for(uint i = 0; i < 3; i++) { // emit triangle
            FragPos = gl_in[i].gl_Position;
            vec4 cpos = SM[f] * FragPos;
            gl_Position = cpos;
            ClipPos = cpos;
            EmitVertex();
        }
        EndPrimitive();
    }
}