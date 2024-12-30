#pragma once

#include "core/Scene.hpp"

struct ObjectSelectionTree {
    ObjectSelectionTree();
    void traverse_nodes(InstancedNode const& root);
    void render(InstancedNode const& root);
};
