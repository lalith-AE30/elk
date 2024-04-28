#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;

out vec2 frag_pos;
out vec2 tex_coord;

void main() {
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
    frag_pos = vec2(aPos.x, aPos.y);
    tex_coord = aTexCoord;
}