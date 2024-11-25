#ifndef MESH_HPP
#define MESH_HPP

#include <glm/glm.hpp>
#include <stb_image.h>

#include <string>
#include <vector>

#include "Shader.hpp"

struct Vertex {
    glm::vec3 m_position;
    glm::vec3 m_normal;
    glm::vec2 m_tex_coords;
};

struct Texture {
    unsigned int m_id;
    std::string m_type;
    std::string m_path;

    static unsigned int load_texture_from_file(const char *path, const std::string &directory);
};

class Mesh {
public:
    std::vector<Vertex> m_vertices;
    std::vector<unsigned int> m_indices;
    std::vector<Texture> m_textures;

    Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);

    void draw() const;
    void draw(Shader &shader) const;

private:
    unsigned int m_vao, m_vbo, m_ebo;
    void setup_mesh();
};

#endif // MESH_HPP
