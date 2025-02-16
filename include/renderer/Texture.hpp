#pragma once

#include <assimp/scene.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <optional>
#include <stb_image.h>
#include <vector>

struct Image {
    int width;
    int height;
    int channels;
    std::vector<unsigned char> data;

    static std::optional<Image> load_from_file(char const* path);
};

struct ColorTexture;

struct Texture {
    unsigned int m_id;
    int width;
    int height;
    int channels;
    bool is_loaded{false};

    static std::optional<Texture> load_from_image(Image);
    static Texture fallback_placeholder(unsigned int id);
    static ColorTexture single_color(glm::vec4 color);

    Texture(Texture const&) = delete;
    Texture() = default;
    Texture(Texture&&);
    Texture& operator=(Texture const&) = delete;
    Texture& operator=(Texture&&);
    ~Texture();

protected:
    Texture(unsigned int m_id, int width, int height, int channels, bool is_loaded);
};

struct ColorTexture : public Texture {
    glm::vec4 color();
    void color(glm::vec4);

private:
    friend Texture;

    ColorTexture(unsigned int id, glm::vec4 color);

    glm::vec4 m_color;
};
