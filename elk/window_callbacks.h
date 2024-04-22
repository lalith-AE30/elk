#ifndef WINDOW_CALLBACK_H
#define WINDOW_CALLBACK_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <string>

#include "user_input.h"
#include "camera.h" 

struct WindowState {
    unsigned int scr_width = 800;
    unsigned int scr_height = 600;
    bool capture_controller = true;
    float distance = 10.0f;
    int mesh = -1;
    Camera* camera = NULL;
};

namespace controller {
    WindowState getState();
    void updateMeshIndex(int index);
    void processInput(GLFWwindow* window, Bindings* bindings, float dt);
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
GLFWwindow* bindWindow(unsigned int scr_width, unsigned int scr_height, Camera* camera, const std::string& title);

#endif // !WINDOW_CALLBACK_H
