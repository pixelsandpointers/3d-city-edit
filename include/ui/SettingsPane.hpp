#pragma once

#include "core/CameraController.hpp"
#include "renderer/Shader.hpp"
#include <imgui.h>
#include <unordered_map>

struct SettingsPane {
    void render();

    bool draw_wireframe = false;
    glm::vec3 fallback_color{255, 255, 255};
    Uniforms uniforms{};
    ViewingMode viewing_mode = ViewingMode::RENDERED;
    CameraController::Type camera_type = CameraController::Type::UNITY;
    // Movement speed in distance / second
    float movement_speed{1000.0f};

    // Mouse sensitivity for looking around in radians / second
    float rotation_speed{0.2f};

    float zoom_speed{1.0f};
};
