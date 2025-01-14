#pragma once

#include "core/Scene.hpp"

struct ObjectSelectionTree {
    ObjectSelectionTree();
    void traverse_nodes(InstancedNode& root);
    void render();
};
