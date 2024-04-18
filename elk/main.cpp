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
    float mix;
    bool capture_controller;
} state = { 800, 600, 0.5, true };

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

    Shader shader_prog("shaders/verter.glsl", "shaders/fragger.glsl");

    float vertices[] = {
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f
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
    glm::vec3 sphere_positions(5.0f, 0.3f, 6.0f);

    GLuint VBO, VAO;
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    GLuint texture1 = map_texture("textures/container.jpg"), texture2 = map_texture("textures/awesomeface.png");

    shader_prog.use();
    shader_prog.setInt("texture1", 0);
    shader_prog.setInt("texture2", 1);

    float dt = 0.0f;
    float last_frame = 0.0f;

    Bindings bindings = generate_bindings(window);

    while (!glfwWindowShouldClose(window))
    {
        float current_frame = static_cast<float>(glfwGetTime());
        dt = current_frame - last_frame;
        last_frame = current_frame;

        processInput(window, &bindings, dt);

        glClearColor(0.01f, 0.0f, 0.01f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader_prog.use();

        shader_prog.setFloat("opac", state.mix);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texture2);

        glm::mat4 proj = glm::perspective(glm::radians(camera.zoom), (float)state.scr_width / (float)state.scr_height, 0.1f, 100.0f);
        shader_prog.setMat4("proj", proj);

        glm::mat4 view = camera.GetViewMatrix();
        shader_prog.setMat4("view", view);

        glBindVertexArray(VAO);
        for (unsigned int i = 0; i < 10; i++)
        {
            // calculate the model matrix for each object and pass it to shader before drawing
            glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
            model = glm::translate(model, cube_positions[i]);
            float angle = 20.0f * i;
            model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
            shader_prog.setMat4("model", model);

            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
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
        if (bindings->key_up.down())
            state.mix += 0.01;
        if (bindings->key_down.down())
            state.mix -= 0.01;
        camera.ProcessMouseMovement(window);
    }
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetScrollCallback(window, scrollCallback);
    state.capture_controller = true;
    state.scr_width = width;
    state.scr_height = height;
    glViewport(0, 0, width, height);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}