#version 330 core 

out vec4 color;

uniform vec3 MatAmb;
uniform vec3 MatDif;
uniform vec3 MatSpec;
uniform float MatShine;

//interpolated normal and light vector in camera space
in vec3 fragNor;
in vec3 lightDir;
//position of the vertex in camera space
in vec3 EPos;

void main()
{
	vec3 normal = normalize(fragNor);
	vec3 light = normalize(lightDir);
	float dC = max(0, dot(normal, light));
	color = vec4(MatAmb + dC*MatDif, 1.0);
}
