#version 460 core
#extension GL_GOOGLE_include_directive : enable
#include "globals.glslinc"
#include "point_lighting.glslinc"

in vec3 VertPos;

flat out uint instance_id;

layout(std430, binding = 3) buffer lights_in {
    PointLight lights[];
};

void main() {
    mat4 M = mat4(lights[gl_InstanceID].scale);
    vec4 T = vec4(lights[gl_InstanceID].position, 1.0f);
    M[3] = T;
    gl_Position = VP * M * vec4(VertPos, 1.0);
    instance_id = gl_InstanceID;
}