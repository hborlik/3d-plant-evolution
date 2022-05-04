#version 330 core

in vec2 texCoord;
out vec4 color;
uniform sampler2D gBuf;
uniform sampler2D gNor;
uniform sampler2D gColorSpec;

/* just pass through the texture color we will add to this next lab */
void main(){
   vec3 Ldir = vec3(1.0, 1.0, 0);
   Ldir = normalize(Ldir);
   vec3 tBuf = texture( gBuf, texCoord ).rgb;
   vec3 tNor = texture( gNor, texCoord ).rgb;
   vec3 tColor = texture( gColorSpec, texCoord ).rgb;
   float diffuse = dot(Ldir, tNor);
   color = vec4(tColor * diffuse, 1.0);

}
