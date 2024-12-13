#pragma once

#include "core/Project.hpp"
#include "renderer/Camera.hpp"
#include "renderer/Shader.hpp"
#include <optional>
#include <variant>

struct Node;

struct AssetBrowser {
    AssetBrowser();
    void render();

private:
    /*
     * The value of `Node const*` must not be nullptr. A `std::reference_wrapper` is not useful here,
     * because `Node` doesn't implement `operator==` and thus cannot be compared.
     */
    using NodeVariantType = std::variant<Node const*, std::filesystem::path>;

    std::optional<NodeVariantType> m_selected_item;
    unsigned int m_preview_texture;
    unsigned int m_preview_depth_rbo;
    bool m_preview_ready{false};
    Shader m_preview_shader{ViewingMode::ALBEDO};
    Camera m_preview_camera{glm::vec3{}, glm::vec3{}};
    Framebuffer m_preview_framebuffer{.id = 0, .width = 200, .height = 200, .aspect = 1};
    Uniforms m_preview_uniforms;

    void traverse_model(Node const&);
    void traverse_directory(FSCacheNode const&);
    void render_model_preview();
    bool is_selected_item_equal(NodeVariantType);
};
