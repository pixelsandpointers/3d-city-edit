#pragma once

#include "core/Project.hpp"

struct Node;

struct AssetBrowser {
    void render();

private:
    Node const* m_selected_node{nullptr};
    void traverse_model(Node const&);
    void traverse_directory(FSCacheNode const&);
};
