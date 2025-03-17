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

    // first byte diffuse
    // second byte opacity
    uint32_t texture_indices;
};

struct AABB {
    glm::vec3 min;
    glm::vec3 max;

    AABB merge(AABB const&);
};

struct IntermediateMesh {
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture const*> textures;
    AABB aabb;

    bool merge(IntermediateMesh const&);
};

class Mesh {
public:
    std::vector<Texture const*> textures;
    AABB aabb;

    Mesh(IntermediateMesh);

    void draw() const;
    void draw(ViewingMode) const;
    [[nodiscard]] bool is_fully_loaded() const;
    void setup_mesh(std::vector<Vertex> const& vertices, std::vector<unsigned int> const& indices);

private:
    unsigned int m_vao{0}, m_vbo{0}, m_ebo{0};
    unsigned int m_indices_size;
};
