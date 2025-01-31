#include "renderer/Texture.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <iostream>

std::optional<Image> Image::load_from_file(char const* path)
{
    unsigned int texture_id;
    glGenTextures(1, &texture_id);

    int width, height, n_components;
    unsigned char* data = stbi_load(path, &width, &height, &n_components, 0);

    if (!data) {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        return {};
    }

    switch (n_components) {
    case 1:
    case 3:
    case 4:
        break;
    default:
        stbi_image_free(data);
        glDeleteTextures(1, &texture_id);
        return {};
    }

    return Image{
        .width = width,
        .height = height,
        .channels = n_components,
        .data = std::vector<unsigned char>(data, data + n_components * width * height),
    };
}

std::optional<Texture> Texture::load_from_image(Image image)
{
    unsigned int texture_id;
    glGenTextures(1, &texture_id);

    GLenum format{};
    GLenum internal_format{};
    switch (image.channels) {
    case 1:
        internal_format = GL_RED;
        format = GL_RED;
        break;
    case 3:
        internal_format = GL_SRGB;
        format = GL_RGB;
        break;
    case 4:
        internal_format = GL_SRGB_ALPHA;
        format = GL_RGB;
        break;
    default:
        glDeleteTextures(1, &texture_id);
        return {};
    }

    glBindTexture(GL_TEXTURE_2D, texture_id);
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, image.width, image.height, 0, format, GL_UNSIGNED_BYTE, image.data.data());
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return Texture{
        texture_id,
        image.width,
        image.height,
        image.channels,
        true,
    };
}

Texture Texture::fallback_placeholder(unsigned int id)
{
    return Texture{id, 0, 0, 0, false};
}

Texture Texture::single_color(glm::vec4 color)
{
    unsigned int id;
    glGenTextures(1, &id);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_FLOAT, glm::value_ptr(color));
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return Texture{id, 1, 1, 4, true};
}

Texture::Texture(Texture&& other)
    : m_id{other.m_id}
    , width{other.width}
    , height{other.height}
    , channels{other.channels}
{
    // Important: Destructor will be called after move!
    other.m_id = 0;
}

Texture& Texture::operator=(Texture&& other)
{
    if (this != &other) {
        m_id = other.m_id;
        width = other.width;
        height = other.height;
        channels = other.channels;

        // Important: Destructor will be called after move!
        other.m_id = 0;
    }

    return *this;
}

Texture::~Texture()
{
    if (is_loaded && m_id != 0) {
        glDeleteTextures(1, &m_id);
    }
}

Texture::Texture(unsigned int m_id, int width, int height, int channels, bool is_loaded)
    : m_id{m_id}
    , width{width}
    , height{height}
    , channels{channels}
    , is_loaded{is_loaded}
{ }
