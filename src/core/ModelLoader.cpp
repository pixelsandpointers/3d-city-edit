#include "core/ModelLoader.hpp"

#include "core/Project.hpp"
#include "renderer/Mesh.hpp"
#include "renderer/Texture.hpp"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <fstream>
#include <glad/glad.h>
#include <glm/gtc/quaternion.hpp>
#include <iostream>
#include <vector>

glm::vec3 ai_to_glm_vec(aiVector3D vector)
{
    return glm::vec3{vector.x, vector.y, vector.z};
}

/*
 * Some texture paths in the given model are incorrect. This function makes some guesses
 * that correct most of them.
 */
std::optional<std::filesystem::path> guess_texture_path(std::filesystem::path directory, std::string texture_path_string)
{
    std::replace(texture_path_string.begin(), texture_path_string.end(), '\\', '/');
    auto texture_path = std::filesystem::path{texture_path_string};

    // TODO: this fails on Windows without try/catch - requires investigation
    try {
        // 1. Attempt: maybe the provided path is correct.
        if (std::filesystem::exists(directory / texture_path)) {
            return directory / texture_path;
        }

        // 2. Attempt: maybe the file is in a subdirectory.
        auto search_in_subdirectory = [&](std::string filename) -> std::optional<std::filesystem::path> {
            for (auto const& entry : std::filesystem::directory_iterator{directory}) {
                if (std::filesystem::exists(entry.path() / filename)) {
                    return entry.path() / filename;
                }
            }
            return {};
        };

        auto filename = texture_path.filename().string();
        if (auto optional_path = search_in_subdirectory(filename); optional_path.has_value()) {
            return optional_path.value();
        }

        // 3. Attempt: maybe the file is in a subdirectory and it ends on '..bmp', but the file ends on '_.bmp' (wtf?).
        if (filename.ends_with("..bmp")) {
            filename.erase(filename.size() - 5);
            filename += "_.bmp";
            if (auto optional_path = search_in_subdirectory(filename); optional_path.has_value()) {
                return optional_path.value();
            }
        }
    } catch (std::exception const& e) {
        std::cerr << "Failed to guess texture path: " << e.what() << "\n";
    }
    // Fail
    return {};
}

Texture const* load_material_texture(aiMaterial* mat, aiTextureType type, std::filesystem::path directory)
{
    if (mat->GetTextureCount(type) == 0) {
        return nullptr;
    }

    aiString string;
    mat->GetTexture(type, 0, &string);
    auto filename = std::string{string.C_Str()};

    auto texture_path = guess_texture_path(directory, filename);
    if (!texture_path.has_value()) {
        std::cout << "Failed to guess path for bogus texture name '" << filename << "'\n";
        return nullptr;
    }

    assert(Project::get_current() != nullptr);
    auto texture = Project::get_current()->get_texture(texture_path.value());

    return texture;
}

Texture const* load_mask_texture(aiMaterial* mat, std::filesystem::path directory)
{
    if (mat->GetTextureCount(aiTextureType_DIFFUSE) == 0) {
        return nullptr;
    }

    aiString string;
    mat->GetTexture(aiTextureType_DIFFUSE, 0, &string);
    auto filename = std::string{string.C_Str()};

    auto texture_path = guess_texture_path(directory, filename);
    if (!texture_path.has_value()) {
        std::cout << "Failed to guess path for bogus texture name '" << filename << "'\n";
        return nullptr;
    }

    auto project = Project::get_current();
    assert(project);
    auto material_cache_node_path = texture_path.value().replace_extension(".mat");

    try {
        auto material_file_stream = std::ifstream{material_cache_node_path};

        std::string line;
        bool in_mask_line = false;
        std::optional<std::string> mask_texture_guid;
        while (std::getline(material_file_stream, line)) {
            if (in_mask_line) {
                auto start_pos = line.find("guid: ");
                if (start_pos == std::string::npos) {
                    return nullptr;
                }
                auto const guid_length = 32;
                mask_texture_guid = line.substr(start_pos + 6, guid_length);
                break;
            }

            auto start_pos = line.find_first_not_of(" ");
            if (line.compare(start_pos, 7, "- _Mask") != 0) {
                continue;
            }

            in_mask_line = true;
        }

        if (!mask_texture_guid.has_value()) {
            return nullptr;
        }

        auto mask_path = project->get_fs_cache_from_guid(mask_texture_guid.value());
        if (!mask_path.has_value()) {
            return nullptr;
        }

        return project->get_texture(mask_path.value());
    } catch (std::exception&) {
        return nullptr;
    }
}

Mesh process_mesh(aiMesh* mesh, aiScene const* scene, std::filesystem::path directory)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;

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

    auto texture_diffuse = load_material_texture(material, aiTextureType_DIFFUSE, directory);
    if (!texture_diffuse) {
        texture_diffuse = Project::get_current()->fallback_texture();
    }

    // TODO: Find mask texture
    // Each texture has a <texture name>.meta file that includes a guid
    // And each material has a list of linked textures that include a base and mask texture guid
    auto texture_opacity = load_mask_texture(material, directory);
    if (!texture_opacity) {
        texture_opacity = Project::get_current()->white_texture();
    }

    auto aabb = AABB{
        .min = ai_to_glm_vec(mesh->mAABB.mMin),
        .max = ai_to_glm_vec(mesh->mAABB.mMax),
    };

    return Mesh{vertices, indices, texture_diffuse, texture_opacity, aabb};
}

Node process_node(aiNode* node, aiScene const* scene, std::filesystem::path directory, NodeLocation parent_location)
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

    auto name = node->mName.C_Str();

    auto location = NodeLocation::file(parent_location.file_path, parent_location.node_path / name);
    auto new_node = Node::create(name, new_transform, location);

    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        new_node.meshes.push_back(process_mesh(mesh, scene, directory));
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        new_node.children.push_back(process_node(node->mChildren[i], scene, directory, location));
    }

    return new_node;
}

std::optional<Node> ModelLoader::load_model(std::filesystem::path path)
{
    Assimp::Importer importer;
    aiScene const* scene = importer.ReadFile(path.string(), aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace | aiProcess_GenBoundingBoxes);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return {};
    }

    auto directory = path.parent_path();

    auto root_node_location = NodeLocation::file(path, "/");

    // node processing seems off
    return process_node(scene->mRootNode, scene, directory, root_node_location);
}
