#version 330 core

in vec2 texCoord;
out vec4 color;
uniform sampler2D texBuf;
uniform float time;

void main(){
	vec3 blue = vec3(0.5, 0, 0.5);
	vec3 texColor = texture( texBuf, vec2(texCoord.x + 0.2 * sin(time) + time / 5, texCoord.y + 0.1 * sin(time + gl_FragCoord.x / 150.0)) ).rgb;
	// modify to show this is a 2D image
	if (gl_FragCoord.x > 680)
		texColor -= blue;

	color = vec4(texColor, 1.0);

}
