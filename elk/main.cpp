#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

#include <format>
#include <fstream>
#include <iostream>
#include <vector>

#include "camera.h"
#include "model_loader.h"
#include "shader.h"
#include "shader_utils.h"
#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#endif // !STB_IMAGE_IMPLEMENTATION
#include "user_input.h"

struct {
    unsigned int scr_width;
    unsigned int scr_height;
    bool capture_controller;
    float distance;
    int mesh;
} window_state = { 800, 600, true, 50.0f , -1};

void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window, Bindings* const bindings, float dt);

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));

int main()
{
    glfwInit();
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    }

    GLFWwindow* window = glfwCreateWindow(window_state.scr_width, window_state.scr_height, "LearnOpenGL", NULL, NULL);
    {
        if (window == NULL)
        {
            std::cerr << "Failed to create GLFW window" << std::endl;
            glfwTerminate();
            return -1;
        }
        glfwMakeContextCurrent(window);
        glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
        glfwSetScrollCallback(window, scrollCallback);
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
        {
            std::cout << "Failed to initialize GLAD" << std::endl;
            return -1;
        }
        glEnable(GL_DEPTH_TEST);
    }


    Shader lights_shader("shaders/cubes_vertex.glsl", "shaders/cubes_fragment.glsl");
    Shader light_source_shader("shaders/light_vertex.glsl", "shaders/light_fragment.glsl");

    Model model_3d("models/anime_girl/D0901D64.obj");

    Bindings bindings = generate_bindings(window);

    DirectionalLight dir_light = {
        .dir = glm::vec4(-0.2f, -1.0f, -0.3f, 0.0f),
        .ambient = glm::vec4(0.02f, 0.01f, 0.005f, 1.0f),
        .diffuse = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f),
        .specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)
    };
    PointLight point_light = {
        .pos = glm::vec4(1.2f, 1.0f, 2.0f, 1.0f),
        .ambient = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
        .diffuse = glm::vec4(0.7f, 0.2f, 0.2f, 1.0f),
        .specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)
    };
    SpotLight spot_light = {
     .pos = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
     .dir = glm::vec4(0.0f, 0.0f, -1.0f, 0.0f),
     .soft_cutoff = glm::cos(glm::radians(10.0f)),
     .cutoff = glm::cos(glm::radians(11.0f)),
     .ambient = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
     .diffuse = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f),
     .specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)
    };

    glm::vec3 point_light_positions[] = {
        glm::vec3(0.7f,  0.2f,  2.0f),
        glm::vec3(2.3f, -3.3f, -4.0f),
        glm::vec3(-4.0f,  2.0f, -12.0f),
        glm::vec3(0.0f,  0.0f, -3.0f)
    };

    PointLight point_lights[4];
    for (int i = 0; i < 4; i++) {
        point_lights[i] = point_light;
        point_lights[i].pos = glm::vec4(point_light_positions[i], 1.0f);
    }

    float dt = 0.0f;
    float last_frame = 0.0f;
    while (!glfwWindowShouldClose(window))
    {
        float current_frame = static_cast<float>(glfwGetTime());
        dt = current_frame - last_frame;
        last_frame = current_frame;

        processInput(window, &bindings, dt);
        for (auto& point_light : point_lights)
            setVisibility(point_light, window_state.distance);
        setVisibility(spot_light, window_state.distance);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float time = static_cast<float>(glfwGetTime());

        glm::mat4 model(1.0f);
        model = glm::scale(model, glm::vec3(0.1f));
        model = glm::translate(model, glm::vec3(0.0f, -80.0f, 0.0f));
        glm::mat4 proj = glm::perspective(glm::radians(camera.zoom), (float)window_state.scr_width / (float)window_state.scr_height, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        lights_shader.use();
        
        lights_shader.setMat4("proj", proj);
        lights_shader.setMat4("view", view);
        lights_shader.setMat4("model", model);

        lights_shader.setVec4("dir_light.dir",      dir_light.dir);
        lights_shader.setVec4("dir_light.ambient",  dir_light.ambient);
        lights_shader.setVec4("dir_light.diffuse",  dir_light.diffuse);
        lights_shader.setVec4("dir_light.specular", dir_light.specular);

        dir_light.dir = glm::vec4(sin(time), -1.0f, cos(time), 0.0f);
        updateMaterialShader(lights_shader, spot_light, point_lights, dir_light);
        model_3d.draw(lights_shader, window_state.mesh);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window, Bindings* const bindings, float dt) {
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
        camera.refocus = true;
        window_state.capture_controller = true;
    }
    if (window_state.capture_controller) {
        if (bindings->key_w.down())
            camera.ProcessKeyboard(CameraMovement::FORWARD, dt);
        if (bindings->key_s.down())
            camera.ProcessKeyboard(CameraMovement::BACKWARD, dt);
        if (bindings->key_left.down())
            camera.ProcessKeyboard(CameraMovement::LEFT, dt);
        if (bindings->key_right.down())
            camera.ProcessKeyboard(CameraMovement::RIGHT, dt);
        if (bindings->key_space.down())
            camera.ProcessKeyboard(CameraMovement::UP, dt);
        if (bindings->key_lctrl.down())
            camera.ProcessKeyboard(CameraMovement::DOWN, dt);
        //if (bindings->key_up.down())
        //    window_state.distance += 5.0f;
        //if (bindings->key_down.down())
        //    window_state.distance = std::max(0.0f, window_state.distance - 5.0f);
        if (bindings->key_up.clicked())
            window_state.mesh += 1;
        if (bindings->key_down.clicked())
            window_state.mesh -= 1;
        camera.ProcessMouseMovement(window);
    }
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    window_state.scr_width = width;
    window_state.scr_height = height;
    glViewport(0, 0, width, height);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}