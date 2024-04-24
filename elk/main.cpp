#define NK_IMPLEMENTATION
#include <nuklear.h>

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

#include "camera.hpp"
#include "model_loader.hpp"
#include "shader.hpp"
#include "shader_utils.hpp"
#include "window_callbacks.hpp"

// TODO Get Nuklear in here
int main() {
	glfwInit();
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	}

	Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
	
	GLFWwindow* window = bindWindow(800, 600, &camera, "Model Viwer");

	// TODO Remove bindings control from main loop to window handler.
	Bindings bindings = generate_bindings(window);

	Shader lights_shader("shaders/phong_vertex.glsl", "shaders/depth_fragment.glsl");
	Shader light_source_shader("shaders/light_vertex.glsl", "shaders/light_fragment.glsl");

	Model model_3d("models/chess_board/chess_board.obj", true);
	Model model_3d1("models/skull/skull.obj", false);
	Model bulb("models/sphere/sphere.obj");

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

	int nr_lights = 4;
	std::vector<PointLight> point_lights;
	point_lights.resize(nr_lights);
	for (int i = 0; i < nr_lights; i++) {
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

		WindowState state = controller::getState();
		{
			controller::processInput(window, &bindings, dt);
			for (auto& point_light : point_lights)
				setVisibility(point_light, state.distance);
			setVisibility(spot_light, state.distance);
		}

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		float time = static_cast<float>(glfwGetTime());

		glm::mat4 model(1.0f);
		model = glm::translate(model, glm::vec3(0.0f, -0.3f, 0.0f));
		//model = glm::scale(model, glm::vec3(0.1f));
		//model = glm::translate(model, glm::vec3(0.0f, -80.0f, 0.0f));
		float aspect = (float)state.scr_width / (float)state.scr_height;
		glm::mat4 proj = glm::perspective(glm::radians(camera.zoom), aspect, 0.1f, 100.0f);
		//proj = glm::orthoRH(-3.0f, 3.0f, -3.0f/aspect, 3.0f/aspect, 0.0f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();

		lights_shader.use();

		lights_shader.setMat4("proj", proj);
		lights_shader.setMat4("view", view);
		lights_shader.setMat4("model", model);

		lights_shader.setVec4("dir_light.dir", dir_light.dir);
		lights_shader.setVec4("dir_light.ambient", dir_light.ambient);
		lights_shader.setVec4("dir_light.diffuse", dir_light.diffuse);
		lights_shader.setVec4("dir_light.specular", dir_light.specular);

		dir_light.dir = glm::vec4(sin(time), -1.0f, cos(time), 0.0f);
		updateMaterialShader(lights_shader, spot_light, point_lights, dir_light);
		model_3d.draw(lights_shader, state.mesh);

		model = glm::translate(model, glm::vec3(0.0f, 1.0f, 0.0f));
		lights_shader.setMat4("model", model);
		model_3d1.draw(lights_shader, state.mesh);

		light_source_shader.use();

		light_source_shader.setMat4("proj", proj);
		light_source_shader.setMat4("view", view);

		light_source_shader.setVec4("light_color", point_light.diffuse * 1e-1f * state.distance); // TODO Change heuristic constant

		for (auto& pos : point_light_positions) {
			glm::mat4 model(1.0f);
			model = glm::translate(model, pos);
			light_source_shader.setMat4("model", model);
			bulb.draw(light_source_shader);
		}

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}