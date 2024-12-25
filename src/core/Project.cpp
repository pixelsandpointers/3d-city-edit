#include "core/Project.hpp"

#include "core/ModelLoader.hpp"
#include <iostream>

std::unique_ptr<Project> Project::current;

Project* Project::get_current()
{
    return current.get();
}

void Project::load(std::filesystem::path path)
{
    // Ugly workaround for `std::make_unique` not being able to access private constructors
    current = std::unique_ptr<Project>(new Project(path));
}

Project::Project(std::filesystem::path root)
    : root{root}
{
    // TODO: filesystem watcher
    // TODO: build filesystem structure representation
}

Texture* Project::get_texture(std::filesystem::path path)
{
    if (!path.is_absolute()) {
        std::cout << "path " << path << " is not absolute";
        return nullptr;
    }

    if (m_textures.contains(path)) {
        return &m_textures.at(path);
    }

    auto new_texture = Texture::load_texture_from_file(path.string().c_str());
    if (!new_texture.has_value()) {
        return nullptr;
    }

    m_textures.emplace(path, new_texture.value());
    return &m_textures.at(path);
}

Node* Project::get_model(std::filesystem::path path)
{
    if (!path.is_absolute()) {
        std::cout << "path " << path << " is not absolute";
        return nullptr;
    }

    if (m_models.contains(path)) {
        return &m_models.at(path);
    }

    auto new_model = ModelLoader::load_model(path);
    if (!new_model.has_value()) {
        return nullptr;
    }

    m_models.emplace(path, new_model.value());
    return &m_models.at(path);
}
