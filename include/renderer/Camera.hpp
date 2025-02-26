#pragma once

#include "core/Scene.hpp"
#include "renderer/Shader.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Framebuffer {
    unsigned int id;
    unsigned int color_texture;
    unsigned int depth_rbo;
    int width;
    int height;
    float aspect;

    static Framebuffer get_default(int width, int height);
    static Framebuffer create_simple(int width, int height);

    Framebuffer() = default;
    Framebuffer(Framebuffer&) = delete;
    Framebuffer(Framebuffer&&);
    ~Framebuffer();
    void resize(int width, int height);

private:
    // This is ugly and shouldn't be necessary...
    // Why is the definition of an Aggregate so strange?
    Framebuffer(unsigned int id, unsigned int color_texture, unsigned int depth_rbo, int width, int height, float aspect)
        : id{id}
        , color_texture{color_texture}
        , depth_rbo{depth_rbo}
        , width{width}
        , height{height}
        , aspect{aspect}
    { }
};

class Camera {
public:
    // camera Attributes
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up{0.0f, 1.0f, 0.0f};
    float near{100.0f};
    float far{100000.0f};

    // Vertical FOV in radians
    float fov;

    // constructor with vectors
    Camera(glm::vec3 position, glm::vec3 target, float fov = glm::radians(90.0f));

    void draw(ViewingMode, Uniforms const&, Framebuffer const&, InstancedNode const&);

    // The `draw` function must be called before `draw_outline`.
    void draw_outline(Framebuffer const&, InstancedNode const&);

private:
    Framebuffer m_mask_framebuffer{Framebuffer::create_simple(1, 1)};
    unsigned int m_quad_vao{0}, m_quad_vbo{0};

    void draw_quad();
};
