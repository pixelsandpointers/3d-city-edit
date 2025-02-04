#include "renderer/Camera.hpp"

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

Framebuffer Framebuffer::create_simple(int width, int height)
{
    auto fb = Framebuffer::get_default(width, height);

    glGenFramebuffers(1, &fb.id);
    glBindFramebuffer(GL_FRAMEBUFFER, fb.id);

    glGenTextures(1, &fb.color_texture);
    glBindTexture(GL_TEXTURE_2D, fb.color_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, fb.width, fb.height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
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
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
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

// add viewing type => lid, wireframe, etc
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

    glm::mat4 projection = glm::perspective(fov, framebuffer.aspect, near, far);
    auto view = glm::lookAt(position, target, up);

    // view/projection transformations
    shader.set_mat4("projection", projection);
    shader.set_mat4("view", view);
    shader.set_vec3("cameraPos", position);
    shader.set_float("ambientStrength", uniforms.ambient_strength);
    shader.set_float("specularityFactor", uniforms.specularity_factor);
    shader.set_float("shininess", uniforms.shininess);
    shader.set_float("gamma", uniforms.gamma);
    shader.set_vec3("light.direction", glm::vec3(uniforms.light.direction));
    shader.set_vec3("light.color", uniforms.light.color);
    shader.set_float("light.power", uniforms.light.power);

    node.traverse([&](auto transform_matrix, auto const& node_data) {
        for (auto const& mesh : node_data.meshes) {
            shader.set_mat4("model", transform_matrix);
            mesh.draw(mode);
        }
    });

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
