#version 330 core

in vec2 texCoord;
out vec4 color;
uniform sampler2D texBuf;
uniform float time;
uniform vec2 screen_size;

void main(){
	vec3 blue = vec3(0.5, 0, 0.5);
	vec3 texColor = texture( texBuf, vec2(texCoord.x, texCoord.y)).rgb;
	// modify to show this is a 2D image
	float d = round(8 * length(vec2(0.5, 0.5 + 0.2 * sin(time)) - texCoord) * 2.0f) / 8.0;
	texColor += max(0, 1 - d*d) * vec3(1, 1, 0.0);
	if (gl_FragCoord.x > 680)
		texColor -= blue;

	color = vec4(texColor, 1.0);

}
