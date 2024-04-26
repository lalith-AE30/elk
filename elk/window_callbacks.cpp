#include "window_callbacks.hpp"
#include "model_loader.hpp"
#include "shader_utils.hpp"

#include <vector>
#include <iostream>
#include <system_error>

struct Lights {
    std::vector<DirectionalLight> dir_lights;
    std::vector<PointLight> point_lights;
    std::vector<SpotLight> spot_lights;
};

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
        // TODO Change to more efficient callbacks
        // TODO Add better controls for modified keys.
        if (window_state.capture_controller) {
            if (bindings->key_w.down())
                window_state.camera->processKeyboard(CameraMovement::FORWARD, dt);
            if (bindings->key_s.down())
                window_state.camera->processKeyboard(CameraMovement::BACKWARD, dt);
            if (bindings->key_a.down())
                window_state.camera->processKeyboard(CameraMovement::LEFT, dt);
            if (bindings->key_d.down())
                window_state.camera->processKeyboard(CameraMovement::RIGHT, dt);
            if (bindings->key_space.down())
                window_state.camera->processKeyboard(CameraMovement::UP, dt);
            if (bindings->key_lctrl.down())
                window_state.camera->processKeyboard(CameraMovement::DOWN, dt);
            if (bindings->key_up.down() && bindings->key_lalt.down())
                window_state.distance += 5.0f;
            if (bindings->key_down.down() && bindings->key_lalt.down())
                window_state.distance = std::max(0.0f, window_state.distance - 5.0f);
            if (bindings->key_up.clicked() && !bindings->key_lalt.down())
                window_state.mesh += 1;
            if (bindings->key_down.clicked() && !bindings->key_lalt.down())
                window_state.mesh -= 1;
            if (bindings->key_f.clicked()) {
                if (window_state.depth_testing)
                    glDisable(GL_DEPTH_TEST);
                else {
                    glEnable(GL_DEPTH_TEST);
                }
                window_state.depth_testing = !window_state.depth_testing;
            }
            window_state.camera->processMouseMovement(window);
        }
    }
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    window_state.scr_width = width;
    window_state.scr_height = height;
    glViewport(0, 0, width, height);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    window_state.camera->processMouseScroll(static_cast<float>(yoffset));
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

// TODO Add instancing before it can go into usage due to high memory usage
//class Scene {
//public:
//    Camera camera;
//    std::vector<Model> models;
//    Lights lights;
//
//    Scene(Camera camera, std::vector<Model> models, Lights lights) :
//        camera(camera),
//        models(models),
//        lights(lights)
//    { }
//};

class Window {
private:
    GLFWwindow* window;

public:
    const unsigned int scr_width, scr_height;
    //Scene scene;
    Bindings bindings;

    // TODO make camera to window optional
    Window(unsigned int scr_width, unsigned int scr_height, Camera camera, const std::string& title) :
        scr_width(scr_width),
        scr_height(scr_height),
        bindings(generate_empty_binding())
    {
        window = glfwCreateWindow(scr_width, scr_height, title.c_str(), NULL, NULL);
        {
            if (window == NULL) {
                throw std::system_error(-1, std::generic_category(), "Failed to create GLFW window");
                glfwTerminate();
                return;
            }
            glfwMakeContextCurrent(window);
            glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
            glfwSetScrollCallback(window, scrollCallback);
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
                throw std::system_error(-2, std::generic_category(), "Failed to initialize GLAD");
            glEnable(GL_DEPTH_TEST);
            window_state.depth_testing = true;
        }
        // TODO !Important make user_input.h classes available here
    }
};