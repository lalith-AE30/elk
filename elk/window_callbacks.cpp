#include "window_callbacks.h"

#include <iostream>

// TODO Add binding so that multiple windows can differentiate, and further hide it from user
WindowState window_state;

namespace controller {
    WindowState getState() {
        return window_state;
    }
    void updateMeshIndex(int index) {
        window_state.mesh = index;
    }
    void processInput(GLFWwindow* window, Bindings* bindings, float dt) {
        if (bindings->key_esc.clicked()) {
            if (!window_state.capture_controller) {
                glfwSetWindowShouldClose(window, true);
            }
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
            glfwSetScrollCallback(window, NULL);
            window_state.capture_controller = false;
        }
        if (!window_state.capture_controller && bindings->mouse_left.clicked()) {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            glfwSetScrollCallback(window, scrollCallback);
            window_state.camera->refocus = true;
            window_state.capture_controller = true;
        }
        if (window_state.capture_controller) {
            if (bindings->key_w.down())
                window_state.camera->ProcessKeyboard(CameraMovement::FORWARD, dt);
            if (bindings->key_s.down())
                window_state.camera->ProcessKeyboard(CameraMovement::BACKWARD, dt);
            if (bindings->key_a.down())
                window_state.camera->ProcessKeyboard(CameraMovement::LEFT, dt);
            if (bindings->key_d.down())
                window_state.camera->ProcessKeyboard(CameraMovement::RIGHT, dt);
            if (bindings->key_space.down())
                window_state.camera->ProcessKeyboard(CameraMovement::UP, dt);
            if (bindings->key_lctrl.down() && !bindings->key_up.down() && !bindings->key_down.down())
                window_state.camera->ProcessKeyboard(CameraMovement::DOWN, dt);
            if (bindings->key_up.down() && bindings->key_lctrl.down())
                window_state.distance += 5.0f;
            if (bindings->key_down.down() && bindings->key_lctrl.down())
                window_state.distance = std::max(0.0f, window_state.distance - 5.0f);
            if (bindings->key_up.clicked())
                window_state.mesh += 1;
            if (bindings->key_down.clicked())
                window_state.mesh -= 1;
            window_state.camera->ProcessMouseMovement(window);
        }
    }
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    window_state.scr_width = width;
    window_state.scr_height = height;
    glViewport(0, 0, width, height);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    window_state.camera->ProcessMouseScroll(static_cast<float>(yoffset));
}

GLFWwindow* bindWindow(unsigned int scr_width, unsigned int scr_height, Camera* camera, const std::string& title) {
    window_state.scr_width = scr_width;
    window_state.scr_height = scr_height;

    GLFWwindow* window = glfwCreateWindow(window_state.scr_width, window_state.scr_height, title.c_str(), NULL, NULL);
    {
        if (window == NULL)
        {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return NULL;
        }
        glfwMakeContextCurrent(window);
        glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
        glfwSetScrollCallback(window, scrollCallback);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return NULL;
        }
        glEnable(GL_DEPTH_TEST);
    }

    window_state.camera = camera;

    return window;

}