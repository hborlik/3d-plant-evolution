#version 120
uniform vec3 UaColor;
uniform vec3 UdColor;
uniform vec3 UsColor;
uniform float Ushine;

varying vec3 vCol;
varying vec3 vNormal;
varying vec3 vLight;
varying vec3 vView;

uniform int uShadeModel;
void main() {

  vec3 Refl, Light, Norm, Spec, View, Diffuse;
  vec3 Half;
  bool blinn = true;

   if (uShadeModel == 1) {
      Light = normalize(vLight);
      Norm = normalize(vNormal);
      View = normalize(vView);
      Refl = 2.0*(dot(Light, Norm))*Norm -1.0*Light;
      Refl= normalize(Refl);
      if (blinn) {
         Half = Light+View;
         Spec = pow(clamp(dot(normalize(Half), Norm), 0.0, 1.0), Ushine)*UsColor;
      } else {
         Spec = pow(clamp(dot(View, Refl), 0.0, 1.0), Ushine)*UsColor;
      }
      Diffuse = clamp(dot(Norm, Light), 0.0, 1.0)*UdColor;
      gl_FragColor = vec4((Diffuse + Spec + UaColor), 1);
   } else {
      gl_FragColor = vec4(vCol, 1);
   }
}
