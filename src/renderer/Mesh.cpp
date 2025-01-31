#include "renderer/Mesh.hpp"

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<std::pair<std::string, Texture const*>> textures, AABB aabb)
    : m_vertices(vertices)
    , m_indices(indices)
    , m_textures(textures)
    , aabb(aabb)
{
    setup_mesh();
}

void Mesh::draw() const
{
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_indices.size(), GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void Mesh::draw(Shader& shader) const
{
    for (unsigned int i = 0; i < m_textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        glUniform1i(glGetUniformLocation(shader.m_id, m_textures[i].first.c_str()), i);
        glBindTexture(GL_TEXTURE_2D, m_textures[i].second->m_id);
    }
    // set active
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(m_indices.size()), GL_UNSIGNED_INT, nullptr);
    // unbind VAO
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
}

void Mesh::setup_mesh()
{
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertices.size() * sizeof(Vertex), &m_vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices.size() * sizeof(unsigned int), &m_indices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    // vertex normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_normal));
    glEnableVertexAttribArray(1);
    // vertex texture coords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_tex_coords));
    glEnableVertexAttribArray(2);
}

bool Mesh::is_fully_loaded() const
{
    for (auto const& texture : m_textures) {
        if (!texture.second->is_loaded) {
            return false;
        }
    }

    return true;
}

AABB AABB::merge(AABB const& other)
{
    return AABB{
        .min = glm::vec3{std::min(min.x, other.min.x), std::min(min.y, other.min.y), std::min(min.z, other.min.z)},
        .max = glm::vec3{std::max(max.x, other.max.x), std::max(max.y, other.max.y), std::max(max.z, other.max.z)},
    };
}
