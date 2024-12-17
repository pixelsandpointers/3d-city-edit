#pragma once

#include <glfw.h>
#include <glm/glm.hpp>

struct Input {
    static void init(GLFWwindow*);
    static void update();
    static void late_update();
    static bool key_pressed(int key);
    static bool button_pressed(int key);
    static glm::dvec2 cursor_delta();
    static glm::dvec2 scroll_delta();

private:
    static GLFWwindow* m_window;
    static glm::dvec2 m_last_cursor_position;
    static glm::dvec2 m_cursor_delta;
    static glm::dvec2 m_scroll_delta;

    static void scroll_callback(GLFWwindow*, double, double);
};
