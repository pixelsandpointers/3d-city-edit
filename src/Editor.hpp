#pragma once
// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on

#include "Event.hpp"
#include "controller/CameraController.hpp"
#include "renderer/Camera.hpp"
#include "renderer/Model.hpp"
#include "renderer/Shader.hpp"

#include <filesystem>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

class Editor {
public:
    Editor(int width = 800, int height = 600);
    bool init();
    void run();
    void shutdown() const;

protected:
    float m_time_delta{0.f};

private:
    int m_window_width, m_window_height;
    GLFWwindow* m_window{nullptr};
    bool m_running;
    EventDispatcher m_event_dispatcher;
    std::unique_ptr<Camera> m_uber_camera;
    std::unique_ptr<CameraController> m_uber_camera_controller;
    void setup_imgui() const;
    void render();
};
