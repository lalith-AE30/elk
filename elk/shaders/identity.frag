#version 330 core

out vec4 frag_color;

void main() {
    frag_color = vec4(float(gl_FragCoord.x==gl_FragCoord.y));
}