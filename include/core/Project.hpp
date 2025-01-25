#pragma once

#include "core/Scene.hpp"
#include "renderer/Texture.hpp"
#include <filesystem>
#include <unordered_map>

// Needed when using glm::vec4 as key in std::unordered_map
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/hash.hpp"

struct FSCacheNode {
    enum class Type {
        DIRECTORY,
        MODEL,
        TEXTURE,
        OTHER,
    };

    std::filesystem::path path;
    std::filesystem::file_time_type mtime;
    Type type;
    std::vector<FSCacheNode> children;

    FSCacheNode* get_child(std::filesystem::path);
};

class Project {
public:
    std::filesystem::path root;
    InstancedNode* selected_node{nullptr};
    std::optional<InstancedNode> scene;

    static Project* get_current();
    static void load(std::filesystem::path);
    void store();

    FSCacheNode* get_fs_cache();
    FSCacheNode* get_fs_cache(std::filesystem::path);
    Texture const* get_texture(std::filesystem::path);
    Node* get_model(std::filesystem::path);
    Node* get_cached_model(std::filesystem::path);
    Node* get_node(NodeLocation);
    void rebuild_fs_cache_timed(double current_time);
    Texture const* fallback_texture() const;

private:
    static std::unique_ptr<Project> current;

    Texture m_fallback_texture;
    std::unordered_map<std::filesystem::path, Texture> m_textures;
    std::unordered_map<std::filesystem::path, Node> m_models;
    std::unique_ptr<FSCacheNode> m_fs_cache;
    double m_fs_cache_last_updated{0};

    Project(std::filesystem::path);
    void rebuild_fs_cache();
};
