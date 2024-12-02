#include "Model.hpp"

Model::Model(std::string const& path)
{
    load_model(path);
}

void Model::draw() const
{
    for (auto &mesh: m_meshes)
        mesh.draw();
}

void Model::draw(Shader& shader) const
{
    for (unsigned int i = 0; i < m_meshes.size(); ++i)
        m_meshes[i].draw(shader);
}

void Model::load_model(std::string const& path)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        std::cout << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
        return;
    }

    m_directory = path.substr(0, path.find_last_of('/'));

    // node processing seems off
    process_node(scene->mRootNode, scene);
}
void Model::process_node(aiNode* node, aiScene const* scene)
{
    for (unsigned int i = 0; i < node->mNumMeshes; ++i) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        m_meshes.push_back(process_mesh(mesh, scene));
    }
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        process_node(node->mChildren[i], scene);
    }
}
Mesh Model::process_mesh(aiMesh* mesh, aiScene const* scene)
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::vector<Texture> textures;

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
    std::vector<Texture> diffuse = load_material_textures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuse.begin(), diffuse.end());

    // specular
    std::vector<Texture> specular = load_material_textures(material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specular.begin(), specular.end());

    // normals
    std::vector<Texture> normal = load_material_textures(material, aiTextureType_NORMALS, "texture_normal");
    textures.insert(textures.end(), normal.begin(), normal.end());

    // height map
    std::vector<Texture> height = load_material_textures(material, aiTextureType_AMBIENT, "texture_height");
    textures.insert(textures.end(), height.begin(), height.end());

    return Mesh{vertices, indices, textures};
}

std::vector<Texture> Model::load_material_textures(aiMaterial* mat, aiTextureType type, std::string type_name)
{
    std::vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); ++i) {
        aiString string;
        mat->GetTexture(type, i, &string);
        bool skip = false;
        for (unsigned int j = 0; j < m_textures.size(); ++j) {
            if (m_textures[j].m_path == string.C_Str()) {
                textures.push_back(m_textures[j]);
                skip = true;
                break;
            }
        }

        if (!skip) {
            Texture texture{
                Texture::load_texture_from_file(string.C_Str(), m_directory),
                type_name,
                string.C_Str()};

            textures.push_back(texture);
            m_textures.push_back(texture);
        }
    }
    return textures;
}
