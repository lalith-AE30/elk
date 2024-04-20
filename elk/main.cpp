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

#include "stb_image.h"
#include "user_input.h"
#include "shader.h"
#include "camera.h"

GLuint map_texture(const char* filename) {
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);

    unsigned char* data = stbi_load(filename, &width, &height, &nrChannels, 3);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture" << std::endl;
        return -1;
    }
    stbi_image_free(data);
    return texture;
}

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

    Shader lighting_shader("shaders/cubes_vertex.glsl", "shaders/cubes_fragment.glsl");
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

    GLuint texture1 = map_texture("textures/container.jpg"), texture2 = map_texture("textures/moon.jpg");
    GLuint diffuse_map = map_texture("textures/container2.png");

    lighting_shader.use();
    lighting_shader.setInt("texture1", 0);
    lighting_shader.setInt("texture2", 1);
    lighting_shader.setInt("material.diffuse", 2);

    float dt = 0.0f;
    float last_frame = 0.0f;

    Bindings bindings = generate_bindings(window);

    struct Material {
        glm::vec4 ambient = glm::vec4(1.0f, 0.5f, 0.31f, 1.0f);
        glm::vec4 diffuse = glm::vec4(1.0f, 0.5f, 0.31f, 1.0f);
        glm::vec4 specular = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
        float shininess = 32.0f;
    } material;

    struct Light {
        glm::vec4 pos = glm::vec4(1.2f, 1.0f, 2.0f, 1.0f);

        glm::vec4 ambient = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
        glm::vec4 diffuse = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f);
        glm::vec4 specular = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
    } light;

    while (!glfwWindowShouldClose(window))
    {
        float current_frame = static_cast<float>(glfwGetTime());
        dt = current_frame - last_frame;
        last_frame = current_frame;

        processInput(window, &bindings, dt);

        glClearColor(0.01f, 0.0f, 0.01f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //glm::vec3 light_color;
        //light_color.x = sin(glfwGetTime() * 2.0f);
        //light_color.y = sin(glfwGetTime() * 0.7f);
        //light_color.z = sin(glfwGetTime() * 1.3f);

        //glm::vec3 diffuse_color = light_color * glm::vec3(0.5f);
        //glm::vec3 ambient_color = diffuse_color * glm::vec3(0.2f);

        //light.ambient = glm::vec4(ambient_color, 1.0f);
        //light.diffuse= glm::vec4(diffuse_color, 1.0f);

        glm::mat4 proj = glm::perspective(glm::radians(camera.zoom), (float)state.scr_width / (float)state.scr_height, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        {
            lighting_shader.use();

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texture1);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, texture2);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, diffuse_map);

            lighting_shader.setMat4("proj", proj);
            lighting_shader.setMat4("view", view);

            lighting_shader.setVec4("light.pos", view * light.pos);
            lighting_shader.setVec4("light.ambient", light.ambient);
            lighting_shader.setVec4("light.diffuse", light.diffuse);
            lighting_shader.setVec4("light.specular", light.specular);

            lighting_shader.setVec4("material.ambient", material.ambient);
            lighting_shader.setVec4("material.diffuse", material.diffuse);
            lighting_shader.setVec4("material.specular", material.specular);
            lighting_shader.setFloat("material.shininess", material.shininess);

            glBindVertexArray(modelVAO);
            for (unsigned int i = 0; i < 10; i++)
            {
                glm::mat4 model = glm::mat4(1.0f);
                model = glm::translate(model, cube_positions[i]);
                float angle = 20.0f * i;
                model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
                lighting_shader.setMat4("model", model);

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