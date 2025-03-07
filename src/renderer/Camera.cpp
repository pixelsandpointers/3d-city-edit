#include "renderer/Camera.hpp"

#include "core/Project.hpp"
#include <iostream>

Framebuffer Framebuffer::get_default(int width, int height)
{
    // Can't use Aggregate initialization here :(
    // ... At least I can't find out how with a deleted copy constructor.
    return Framebuffer{
        0,
        0,
        0,
        width,
        height,
        static_cast<float>(width) / static_cast<float>(height),
    };
}

Framebuffer Framebuffer::create_simple(int width, int height, Preset preset)
{
    auto fb = Framebuffer::get_default(width, height);

    switch (preset) {
    case Preset::RGB_UNSIGNED_INTEGRAL_NORMALIZED:
        fb.m_internal_format = GL_RGB;
        fb.m_format = GL_RGB;
        fb.m_type = GL_UNSIGNED_BYTE;
        break;
    case Preset::R_FLOAT:
        fb.m_internal_format = GL_R32F;
        fb.m_format = GL_RED;
        fb.m_type = GL_FLOAT;
        break;
    }

    glGenFramebuffers(1, &fb.id);
    glBindFramebuffer(GL_FRAMEBUFFER, fb.id);

    glGenTextures(1, &fb.color_texture);
    glBindTexture(GL_TEXTURE_2D, fb.color_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, fb.m_internal_format, fb.width, fb.height, 0, fb.m_format, fb.m_type, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fb.color_texture, 0);

    glGenRenderbuffers(1, &fb.depth_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, fb.depth_rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, fb.width, fb.height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fb.depth_rbo);

    auto framebuffer_ready = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    if (!framebuffer_ready) {
        std::cerr << "Framebuffer creation failed!\n";
        std::abort();
    }

    return fb;
}

Framebuffer::Framebuffer(Framebuffer&& other)
    : id{other.id}
    , color_texture{other.color_texture}
    , depth_rbo{other.depth_rbo}
    , width{other.width}
    , height{other.height}
    , aspect{other.aspect}
{
    // Destructor is called on old object after move
    other.id = 0;
    other.color_texture = 0;
    other.depth_rbo = 0;
}

Framebuffer::~Framebuffer()
{
    glDeleteFramebuffers(1, &id);
    glDeleteTextures(1, &color_texture);
    glDeleteRenderbuffers(1, &depth_rbo);
}

void Framebuffer::resize(int w, int h)
{
    width = w;
    height = h;
    aspect = static_cast<float>(w) / static_cast<float>(h);

    if (color_texture) {
        glBindTexture(GL_TEXTURE_2D, color_texture);
        glTexImage2D(GL_TEXTURE_2D, 0, m_internal_format, width, height, 0, m_format, m_type, NULL);
    }

    if (depth_rbo) {
        glBindRenderbuffer(GL_RENDERBUFFER, depth_rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    }
}

Camera::Camera(glm::vec3 position, glm::vec3 target, float fov)
    : position(position)
    , target(target)
    , fov(fov)
{
}

glm::mat4 Camera::view() const
{
    return glm::lookAt(position, target, up);
}

glm::mat4 Camera::projection(float aspect) const
{
    return glm::perspective(fov, aspect, near, far);
}

void Camera::draw(ViewingMode mode,
    Uniforms const& uniforms,
    Framebuffer const& framebuffer,
    InstancedNode const& node)
{
    auto const& shader = Shader::get_shader_for_mode(mode);

    shader.use();

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.id);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, framebuffer.width, framebuffer.height);

    // view/projection transformations
    shader.set_uniform(shader.uniform_locations.projection, projection(framebuffer.aspect));
    shader.set_uniform(shader.uniform_locations.view, view());
    shader.set_uniform(shader.uniform_locations.camera_pos, position);
    shader.set_uniform(shader.uniform_locations.ambient_strength, uniforms.ambient_strength);
    shader.set_uniform(shader.uniform_locations.specularity_factor, uniforms.specularity_factor);
    shader.set_uniform(shader.uniform_locations.shininess, uniforms.shininess);
    shader.set_uniform(shader.uniform_locations.gamma, uniforms.gamma);
    shader.set_uniform(shader.uniform_locations.light_direction, glm::vec3(uniforms.light.direction));
    shader.set_uniform(shader.uniform_locations.light_color, uniforms.light.color);
    shader.set_uniform(shader.uniform_locations.light_power, uniforms.light.power);

    node.traverse([&](auto transform_matrix, auto const& node_data) {
        shader.set_uniform(shader.uniform_locations.model, transform_matrix);
        for (auto const& mesh : node_data.meshes) {
            mesh.draw(mode);
        }
    });

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Camera::draw_outline(Framebuffer const& framebuffer, InstancedNode const& node)
{
    auto project = Project::get_current();
    if (framebuffer.width != m_mask_framebuffer.width || framebuffer.height != m_mask_framebuffer.height) {
        m_mask_framebuffer.resize(framebuffer.width, framebuffer.height);
    }

    glBindFramebuffer(GL_FRAMEBUFFER, m_mask_framebuffer.id);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // render selected node in white to m_mask_framebuffer (albedo shader with outline color texture)
    Shader::albedo.use();
    Shader::albedo.set_uniform(Shader::albedo.uniform_locations.projection, projection(framebuffer.aspect));
    Shader::albedo.set_uniform(Shader::albedo.uniform_locations.view, view());
    Shader::albedo.set_uniform(Shader::albedo.uniform_locations.gamma, 1.0f);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, project->white_texture()->id);
    Shader::albedo.set_uniform(Shader::albedo.uniform_locations.texture_diffuse, 0);

    node.traverse([&](auto transform_matrix, auto const& node_data) {
        Shader::albedo.set_uniform(Shader::albedo.uniform_locations.model, transform_matrix);
        for (auto const& mesh : node_data.meshes) {
            mesh.draw();
        }
    });

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer.id);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    Shader::post_process_outline.use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_mask_framebuffer.color_texture);

    Shader::post_process_outline.set_uniform(Shader::post_process_outline.uniform_locations.tex, 0);
    Shader::post_process_outline.set_uniform(Shader::post_process_outline.uniform_locations.color, glm::vec3{0.84f, 0.5f, 0.1f});

    draw_quad();

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Camera::draw_quad()
{
    if (m_quad_vao == 0) {
        float quad_vertices[] = {
            // clang-format off
            // positions  // texture Coords
            -1.0f, 1.0f,  0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f,
            1.0f,  1.0f,  1.0f, 1.0f,
            1.0f,  -1.0f, 1.0f, 0.0f,
            // clang-format on
        };
        // setup plane VAO
        glGenVertexArrays(1, &m_quad_vao);
        glGenBuffers(1, &m_quad_vbo);
        glBindVertexArray(m_quad_vao);
        glBindBuffer(GL_ARRAY_BUFFER, m_quad_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), &quad_vertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    }

    glBindVertexArray(m_quad_vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
