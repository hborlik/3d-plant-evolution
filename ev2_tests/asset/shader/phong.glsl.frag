#version 460 core
out vec4 color;

in vec3 vert_normal;
in vec3 world_pos;
in vec3 vert_color;
in vec2 tex_pos;

in vec3 view_vec;

uniform vec3 ambient;
uniform vec3 diffuse;
uniform vec3 specular;
uniform vec3 transmittance;
uniform vec3 emission;
uniform float shininess;
uniform float ior;
uniform float dissolve;

layout (std140) uniform Globals {
    mat4 V;
    mat4 P;
    vec3 CameraPos;
};

void main() {
    vec3 light_position = vec3(100, 500, 0);
    vec3 Light = normalize(light_position - world_pos);
    vec3 normal = normalize(vert_normal);

    vec3 View = -normalize(view_vec);

    vec3 Refl = 2.0 * (dot(Light, normal)) * normal - Light;
    Refl = normalize(Refl);
    vec3 Spec = pow(clamp(dot(Refl, View), 0.0, 1.0), shininess) * specular;

    float dC = clamp(dot(normal, Light) + 0.3, 0.0, 1.0);

    color = vec4(emission + diffuse * dC, 1.0);
}