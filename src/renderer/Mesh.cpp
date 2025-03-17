#include "renderer/Mesh.hpp"

#include "core/Project.hpp"
#include <iostream>
#include <map>

bool IntermediateMesh::merge(IntermediateMesh const& other)
{
    std::size_t new_textures_count = 0;
    for (auto other_texture : other.textures) {
        auto found_it = std::find(textures.begin(), textures.end(), other_texture);
        if (found_it == textures.end()) {
            ++new_textures_count;
        }
    }

    if (textures.size() + new_textures_count > 16) {
        return false;
    }

    std::map<std::size_t, std::size_t> texture_mapping;
    for (std::size_t i = 0; auto other_texture : other.textures) {
        auto found_it = std::find(textures.begin(), textures.end(), other_texture);
        if (found_it == textures.end()) {
            texture_mapping[i] = textures.size();
            textures.push_back(other_texture);
        } else {
            texture_mapping[i] = found_it - textures.begin();
        }
        ++i;
    }

    for (auto vertex : other.vertices) {
        auto diffuse_index = texture_mapping[vertex.texture_indices >> 24];
        auto opacity_index = texture_mapping[(vertex.texture_indices >> 16) & 0xff];
        vertex.texture_indices = (diffuse_index << 24) | (opacity_index << 16);
        vertices.push_back(vertex);
    }

    auto index_offset = indices.size();
    for (auto index : other.indices) {
        indices.push_back(index + index_offset);
    }

    aabb = aabb.merge(other.aabb);
    return true;
}

Mesh::Mesh(IntermediateMesh mesh)
{
    m_indices_size = mesh.indices.size();
    textures = mesh.textures;

    setup_mesh(mesh.vertices, mesh.indices);
}

void Mesh::draw() const
{
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_indices_size, GL_UNSIGNED_INT, nullptr);
}

void Mesh::draw(ViewingMode mode) const
{
    auto const& shader = Shader::get_shader_for_mode(mode);

    for (std::size_t i = 0; auto const& texture : textures) {
        auto id = mode == ViewingMode::SOLID ? Project::get_current()->fallback_texture()->id : texture->id;

        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, id);
        shader.set_uniform(shader.uniform_locations.textures[i], static_cast<int>(i));

        ++i;
    }

    // set active
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, static_cast<unsigned int>(m_indices_size), GL_UNSIGNED_INT, nullptr);
}

void Mesh::setup_mesh(std::vector<Vertex> const& vertices, std::vector<unsigned int> const& indices)
{
    if (m_vao != 0) {
        return;
    }

    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_ebo);

    glBindVertexArray(m_vao);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    // vertex normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_normal));
    glEnableVertexAttribArray(1);
    // vertex texture coords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, m_tex_coords));
    glEnableVertexAttribArray(2);
    // texture index
    glVertexAttribIPointer(3, 1, GL_UNSIGNED_INT, sizeof(Vertex), (void*)offsetof(Vertex, texture_indices));
    glEnableVertexAttribArray(3);
}

bool Mesh::is_fully_loaded() const
{
    for (auto const& texture : textures) {
        if (!texture->is_loaded) {
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
