#version 120
uniform vec3 UaColor;
uniform vec3 UdColor;
uniform vec3 UsColor;
uniform float Ushine;

varying vec3 vCol;
varying vec3 vNormal;
varying vec3 vLight;
varying vec3 vView;

void main() {

  vec3 Refl, Light, Norm, Spec, View, Diffuse;
  vec3 Half;

  Light = normalize(vLight);
  Norm = normalize(vNormal);
  View = normalize(vView);
  Half = Light+View;
  Spec = pow(clamp(dot(normalize(Half), Norm), 0.0, 1.0), Ushine)*UsColor;
  Diffuse = clamp(dot(Norm, Light), 0.0, 1.0)*UdColor;
  gl_FragColor = vec4((Diffuse + Spec + UaColor), 1);
}
