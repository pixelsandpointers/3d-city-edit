#pragma once
#include <glm/glm.hpp>
#include <stb_image.h>

#include <vector>

#include "Shader.hpp"
#include "Texture.hpp"

struct Vertex {
    glm::vec3 m_position;
    glm::vec3 m_normal;
    glm::vec2 m_tex_coords;
};


class Mesh {
public:
    std::vector<Vertex> m_vertices;
    std::vector<unsigned int> m_indices;
    std::vector<Texture> m_textures;

    Mesh(std::vector<Vertex> vertices,
         std::vector<unsigned int> indices,
         std::vector<Texture> textures);

    void draw() const;
    void draw(Shader &shader) const;

private:
    unsigned int m_vao, m_vbo, m_ebo;
    void setup_mesh();
};
