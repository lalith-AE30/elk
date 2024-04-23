#ifndef USER_INPUT_H
#define USER_INPUT_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

enum class KeyType {
    MOUSE,
    KEYBOARD
};

class KeyBind {
private:
    bool prev_state = GLFW_RELEASE;
    const KeyType type_;

    int key_state();

public:
    const int key_;
    GLFWwindow* const window_;

    KeyBind(GLFWwindow* window, int key, KeyType type = KeyType::KEYBOARD) : window_(window), key_(key), type_(type) { }

    bool clicked();
    bool down();
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

Bindings generate_empty_binding();
Bindings generate_bindings(GLFWwindow* window);

#endif // !USER_INPUT_H