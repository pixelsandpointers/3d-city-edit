#pragma once

#include "renderer/Texture.hpp"
#include <filesystem>
#include <unordered_map>

struct AssetManager {
    static Texture* get_texture(std::filesystem::path);

private:
    static std::unordered_map<std::filesystem::path, Texture> textures;
};
