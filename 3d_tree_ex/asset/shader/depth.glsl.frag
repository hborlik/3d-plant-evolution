#version 460 core
in vec4 FragPos;
in vec4 ClipPos;

uniform vec3 lightPos;
uniform ShaderData {
    mat4 SM[6];
    float lightFarPlane;
    float lightNearPlane;
};

void main()
{
    // get distance between fragment and light source
    float lightDistance = length(FragPos.xyz - lightPos);

    float ndc_d = ClipPos.z / ClipPos.w;

    // back to NDC (Clip pos)
    float z = gl_FragCoord.z * 2.0 - 1;
    // actual depth
    float rdepth = (2.0 * lightNearPlane * lightFarPlane) / (lightFarPlane + lightNearPlane - z * (lightFarPlane - lightNearPlane));
    float adepth = (2.0 * lightNearPlane * lightFarPlane) / (lightFarPlane + lightNearPlane - ndc_d * (lightFarPlane - lightNearPlane));

    // corrected depth should be below surface (larger)
    float bias = rdepth - adepth;
    
    // push depth threshold below surface to prevent shadow being renderd on lit area
    gl_FragDepth = (lightDistance + bias) / lightFarPlane;
}  