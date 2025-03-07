#include "renderer/Mesh.hpp"

#include "core/Project.hpp"

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, Texture const* texture_diffuse, Texture const* texture_opacity, AABB aabb)
    : m_vertices(vertices)
    , m_indices(indices)
    , m_texture_diffuse(texture_diffuse)
    , m_texture_opacity(texture_opacity)
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

void Mesh::draw(ViewingMode mode) const
{
    auto const& shader = Shader::get_shader_for_mode(mode);
    auto diffuse_texture_id = mode == ViewingMode::SOLID ? Project::get_current()->fallback_texture()->id : m_texture_diffuse->id;

    // set diffuse texture
    glActiveTexture(GL_TEXTURE0);
    shader.set_uniform(shader.uniform_locations.texture_diffuse, 0);
    glBindTexture(GL_TEXTURE_2D, diffuse_texture_id);

    // set opacity texture
    glActiveTexture(GL_TEXTURE1);
    shader.set_uniform(shader.uniform_locations.texture_opacity, 1);
    glBindTexture(GL_TEXTURE_2D, m_texture_opacity->id);

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
    return m_texture_diffuse->is_loaded;
}

AABB AABB::merge(AABB const& other)
{
    return AABB{
        .min = glm::vec3{std::min(min.x, other.min.x), std::min(min.y, other.min.y), std::min(min.z, other.min.z)},
        .max = glm::vec3{std::max(max.x, other.max.x), std::max(max.y, other.max.y), std::max(max.z, other.max.z)},
    };
}
