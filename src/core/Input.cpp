#include "core/Input.hpp"
#include <imgui.h>

GLFWwindow* Input::m_window;
glm::dvec2 Input::m_last_cursor_position;
glm::dvec2 Input::m_cursor_delta;
glm::dvec2 Input::m_scroll_delta;

void Input::init(GLFWwindow* window)
{
    m_window = window;
    glfwSetScrollCallback(window, scroll_callback);
}

void Input::update()
{
    glm::dvec2 cursor_position;
    glfwGetCursorPos(m_window, &cursor_position.x, &cursor_position.y);
    m_cursor_delta = cursor_position - m_last_cursor_position;
    m_last_cursor_position = cursor_position;
}

void Input::late_update()
{
    m_scroll_delta = glm::dvec2{0.0};
}

bool Input::key_pressed(int key)
{
    return glfwGetKey(m_window, key) == GLFW_PRESS;
}

bool Input::button_pressed(int button)
{
    return glfwGetMouseButton(m_window, button) == GLFW_PRESS;
}

glm::dvec2 Input::cursor_delta()
{
    return m_cursor_delta;
}

glm::dvec2 Input::scroll_delta()
{
    return m_scroll_delta;
}

void Input::scroll_callback(GLFWwindow*, double xoffset, double yoffset)
{
    m_scroll_delta = glm::dvec2{xoffset, yoffset};

    auto imgui_io = ImGui::GetIO();
    imgui_io.AddMouseWheelEvent(xoffset, yoffset);
}
