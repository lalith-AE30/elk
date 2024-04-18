#pragma once
#define GLFW_INCLUDE_NONE
#define STB_IMAGE_IMPLEMENTATION
#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <string>

namespace shader_gen {
	struct shader_group {
		std::string vert_file;
		std::string frag_file;
	};

	GLuint create_vert_shader(std::string);

	GLuint create_frag_shader(std::string);

	GLuint createShaderProgram(shader_gen::shader_group = {});
}

namespace utils {
	GLuint loadTexture(std::string);
}