#version 120

uniform sampler2D sTexture;

void main() {
  vec4 tex = texture2D(sTexture, gl_TexCoord[0].st);
  gl_FragColor = clamp(tex, 0.0, 1.0);
}
