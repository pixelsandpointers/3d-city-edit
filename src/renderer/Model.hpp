#pragma once
#include "Mesh.hpp"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <glad/glad.h>
#include <vector>

class Model {
public:
    std::vector<Texture> m_textures;
    std::vector<Mesh> m_meshes;
    std::string m_directory;

    Model(const std::string& path);
    ~Model() = default;
    void draw() const;
    void draw(Shader& shader) const;

private:
    void load_model(const std::string& path);
    void process_node(aiNode *node, const aiScene *scene);
    Mesh process_mesh(aiMesh *mesh, const aiScene *scene);

    // TODO - put this into the texture class
    std::vector<Texture> load_material_textures(aiMaterial *mat, aiTextureType type, std::string type_name);
};
