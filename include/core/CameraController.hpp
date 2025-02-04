#pragma once

#include "renderer/Camera.hpp"
#include <glfw.h>
#include <memory>

struct CameraController {
    enum class Type {
        FREECAM,
        BLENDER,
        UNITY,
    };

    CameraController(Type, glm::vec3 camera_position);
    Type type;
    std::unique_ptr<Camera> camera;

    // Movement speed in distance / second
    float movement_speed{1000.0f};

    // Mouse sensitivity for looking around in radians / second
    float rotation_speed{0.2f};

    float zoom_speed{1.0f};

    void update(float delta_time);

private:
    void update_freecam(float delta_time);
    void update_blender(float delta_time);
    void update_unity(float delta_time);
};
