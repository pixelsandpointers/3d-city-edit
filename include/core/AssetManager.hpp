#pragma once

#include "core/Scene.hpp"
#include "renderer/Texture.hpp"
#include <filesystem>
#include <unordered_map>

struct AssetManager {
    static Texture* get_texture(std::filesystem::path);
    static Node* get_model(std::filesystem::path);

private:
    static std::unordered_map<std::filesystem::path, Texture> textures;
    static std::unordered_map<std::filesystem::path, Node> models;
};
