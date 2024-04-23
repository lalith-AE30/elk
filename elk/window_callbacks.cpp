#include "window_callbacks.h"

#include <iostream>
#include <system_error>

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

// TODO Add models and light setup and corresponding stuff in header
class Scene {
public:
    Scene(Camera camera) {
        //Model model_3d("models/skull/skull.obj", false);

        //DirectionalLight dir_light = {
        //    .dir = glm::vec4(-0.2f, -1.0f, -0.3f, 0.0f),
        //    .ambient = glm::vec4(0.02f, 0.01f, 0.005f, 1.0f),
        //    .diffuse = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f),
        //    .specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)
        //};
        //PointLight point_light = {
        //    .pos = glm::vec4(1.2f, 1.0f, 2.0f, 1.0f),
        //    .ambient = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
        //    .diffuse = glm::vec4(0.7f, 0.2f, 0.2f, 1.0f),
        //    .specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)
        //};
        //SpotLight spot_light = {
        // .pos = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
        // .dir = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f),
        // .soft_cutoff = glm::cos(glm::radians(10.0f)),
        // .cutoff = glm::cos(glm::radians(11.0f)),
        // .ambient = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
        // .diffuse = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f),
        // .specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)
        //};

        //glm::vec3 point_light_positions[] = {
        //    glm::vec3(0.7f,  0.2f,  2.0f),
        //    glm::vec3(2.3f, -3.3f, -4.0f),
        //    glm::vec3(-4.0f,  2.0f, -12.0f),
        //    glm::vec3(0.0f,  0.0f, -3.0f)
        //};

        //int nr_lights = 4;
        //std::vector<PointLight> point_lights;
        //point_lights.resize(nr_lights);
        //for (int i = 0; i < nr_lights; i++) {
        //    point_lights[i] = point_light;
        //    point_lights[i].pos = glm::vec4(point_light_positions[i], 1.0f);
        //}
    }
};

class Window {
private:
    GLFWwindow* window;

public:
    const unsigned int scr_width, scr_height;
    Scene scene;
    Bindings bindings;

    // TODO make camera to window optional
    Window(unsigned int scr_width, unsigned int scr_height, Camera camera, const std::string& title) : 
        scr_width(scr_width), 
        scr_height(scr_height),
        scene(Scene(camera)),
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
        }
        // TODO !Important make user_input.h classes available here
    }
};