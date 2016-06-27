#version 120

uniform sampler2D sTexture;

void main() {
  vec4 tex = texture2D(sTexture, gl_TexCoord[0].st);
  vec4 color = mix(vec4(0.0, 0.0, 0.3, 1.0), vec4(1.0, 1.0, 0.0, 1.0), tex.r);
  gl_FragColor = clamp(color, 0.0, 1.0);
}
