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
    CameraController::Type camera_type = CameraController::Type::FREECAM;
};
