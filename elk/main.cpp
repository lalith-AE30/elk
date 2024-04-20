#define STB_IMAGE_IMPLEMENTATION
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>
#include <vector>
#include <format>

#include "camera.h"
#include "shader.h"
#include "shader_utils.h"
#include "stb_image.h"
#include "user_input.h"

struct {
    unsigned int scr_width;
    unsigned int scr_height;
    bool capture_controller;
} state = { 800, 600, true };

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

    GLFWwindow* window = glfwCreateWindow(state.scr_width, state.scr_height, "LearnOpenGL", NULL, NULL);
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
    }

    glEnable(GL_DEPTH_TEST);

    Shader directional_light_shader("shaders/cubes_vertex.glsl", "shaders/cubes_fragment_dir.glsl");
    Shader point_light_shader("shaders/cubes_vertex.glsl", "shaders/cubes_fragment_point.glsl");
    Shader light_source_shader("shaders/light_vertex.glsl", "shaders/light_fragment.glsl");

    float vertices[] = {
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,  0.0f, 0.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
    };
    glm::vec3 cube_positions[] = {
        glm::vec3(0.0f,  0.0f,  0.0f),
        glm::vec3(2.0f,  5.0f, -15.0f),
        glm::vec3(-1.5f, -2.2f, -2.5f),
        glm::vec3(-3.8f, -2.0f, -12.3f),
        glm::vec3(2.4f, -0.4f, -3.5f),
        glm::vec3(-1.7f,  3.0f, -7.5f),
        glm::vec3(1.3f, -2.0f, -2.5f),
        glm::vec3(1.5f,  2.0f, -2.5f),
        glm::vec3(1.5f,  0.2f, -1.5f),
        glm::vec3(-1.3f,  1.0f, -1.5f)
    };

    GLuint VBO, modelVAO, lightVAO;
    {
        glGenBuffers(1, &VBO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        {
            glGenVertexArrays(1, &modelVAO);

            glBindVertexArray(modelVAO);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);

            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
            glEnableVertexAttribArray(1);

            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
            glEnableVertexAttribArray(2);
        }

        {
            glGenVertexArrays(1, &lightVAO);
            glBindVertexArray(lightVAO);

            glBindBuffer(GL_ARRAY_BUFFER, VBO);

            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
        }
    }

    GLuint diffuse_map = LoadTexture("textures/container2.png");
    GLuint specular_map = LoadTexture("textures/lighting_maps_specular_color.png");
    GLuint emission_map = LoadTexture("textures/radioactive.png");

    directional_light_shader.use();
    directional_light_shader.setInt("material.diffuse", 0);
    directional_light_shader.setInt("material.specular", 1);
    directional_light_shader.setInt("material.emission", 2);

    point_light_shader.use();
    point_light_shader.setInt("material.diffuse", 0);
    point_light_shader.setInt("material.specular", 1);
    point_light_shader.setInt("material.emission", 2);

    float dt = 0.0f;
    float last_frame = 0.0f;

    Bindings bindings = generate_bindings(window);

    Material material = {
        diffuse_map,
        specular_map,
        emission_map,
        32.0f
    };

    Light light = {
         glm::vec4(1.2f, 1.0f, 2.0f, 1.0f),
         glm::vec4(0.02f, 0.01f, 0.005f, 1.0f),
         glm::vec4(0.5f, 0.5f, 0.5f, 1.0f),
         glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)
    };

    DirectionalLight dir_light = {
     glm::vec4(-0.2f, -1.0f, -0.3f, 0.0f),
     glm::vec4(0.02f, 0.01f, 0.005f, 1.0f),
     glm::vec4(0.5f, 0.5f, 0.5f, 1.0f),
     glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)
    };

    while (!glfwWindowShouldClose(window))
    {
        float current_frame = static_cast<float>(glfwGetTime());
        dt = current_frame - last_frame;
        last_frame = current_frame;

        processInput(window, &bindings, dt);

        glClearColor(0.01f, 0.0f, 0.01f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float time = static_cast<float>(glfwGetTime());

        glm::mat4 proj = glm::perspective(glm::radians(camera.zoom), (float)state.scr_width / (float)state.scr_height, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        {
            directional_light_shader.use();

            directional_light_shader.setMat4("proj", proj);
            directional_light_shader.setMat4("view", view);

            updateMaterialShader(directional_light_shader, material, dir_light, time);

            glBindVertexArray(modelVAO);
            for (unsigned int i = 0; i < 10; i++)
            {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, cube_positions[i]);
                float angle = 20.0f * i;
                model = glm::rotate(model, time + glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
                directional_light_shader.setMat4("model", model);

                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }

        {
            point_light_shader.use();

            point_light_shader.setMat4("proj", proj);
            point_light_shader.setMat4("view", view);

            updateMaterialShader(point_light_shader, material, light, time);

            glBindVertexArray(modelVAO);
            for (unsigned int i = 0; i < 10; i++)
            {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, cube_positions[i]);
                float angle = 20.0f * i;
                model = glm::rotate(model, time + glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
                point_light_shader.setMat4("model", model);

                glDrawArrays(GL_TRIANGLES, 0, 36);
            }
        }

        {
            light_source_shader.use();

            light_source_shader.setMat4("proj", proj);
            light_source_shader.setMat4("view", view);

            light_source_shader.setVec4("light_color", light.diffuse);

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(light.pos));
            model = glm::scale(model, glm::vec3(0.2f));
            light_source_shader.setMat4("model", model);

            glBindVertexArray(lightVAO);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &modelVAO);
    glDeleteBuffers(1, &VBO);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window, Bindings* const bindings, float dt)
{
    if (bindings->key_esc.clicked()) {
        if (!state.capture_controller) {
            glfwSetWindowShouldClose(window, true);
        }
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        glfwSetScrollCallback(window, NULL);
        state.capture_controller = false;
    }
    if (!state.capture_controller && bindings->mouse_left.clicked()) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwSetScrollCallback(window, scrollCallback);
        state.capture_controller = true;
    }
    if (state.capture_controller) {
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
        camera.ProcessMouseMovement(window);
    }
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    state.scr_width = width;
    state.scr_height = height;
    glViewport(0, 0, width, height);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}