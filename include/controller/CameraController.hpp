#pragma once
#include "Event.hpp"
#include "renderer/Camera.hpp"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

class CameraController {
public:
    CameraController(Camera& camera, EventDispatcher& dispatcher);
    enum class Action {
        MOVE_FORWARD,
        MOVE_BACKWARD,
        MOVE_LEFT,
        MOVE_RIGHT,
        MOVE_UP,
        MOVE_DOWN,
        ROTATE_LEFT,
        ROTATE_RIGHT,
    };

    void on_mouse_movement(float xoffset, float yoffset) const;
    void on_mouse_scroll(double yoffset) const;
    void on_key_press(Action action, float delta) const;
    void setup_event_handlers(EventDispatcher& dispatcher) const;

private:
    Camera& m_camera;
    float m_speed = 10.f;
};
