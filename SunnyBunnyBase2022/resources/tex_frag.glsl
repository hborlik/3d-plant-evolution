#version 330 core

in vec2 texCoord;
out vec4 color;
uniform sampler2D texBuf;

void main(){
	vec3 blue = vec3(0.5, 0, 0.5);
	vec3 texColor = texture( texBuf, texCoord ).rgb;
	// modify to show this is a 2D image
	if (gl_FragCoord.x > 680)
		texColor -= blue;

	color = vec4(texColor, 1.0);

}
