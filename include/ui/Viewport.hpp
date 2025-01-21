#pragma once

#include "core/CameraController.hpp"
#include "renderer/Camera.hpp"
#include <imgui.h>

struct ShaderUniformPane;

struct Viewport {
    Viewport();
    // TODO: Move the shader stuff to some config (maybe as Project::config?) and remove the ShaderUniformPane parameter.
    void render(double delta_time, ShaderUniformPane const&);

private:
    Framebuffer m_framebuffer;
    Shader m_shader;
    ViewingMode m_last_viewing_mode;
    CameraController m_camera_controller;
};
