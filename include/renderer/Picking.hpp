#pragma once

#include "core/Scene.hpp"
#include "renderer/Camera.hpp"
#include <glm/glm.hpp>

class Picking {
public:
    InstancedNode* get_selected_node(Camera const&, InstancedNode& scene, glm::vec2 cursor_position, glm::vec2 framebuffer_size);

private:
    Framebuffer m_framebuffer{Framebuffer::create_simple(1, 1, Framebuffer::Preset::R_FLOAT)};
};
