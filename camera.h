#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class CameraMovement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera
{
public:
	glm::vec3 pos;

	float movement_speed;
	float mouse_sensitivity;
	float zoom;

	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) :
		front(glm::vec3(0.0f, 0.0f, -1.0f)),
		movement_speed(SPEED),
		mouse_sensitivity(SENSITIVITY),
		zoom(ZOOM)
	{
		pos = position;
		world_up = up;
		yaw_ = yaw;
		pitch_ = pitch;
		updateCameraVectors();
	}

	Camera(float pos_x, float pos_y, float pos_z, float up_x, float up_y, float up_z, float yaw = YAW, float pitch = PITCH) : 
		front(glm::vec3(0.0f, 0.0f, -1.0f)), 
		movement_speed(SPEED), 
		mouse_sensitivity(SENSITIVITY), 
		zoom(ZOOM)
	{
		pos = glm::vec3(pos_x, pos_y, pos_z);
		world_up = glm::vec3(up_x, up_y, up_z);
		yaw_ = yaw;
		pitch_ = pitch;
		updateCameraVectors();
	}

	glm::mat4 GetViewMatrix() const {
		return glm::lookAt(pos, pos + front, up);
	}

	void ProcessKeyboard(CameraMovement direction, float dt)
	{
		float velocity = movement_speed * dt;
		switch (direction) {
		case CameraMovement::FORWARD:
			pos += front * velocity;
			break;
		case CameraMovement::BACKWARD:
			pos -= front * velocity;
			break;
		case CameraMovement::RIGHT:
			pos += right * velocity;
			break;
		case CameraMovement::LEFT:
			pos -= right * velocity;
			break;
		case CameraMovement::UP:
			pos += up * velocity;
			break;
		case CameraMovement::DOWN:
			pos -= up * velocity;
			break;
		}
	}

	void ProcessMouseMovement(GLFWwindow* window, GLboolean constrainPitch = true)
	{
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		float xoffset = mouse_sensitivity * (xpos - last_x);
		float yoffset = -mouse_sensitivity * (ypos - last_y);
		last_x = xpos;
		last_y = ypos;

		yaw_ += xoffset;
		pitch_ += yoffset;

		// make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (pitch_ > 89.0f)
				pitch_ = 89.0f;
			if (pitch_ < -89.0f)
				pitch_ = -89.0f;
		}

		// update Front, Right and Up Vectors using the updated Euler angles
		updateCameraVectors();
	}

	void ProcessMouseScroll(float yoffset)
	{
		zoom -= (float)yoffset;
		if (zoom < 1.0f)
			zoom = 1.0f;
		if (zoom > 120.0f)
			zoom = 120.0f;
	}

private:
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 world_up;

	// euler Angles
	float yaw_, pitch_;

	float last_x = 400, last_y = 300;
	void updateCameraVectors()
	{
		// calculate the front vector using Euler Angles
		front = glm::vec3(
			cos(glm::radians(yaw_)) * cos(glm::radians(pitch_)),
			sin(glm::radians(pitch_)),
			sin(glm::radians(yaw_)) * cos(glm::radians(pitch_))
		);
		// re-calculate the Right and Up vector
		right = glm::normalize(glm::cross(front, world_up));
		up = glm::normalize(glm::cross(right, front));
	}
};

#endif