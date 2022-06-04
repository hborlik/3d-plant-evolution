#version 460 core

// gaussian blur in multiple passes (either horizontal or vertical)

out layout(location = 0) vec4 bloom_blur;
in vec2 tex_coord;

uniform sampler2D bloom_blur_in;

uniform bool horizontal;
uniform float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);

void main() {
    const vec2 tex_offset = 1.0 / textureSize(bloom_blur_in, 0);
    vec3 result = texture(bloom_blur_in, tex_coord).rgb * weight[0];

    if(horizontal)
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(bloom_blur_in, tex_coord + vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
            result += texture(bloom_blur_in, tex_coord - vec2(tex_offset.x * i, 0.0)).rgb * weight[i];
        }
    }
    else
    {
        for(int i = 1; i < 5; ++i)
        {
            result += texture(bloom_blur_in, tex_coord + vec2(0.0, tex_offset.y * i)).rgb * weight[i];
            result += texture(bloom_blur_in, tex_coord - vec2(0.0, tex_offset.y * i)).rgb * weight[i];
        }
    }
    bloom_blur = vec4(result, 1.0);
}