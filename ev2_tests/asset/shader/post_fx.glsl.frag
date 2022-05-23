#include "globals.glslinc"
out vec4 frag_color;
in vec2 tex_coord;

uniform sampler2D hdrBuffer;

uniform float exposure = 0.5;
uniform float gamma = 2.2;

void main() {
    const vec3 hdrColor = texture(hdrBuffer, tex_coord).rgb;
  
    // exposure tone mapping
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
    // gamma correction 
    mapped = pow(mapped, vec3(1.0 / gamma));

    frag_color = vec4(mapped, 1.0f);
}