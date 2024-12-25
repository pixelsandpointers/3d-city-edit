#pragma once

#include "core/Scene.hpp"
#include "renderer/Texture.hpp"
#include <filesystem>
#include <unordered_map>

class Project {
public:
    std::filesystem::path root;

    static Project* get_current();
    static void load(std::filesystem::path);

    Texture* get_texture(std::filesystem::path);
    Node* get_model(std::filesystem::path);

private:
    static std::unique_ptr<Project> current;

    std::unordered_map<std::filesystem::path, Texture> m_textures;
    std::unordered_map<std::filesystem::path, Node> m_models;

    Project(std::filesystem::path);
};
