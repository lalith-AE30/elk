#ifndef USER_INPUT_H
#define USER_INPUT_H

#include <glad/glad.h>
#include <glm/glm.hpp>

enum class KeyType {
	MOUSE,
	KEYBOARD
};

class KeyBind {
private:
	bool prev_state = GLFW_RELEASE;
	const KeyType type_;

	int key_state() {
		switch (type_) {
		case KeyType::MOUSE:
			return glfwGetMouseButton(window_, key_);
		case KeyType::KEYBOARD:
			return glfwGetKey(window_, key_);
		}
	}

public:
	const int key_;
	GLFWwindow* const window_;

	KeyBind(GLFWwindow* window, int key, KeyType type = KeyType::KEYBOARD) : window_(window), key_(key), type_(type) { }

	bool clicked() {
		bool click = (prev_state != GLFW_PRESS && key_state() == GLFW_PRESS);
		prev_state = key_state();
		return click;
	}

	bool down() {
		return (key_state() == GLFW_PRESS);
	}
};

struct Bindings {
	KeyBind forward,
		backward,
		left,
		right,
		space,
		lctrl,
		esc,
		left_mouse;
};

Bindings generate_bindings(GLFWwindow* window) {
	Bindings bindings = {
		KeyBind(window, GLFW_KEY_W),
		KeyBind(window, GLFW_KEY_S),
		KeyBind(window, GLFW_KEY_A),
		KeyBind(window, GLFW_KEY_D),
		KeyBind(window, GLFW_KEY_SPACE),
		KeyBind(window, GLFW_KEY_LEFT_CONTROL),
		KeyBind(window, GLFW_KEY_ESCAPE),
		KeyBind(window, GLFW_MOUSE_BUTTON_1, KeyType::MOUSE)
	};
	return bindings;
}


#endif // !USER_INPUT_H

