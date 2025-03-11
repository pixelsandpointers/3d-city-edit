#pragma once

#include "core/CameraController.hpp"
#include "renderer/Camera.hpp"
#include "renderer/Picking.hpp"
#include <imgui.h>

struct Viewport {
    enum class GizmoOperation {
        TRANSLATE,
        ROTATE,
        SCALE,
    };

    GizmoOperation gizmo_operation{GizmoOperation::TRANSLATE};

    Viewport();
    void render(double delta_time);

private:
    Framebuffer m_framebuffer;
    Framebuffer m_blitted_framebuffer;
    CameraController m_camera_controller;
    Picking m_picker;
};
