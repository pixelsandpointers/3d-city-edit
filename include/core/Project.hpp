#pragma once

#include "core/Scene.hpp"
#include "renderer/Texture.hpp"
#include <filesystem>
#include <unordered_map>

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
};

class Project {
public:
    std::filesystem::path root;

    static Project* get_current();
    static void load(std::filesystem::path);

    FSCacheNode* get_fs_cache();
    Texture* get_texture(std::filesystem::path);
    Node* get_model(std::filesystem::path);
    void rebuild_fs_cache();
    void rebuild_fs_cache_timed(double current_time);

private:
    static std::unique_ptr<Project> current;

    std::unordered_map<std::filesystem::path, Texture> m_textures;
    std::unordered_map<std::filesystem::path, Node> m_models;
    std::unique_ptr<FSCacheNode> m_fs_cache;
    double m_fs_cache_last_updated{0};

    Project(std::filesystem::path);
};
