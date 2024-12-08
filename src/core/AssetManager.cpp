#include "core/AssetManager.hpp"
#include <iostream>

std::unordered_map<std::filesystem::path, Texture> AssetManager::textures;

Texture* AssetManager::get_texture(std::filesystem::path path)
{
    if (!path.is_absolute()) {
        std::cout << "path " << path << " is not absolute";
        return nullptr;
    }

    if (textures.contains(path)) {
        return &textures.at(path);
    }

    auto new_texture = Texture::load_texture_from_file(path.string().c_str());
    if (!new_texture.has_value()) {
        return nullptr;
    }

    textures.emplace(path, new_texture.value());
    return &textures.at(path);
}
