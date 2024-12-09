#pragma once

#include "core/Scene.hpp"
#include <filesystem>

struct ModelLoader {
    static std::optional<Node> load_model(std::filesystem::path path);
};
