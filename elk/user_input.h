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
        default:
            return GLFW_RELEASE;
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

// TODO make bindings extensible
struct Bindings {
    KeyBind
        key_w,
        key_s,
        key_a,
        key_d,
        key_space,
        key_lctrl,
        key_esc,
        key_up,
        key_down,
        mouse_left;
};

inline Bindings generate_empty_binding() {
    Bindings bindings = {
    KeyBind(NULL, GLFW_KEY_W),
    KeyBind(NULL, GLFW_KEY_S),
    KeyBind(NULL, GLFW_KEY_A),
    KeyBind(NULL, GLFW_KEY_D),
    KeyBind(NULL, GLFW_KEY_SPACE),
    KeyBind(NULL, GLFW_KEY_LEFT_CONTROL),
    KeyBind(NULL, GLFW_KEY_ESCAPE),
    KeyBind(NULL, GLFW_KEY_UP),
    KeyBind(NULL, GLFW_KEY_DOWN),
    KeyBind(NULL, GLFW_MOUSE_BUTTON_1, KeyType::MOUSE)
    };
    return bindings;
}

inline Bindings generate_bindings(GLFWwindow* window) {
    Bindings bindings = {
        KeyBind(window, GLFW_KEY_W),
        KeyBind(window, GLFW_KEY_S),
        KeyBind(window, GLFW_KEY_A),
        KeyBind(window, GLFW_KEY_D),
        KeyBind(window, GLFW_KEY_SPACE),
        KeyBind(window, GLFW_KEY_LEFT_CONTROL),
        KeyBind(window, GLFW_KEY_ESCAPE),
        KeyBind(window, GLFW_KEY_UP),
        KeyBind(window, GLFW_KEY_DOWN),
        KeyBind(window, GLFW_MOUSE_BUTTON_1, KeyType::MOUSE)
    };
    return bindings;
}

#endif // !USER_INPUT_H