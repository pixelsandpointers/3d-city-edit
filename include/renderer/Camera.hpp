#pragma once

#include "core/Scene.hpp"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Framebuffer {
    unsigned int id;
    int width;
    int height;
    float aspect;
};

enum class ViewingMode {
    FLAT,
    LID,
    WIREFRAME,
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

    void draw(Shader&,
        std::unordered_map<std::string, std::variant<int, float, bool, glm::vec2, glm::vec3, glm::vec4, glm::mat2, glm::mat3, glm::mat4>> const&,
        Framebuffer const&,
        InstancedNode const&);
};
