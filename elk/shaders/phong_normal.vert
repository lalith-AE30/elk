#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec4 pos;
out vec2 tex_coord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main()
{
	gl_Position = proj * view * model * vec4(aPos, 1.0);
	
	pos = view * model * vec4(aPos, 1.0);
	tex_coord = vec2(aTexCoord.x, aTexCoord.y);
}