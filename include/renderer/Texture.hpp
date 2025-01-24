#pragma once

#include <assimp/scene.h>
#include <glad/glad.h>
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

struct Texture {
    unsigned int m_id;
    int width;
    int height;
    int channels;

    [[nodiscard]] bool is_loaded() const { return m_id != 0; }

    static std::optional<Texture> load_from_image(Image);
    static Texture placeholder();

    Texture(Texture const&) = delete;
    Texture() = default;
    Texture(Texture&&);
    Texture& operator=(Texture const&) = delete;
    Texture& operator=(Texture&&);
    ~Texture();

private:
    Texture(unsigned int m_id, int width, int height, int channels);
};
