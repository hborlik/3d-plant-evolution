#version 460 core
in layout(location = 0) vec3 VertPos;

uniform mat4 M;

void main() {
    gl_Position = M * vec4(VertPos, 1);
}