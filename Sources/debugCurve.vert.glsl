attribute vec3 pos;
uniform mat4 MVP;

void kore() {
	gl_Position = MVP * vec4(pos.x, pos.y, pos.z, 1.0);
}
