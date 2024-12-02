#pragma once

#include <assimp/scene.h>
#include <stb_image.h>
#include <glad/glad.h>
#include <iostream>
#include <string>

// TODO - might want to add PBR textures here instead of within the Model class.

struct Texture {
    unsigned int m_id;
    std::string m_type;
    std::string m_path;

    static unsigned int load_texture_from_file(const char *path, const std::string &directory);
    static std::vector<Texture> load_material_textures(aiMaterial *mat, aiTextureType type, std::string type_name);
};
