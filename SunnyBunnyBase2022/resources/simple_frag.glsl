#version 330 core 
in vec3 fragNor;
in vec3 WPos;
//to send the color to a frame buffer
layout(location = 0) out vec4 color;

uniform vec3 MatAmb;
uniform vec3 MatDif;


/* Very simple Diffuse shader */
void main()
{
	vec3 LPos = vec3(2, 2, 2);
	vec3 Dcolor, Scolor;
    vec3 Dlight = normalize(LPos - WPos);
	vec3 normal = normalize(fragNor);
	Dcolor = MatDif*max(dot(normalize(Dlight), normal), 0)+MatAmb;
	color = vec4(Dcolor, 1.0);
}
