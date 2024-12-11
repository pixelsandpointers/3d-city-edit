#pragma once

#include "renderer/Shader.hpp"
#include "renderer/Texture.hpp"

#include <glm/glm.hpp>
#include <stb_image.h>
#include <vector>

struct Vertex {
    glm::vec3 m_position;
    glm::vec3 m_normal;
    glm::vec2 m_tex_coords;
};

struct AABB {
    glm::vec3 min;
    glm::vec3 max;

    AABB merge(AABB const&);
};

class Mesh {
public:
    std::vector<Vertex> m_vertices;
    std::vector<unsigned int> m_indices;
    std::vector<std::pair<std::string, Texture*>> m_textures;
    AABB aabb;

    Mesh(std::vector<Vertex> vertices,
        std::vector<unsigned int> indices,
        std::vector<std::pair<std::string, Texture*>> textures,
        AABB);

    void draw() const;
    void draw(Shader& shader) const;

private:
    unsigned int m_vao, m_vbo, m_ebo;
    void setup_mesh();
};
