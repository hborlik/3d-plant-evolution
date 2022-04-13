#version  330 core
layout(location = 0) in vec4 vertPos;
layout(location = 1) in vec3 vertNor;
uniform mat4 P;
uniform mat4 V;
uniform mat4 M;
uniform vec3 lightPos;

out vec3 fragNor;
out vec3 lightDir;
out vec3 EPos;

void main()
{
	gl_Position = P * V * M * vertPos;
	fragNor = (V*M * vec4(vertNor, 0.0)).xyz;
	lightDir = (V*vec4(1.0, 1.0, -0.5, 0.0)).xyz; //directional in camera
	EPos = vec3(1); //PULLED for release
}
