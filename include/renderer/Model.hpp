#pragma once

#include "Mesh.hpp"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <glad/glad.h>
#include <vector>

class Model {
public:
    std::vector<Texture> m_textures;
    std::vector<Mesh> m_meshes;
    std::string m_directory;

    Model(std::string const& path);
    ~Model() = default;
    void draw() const;
    void draw(Shader& shader) const;

private:
    void load_model(std::string const& path);
    void process_node(aiNode* node, aiScene const* scene);
    Mesh process_mesh(aiMesh* mesh, aiScene const* scene);

    // TODO - put this into the texture class
    std::vector<Texture> load_material_textures(aiMaterial* mat, aiTextureType type, std::string type_name);
};
