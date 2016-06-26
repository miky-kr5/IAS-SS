#version 120

uniform sampler2D sTexture;

void main(void) {
  vec4 tex = texture2D(sTexture, gl_TexCoord[0].st);
  gl_FragColor = clamp(tex, 0.0, 1.0);
  //gl_FragColor = vec4(gl_TexCoord[0].s, gl_TexCoord[0].t, 0.0, 1.0);
}
