#ifdef GL_ES
precision mediump float;
#endif

uniform vec4 color;

void kore() {
	gl_FragColor = color;
}
