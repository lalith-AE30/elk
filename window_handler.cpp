#include "window_handler.h"
#include <iostream>

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

GLFWwindow* windowBootstrap(int scr_width, int scr_height, const char* title) {
	GLFWwindow* window = glfwCreateWindow(800, 600, title, NULL, NULL);
	if (window == NULL) {
		std::cerr << "Failed to create window";
		glfwTerminate();
	}
	glfwMakeContextCurrent(window);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "Failed to initialize GLAD\n";
		return NULL;
	}

	glViewport(0, 0, 800, 600);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	return window;
}
