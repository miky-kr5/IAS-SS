#version 120

uniform sampler2D sTexture;
uniform sampler1D sColorMap;

void main() {
  vec4 tex = texture2D(sTexture, gl_TexCoord[0].st);
  vec4 map = texture1D(sColorMap, tex.r);
  gl_FragColor = map;
}
