#pragma once

#include "renderer/Camera.hpp"
#include <glfw.h>
#include <memory>

struct CameraController {
    enum class Type {
        FREECAM,
        ORBIT,
    };

    CameraController(Type, glm::vec3 camera_position);
    Type type;
    std::unique_ptr<Camera> camera;

    // Movement speed in distance / second
    float speed{1000.0f};

    // Mouse sensitivity for looking around in radians / second
    float look_sensitivity{0.2f};

    void update(float delta_time);

private:
    void update_freecam(float delta_time);
    void update_orbit(float delta_time);
};
