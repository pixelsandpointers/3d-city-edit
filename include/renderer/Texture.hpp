#pragma once

#include <assimp/scene.h>
#include <glad/glad.h>
#include <optional>
#include <stb_image.h>
#include <string>
#include <vector>

// TODO - might want to add PBR textures here instead of within the Model class.

struct Texture {
    unsigned int m_id;
    int width;
    int height;
    int channels;

    static std::optional<Texture> load_texture_from_file(char const* path);
    static std::vector<Texture> load_material_textures(aiMaterial* mat, aiTextureType type, std::string type_name);
};
