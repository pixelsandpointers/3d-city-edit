#pragma once

#include "core/Project.hpp"
#include "renderer/Camera.hpp"
#include "renderer/Shader.hpp"

struct Node;

struct AssetBrowser {
    AssetBrowser();
    void render();

private:
    Node const* m_selected_node{nullptr};
    unsigned int m_preview_texture;
    unsigned int m_preview_depth_rbo;
    bool m_preview_ready{false};
    Shader m_preview_shader{ViewingMode::ALBEDO};
    Camera m_preview_camera{glm::vec3{}, glm::vec3{}};
    Framebuffer m_preview_framebuffer{.id = 0, .width = 200, .height = 200, .aspect = 1};
    Uniforms m_preview_uniforms;

    void traverse_model(Node const&);
    void traverse_directory(FSCacheNode const&);
    void render_preview();
};
