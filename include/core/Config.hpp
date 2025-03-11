#pragma once

#include "core/CameraController.hpp"
#include "renderer/Shader.hpp"

struct Config {
    // render
    ViewingMode viewing_mode{ViewingMode::RENDERED};
    Uniforms viewport_uniforms;
    bool draw_wireframe = false;
    glm::vec3 fallback_color{255, 255, 255};
    float fov{glm::radians(90.0f)}; // Vertical fov in radians
    float near{10.0f};
    float far{100000.0f};

    // camera
    CameraController::Type camera_controller_type{CameraController::Type::UNITY};
    float movement_speed{1000.0f}; // Movement speed in distance / second
    float rotation_speed{0.2f}; // Mouse sensitivity for looking around in radians / second
    float zoom_speed{5.0f};

    // FIXME: The CameraController breaks if position and target are equal
    glm::vec3 camera_position{0.0f};
    glm::vec3 camera_target{0.0f, 0.0f, -1.0f};

    bool gizmo_use_snap{true};
    float gizmo_snap_translation{100.0f};
    float gizmo_snap_rotation{10.0f};
    float gizmo_snap_scale{0.1f};
};
