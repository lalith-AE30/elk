#include "user_input.h"

int KeyBind::key_state() {
    switch (type_) {
    case KeyType::MOUSE:
        return glfwGetMouseButton(window_, key_);
    case KeyType::KEYBOARD:
        return glfwGetKey(window_, key_);
    default:
        return GLFW_RELEASE;
    }
}

bool KeyBind::clicked() {
    bool click = (prev_state != GLFW_PRESS && key_state() == GLFW_PRESS);
    prev_state = key_state();
    return click;
}

bool KeyBind::down() {
    return (key_state() == GLFW_PRESS);
}

Bindings generate_empty_binding() {
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

Bindings generate_bindings(GLFWwindow* window) {
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