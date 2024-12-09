#include "core/ModelLoader.hpp"

#include "core/AssetManager.hpp"
#include "renderer/Mesh.hpp"
#include "renderer/Texture.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <glad/glad.h>
#include <glm/gtc/quaternion.hpp>
#include <iostream>
#include <vector>

glm::vec3 ai_to_glm_vec(aiVector3D vector)
{
    return glm::vec3{vector.x, vector.y, vector.z};
}

std::vector<std::pair<std::string, Texture*>> load_material_textures(aiMaterial* mat, aiTextureType type, std::string type_name, std::filesystem::path directory)
{
    std::vector<std::pair<std::string, Texture*>> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i) {
        aiString string;
        mat->GetTexture(type, i, &string);
        auto filename = std::string{string.C_Str()};
        std::replace(filename.begin(), filename.end(), '\\', '/');

        auto texture_path = directory / filename;

        auto texture = AssetManager::get_texture(texture_path);
        if (!texture) {
            continue;
        }

        textures.push_back(std::make_pair(type_name + std::to_string(i), texture));
    }
    return textures;
}

Mesh process_mesh(aiMesh* mesh, aiScene const* scene, std::filesystem::path directory)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<std::pair<std::string, Texture*>> textures;

    for (unsigned int i = 0; i < mesh->mNumVertices; ++i) {
        Vertex vertex;
        glm::vec3 vector;
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.m_position = vector;

        // normals
        if (mesh->HasNormals()) {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.m_normal = vector;
        }

        // texture coordinates
        if (mesh->mTextureCoords[0]) {
            glm::vec2 tex_coord_vec;
            tex_coord_vec.x = mesh->mTextureCoords[0][i].x;
            tex_coord_vec.y = mesh->mTextureCoords[0][i].y;
            vertex.m_tex_coords = tex_coord_vec;
        } else {
            vertex.m_tex_coords = {0.0f, 0.0f};
        }

        vertices.push_back(vertex);

        // if you want to add more attributes do it here
    }

    for (unsigned int i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; ++j) {
            indices.push_back(face.mIndices[j]);
        }
    }

    // assign materials if any
    auto material = scene->mMaterials[mesh->mMaterialIndex];

    // diffuse
    auto diffuse = load_material_textures(material, aiTextureType_DIFFUSE, "texture_diffuse", directory);
    textures.insert(textures.end(), diffuse.begin(), diffuse.end());

    // specular
    auto specular = load_material_textures(material, aiTextureType_SPECULAR, "texture_specular", directory);
    textures.insert(textures.end(), specular.begin(), specular.end());

    // normals
    auto normal = load_material_textures(material, aiTextureType_NORMALS, "texture_normal", directory);
    textures.insert(textures.end(), normal.begin(), normal.end());

    // height map
    auto height = load_material_textures(material, aiTextureType_AMBIENT, "texture_height", directory);
    textures.insert(textures.end(), height.begin(), height.end());

    return Mesh{vertices, indices, textures};
}

Node process_node(aiNode* node, aiScene const* scene, std::filesystem::path directory)
{
    aiVector3D scale;
    aiQuaternion rotation;
    aiVector3D position;
    node->mTransformation.Decompose(scale, rotation, position);

    auto new_transform = Transform{
        .position = ai_to_glm_vec(position),
        .orientation = glm::quat{rotation.w, rotation.x, rotation.y, rotation.z},
        .scale = ai_to_glm_vec(scale),
    };

    auto new_node = Node::create(node->mName.C_Str(), new_transform);

    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        new_node.meshes.push_back(process_mesh(mesh, scene, directory));
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        new_node.children.push_back(process_node(node->mChildren[i], scene, directory));
    }

    return new_node;
}

std::optional<Node> ModelLoader::load_model(std::filesystem::path path)
{
    Assimp::Importer importer;
    aiScene const* scene = importer.ReadFile(path.string(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return {};
    }

    auto directory = path.parent_path();

    // node processing seems off
    return process_node(scene->mRootNode, scene, directory);
}
