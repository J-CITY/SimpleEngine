#version 460 core

/* Information passed from the fragment shader */
in VS_OUT {
	vec3 color;
} fs_in;

out vec4 FRAGMENT_COLOR;

void main() {
	FRAGMENT_COLOR = vec4(fs_in.color, 1.0);
}
