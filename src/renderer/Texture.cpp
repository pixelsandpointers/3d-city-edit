#include "renderer/Texture.hpp"

#include <iostream>

unsigned int Texture::load_texture_from_file(char const* path)
{
    unsigned int texture_id;
    glGenTextures(1, &texture_id);

    int width, height, n_components;
    unsigned char* data = stbi_load(path, &width, &height, &n_components, 0);

    if (data) {
        GLenum format{};
        switch (n_components) {
        case 1:
            format = GL_RED;
            break;
        case 3:
            format = GL_RGB;
            break;
        case 4:
            format = GL_RGBA;
            break;
        default:
            stbi_image_free(data);
            glDeleteTextures(1, &texture_id);
            return 0;
        }

        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    } else {
        glDeleteTextures(1, &texture_id);
        std::cout << "Texture failed to load at path: " << path << std::endl;
        return 0;
    }

    return texture_id;
}
