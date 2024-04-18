#include <fstream>
#include <iostream>
#include "stb_image.h"
#include "renderables.h"

namespace shader_gen {
	GLuint create_vert_shader(std::string filename = "verter.glsl") {
		std::ifstream file(filename);

		if (!file.is_open()) {
			return NULL;
		}

		std::string file_contents;
		file.seekg(0, std::ios::end);
		file_contents.reserve(file.tellg());
		file.seekg(0, std::ios::beg);

		file_contents.assign((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

		const char* vertexShaderSource = file_contents.c_str();

		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
		glCompileShader(vertexShader);

		int  success;
		char infoLog[500];
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertexShader, 500, NULL, infoLog);
			std::cout << filename << " compilation failed: " << infoLog << std::endl;
			return NULL;
		}

		return vertexShader;
	}

	GLuint create_frag_shader(std::string filename = "fragger.glsl") {
		std::ifstream file(filename);

		if (!file.is_open()) {
			return NULL;
		}

		std::string file_contents;
		file.seekg(0, std::ios::end);
		file_contents.reserve(file.tellg());
		file.seekg(0, std::ios::beg);

		file_contents.assign((std::istreambuf_iterator<char>(file)),
			std::istreambuf_iterator<char>());

		const char* fragShaderSource = file_contents.c_str();

		GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragShader, 1, &fragShaderSource, NULL);
		glCompileShader(fragShader);

		int  success;
		char infoLog[500];
		glGetShaderiv(fragShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragShader, 500, NULL, infoLog);
			std::cout << filename << " compilation failed: " << infoLog << std::endl;
			return NULL;
		}

		return fragShader;
	}

	GLuint createShaderProgram(shader_gen::shader_group grp) {
		if (grp.vert_file.empty()) {
			grp.vert_file = "verter.glsl";
		}
		if (grp.frag_file.empty()) {
			grp.frag_file = "fragger.glsl";
		}

		GLuint shaderProgram = glCreateProgram();

		GLuint vertexShader = create_vert_shader(grp.vert_file);
		GLuint fragmentShader = create_frag_shader(grp.frag_file);

		if (vertexShader == NULL || fragmentShader == NULL) {
			return NULL;
		}

		glAttachShader(shaderProgram, vertexShader);
		glAttachShader(shaderProgram, fragmentShader);
		glLinkProgram(shaderProgram);

		GLint success;
		glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
		if (!success) {
			return NULL;
		}

		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);

		return shaderProgram;
	}
}

namespace utils {
	GLuint loadTexture(std::string filename) {

		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		int width, height, nrChannels;
		std::cout << "Loading: " << filename << std::endl;
		unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrChannels, 0);
		try {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		catch (...) {
			std::cout << "Failed to load texture" << std::endl;
			return -1;
		}

		stbi_image_free(data);

		return texture;
	}
}