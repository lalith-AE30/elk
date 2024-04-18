#version 330 core
out vec4 frag_color;

in vec2 tex_coord;

uniform vec3 light_color;

uniform sampler2D texture1;
uniform sampler2D texture2;

uniform float opac;
uniform float time;


void main() {
	frag_color = vec4(light_color, 1.0) * mix(texture(texture1, tex_coord), texture(texture2, tex_coord), opac);
}