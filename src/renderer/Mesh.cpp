#include "Mesh.hpp"

unsigned int Texture::load_texture_from_file(char const* path, std::string const& directory)
{
    auto filename{std::string(path)};
    filename = {directory + "/" + filename};

    unsigned int texture_id;
    glGenTextures(1, &texture_id);

    int width, height, n_components;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &n_components, 0);

    if (data) {
        GLenum format{};
        if (n_components == 1)
            format = GL_RED;
        else if (n_components == 3)
            format = GL_RGB;
        else if (n_components == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return texture_id;
}

Mesh::Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures)
    : m_vertices(vertices)
    , m_indices(indices)
    , m_textures(textures)
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
    unsigned int diffuse_nr = 1;
    unsigned int specular_nr = 1;
    unsigned int normal_nr = 1;
    unsigned int height_nr = 1;

    for (unsigned int i = 0; i < m_textures.size(); i++) {
        glActiveTexture(GL_TEXTURE0 + i);
        std::string number;
        std::string name{m_textures[i].m_type};

        // allows us to swap between textures
        if (name == "texture_diffuse")
            number = std::to_string(diffuse_nr++);
        else if (name == "texture_specular")
            number = std::to_string(specular_nr++);
        else if (name == "texture_normal")
            number = std::to_string(normal_nr++);
        else if (name == "texture_height")
            number = std::to_string(height_nr++);

        glUniform1i(glGetUniformLocation(shader.m_id, (name + number).c_str()), i);
        glBindTexture(GL_TEXTURE_2D, m_textures[i].m_id);
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
