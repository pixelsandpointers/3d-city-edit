#include "core/Project.hpp"

#include "core/ModelLoader.hpp"
#include <iostream>

bool path_starts_with(std::filesystem::path path, std::filesystem::path prefix)
{
    std::filesystem::path::iterator path_iterator = path.begin();
    std::filesystem::path::iterator path_iterator_end = path.end();
    std::filesystem::path::iterator prefix_iterator = prefix.begin();
    std::filesystem::path::iterator prefix_iterator_end = prefix.end();

    while (path_iterator != path_iterator_end && prefix_iterator != prefix_iterator_end) {
        if (*path_iterator != *prefix_iterator) {
            return false;
        }

        ++path_iterator;
        ++prefix_iterator;
    }

    if (prefix_iterator != prefix_iterator_end) {
        return false;
    }

    return true;
}

FSCacheNode* FSCacheNode::get_child(std::filesystem::path full_path)
{
    if (!path_starts_with(full_path, path)) {
        return nullptr;
    }

    for (auto& child : children) {
        if (auto* found_node = child.get_child(full_path); found_node && found_node->path == full_path) {
            return found_node;
        }
    }

    return this;
}

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
    rebuild_fs_cache();
}

FSCacheNode* Project::get_fs_cache()
{
    return m_fs_cache.get();
}

FSCacheNode* Project::get_fs_cache(std::filesystem::path path)
{
    if (!m_fs_cache) {
        return nullptr;
    }

    return m_fs_cache->get_child(path);
}

Texture* Project::get_texture(std::filesystem::path path)
{
    if (!path.is_absolute()) {
        std::cerr << "path " << path << " is not absolute";
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
        std::cerr << "path " << path << " is not absolute";
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

Node* Project::get_cached_model(std::filesystem::path path)
{
    if (!path.is_absolute()) {
        std::cerr << "path " << path << " is not absolute";
        return nullptr;
    }

    if (m_models.contains(path)) {
        return &m_models.at(path);
    }

    return nullptr;
}

bool case_insensitive_equals(std::string_view a_insensitive, std::string_view b_lower)
{
    if (a_insensitive.length() != b_lower.length()) {
        return false;
    }

    for (std::size_t i = 0; i < a_insensitive.length(); ++i) {
        if (std::tolower(a_insensitive[i]) != b_lower[i]) {
            return false;
        }
    }

    return true;
}

FSCacheNode::Type identify_file(std::filesystem::path path)
{
    if (std::filesystem::is_directory(path)) {
        return FSCacheNode::Type::DIRECTORY;
    }

    if (!std::filesystem::is_regular_file(path)) {
        return FSCacheNode::Type::OTHER;
    }

    // Using the file ending to identify the file type is very error-prone, but it is also performant.
    // A better solution that uses file magic should be implemented in the future.
    auto file_ending = path.extension().string();

    static std::string_view constexpr MODEL_ENDINGS[] = {".fbx", ".obj"};
    static std::string_view constexpr TEXTURE_ENDINGS[] = {".jpg", ".jpeg", ".png", ".tga", ".bmp", ".psd", ".gif"};

    for (auto const model_ending : MODEL_ENDINGS) {
        if (case_insensitive_equals(file_ending, model_ending)) {
            return FSCacheNode::Type::MODEL;
        }
    }

    for (auto const texture_ending : TEXTURE_ENDINGS) {
        if (case_insensitive_equals(file_ending, texture_ending)) {
            return FSCacheNode::Type::TEXTURE;
        }
    }

    return FSCacheNode::Type::OTHER;
}

void rebuild_fs_cache_helper(FSCacheNode& cache_node)
{
    if (!std::filesystem::is_directory(cache_node.path)) {
        return;
    }

    auto const mtime = std::filesystem::last_write_time(cache_node.path);
    if (cache_node.mtime == mtime) {
        for (auto& child : cache_node.children) {
            rebuild_fs_cache_helper(child);
        }
        return;
    }

    cache_node.mtime = mtime;
    cache_node.children.clear();

    for (auto& entry : std::filesystem::directory_iterator{cache_node.path}) {
        auto const file_type = identify_file(entry.path());

        auto new_node = cache_node.children.emplace_back(FSCacheNode{
            .path = entry.path(),
            .mtime = file_type == FSCacheNode::Type::DIRECTORY
                ? std::filesystem::file_time_type{}
                : std::filesystem::last_write_time(entry.path()),
            .type = file_type,
            .children = {},
        });

        if (file_type == FSCacheNode::Type::DIRECTORY) {
            rebuild_fs_cache_helper(new_node);
        }
    }
}

void Project::rebuild_fs_cache()
{
    if (!std::filesystem::is_directory(root)) {
        std::cerr << "Invalid project root path '" << root << "'\n";
        m_fs_cache = {};
        return;
    }

    if (!m_fs_cache) {
        m_fs_cache = std::make_unique<FSCacheNode>(FSCacheNode{
            .path = root,
            .mtime = {},
            .type = FSCacheNode::Type::DIRECTORY,
            .children = {},
        });
    }

    rebuild_fs_cache_helper(*m_fs_cache);
}

void Project::rebuild_fs_cache_timed(double current_time)
{
    auto const update_interval = 5.0;
    if (current_time - m_fs_cache_last_updated < update_interval) {
        return;
    }

    m_fs_cache_last_updated = current_time;
    rebuild_fs_cache();
}
