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
    Framebuffer m_model_preview_framebuffer;
    Camera m_model_preview_camera{glm::vec3{}, glm::vec3{}};
    Uniforms m_model_preview_uniforms;
    unsigned int m_preview_texture;
    std::string m_preview_name;
    bool m_preview_dirty{true};

    void traverse_model(Node const&);
    void traverse_directory(FSCacheNode const&);
    void prepare_preview();
    void render_model_preview();
    bool is_selected_item_equal(NodeVariantType);
};
