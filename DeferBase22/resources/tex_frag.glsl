#version 330 core

in vec2 texCoord;
out vec4 color;
uniform sampler2D texBuf;
uniform sampler2D normalBuf;
uniform sampler2D colorBuf;
//ignored for now
uniform vec3 Lpos;

/* just pass through the texture color we will add to this next lab */
void main(){
   vec3 tPos = texture( texBuf, texCoord ).rgb;
   vec3 tNormal = texture( normalBuf, texCoord ).rgb;
   vec3 tColor = texture( colorBuf, texCoord ).rgb;

   vec3 L = normalize(Lpos - tPos);
   float Ld = length(Lpos - tPos);
   Ld = 1.0 / (Ld * Ld + 3.0 * Ld + 1.0);
   float lc = 0.09 + dot(tNormal, L);

   color = vec4(lc * tColor, 1.0);

}
