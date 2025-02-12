#pragma once

#include "core/CameraController.hpp"
#include "renderer/Camera.hpp"
#include <imgui.h>

struct SettingsPane;

struct Viewport {
    Viewport();
    // TODO: Move the shader stuff to some config (maybe as Project::config?) and remove the SettingsPane parameter.
    void render(double delta_time, SettingsPane const&);

private:
    Framebuffer m_framebuffer;
    CameraController m_camera_controller;
};
