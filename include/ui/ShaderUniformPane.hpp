#pragma once

#include "renderer/Shader.hpp"
#include <imgui.h>
#include <unordered_map>

struct ShaderUniformPane {
    void render();

    bool draw_wireframe = false;
    Uniforms uniforms{};
    ViewingMode viewing_mode = ViewingMode::RENDERED;
};
