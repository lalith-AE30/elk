#pragma once
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glad/glad.h>

GLFWwindow* windowBootstrap(int scr_width, int scr_height, const char* title);