#pragma once

#include "core/Scene.hpp"
#include <imgui_internal.h>

struct ObjectSelectionTree {
    ObjectSelectionTree();
    void traverse_nodes(InstancedNode& root);
    void render();

private:
    std::optional<ImRect> m_prev_rect;
};
