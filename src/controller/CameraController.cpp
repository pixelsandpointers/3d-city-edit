#include "controller/CameraController.hpp"

CameraController::CameraController(Camera& camera, EventDispatcher& dispatcher)
    : m_camera(camera)
{
    setup_event_handlers(dispatcher);
}

void CameraController::on_mouse_movement(float xoffset, float yoffset) const
{
    m_camera.m_yaw += xoffset * m_camera.m_sensitivity;
    m_camera.m_pitch += yoffset * m_camera.m_sensitivity;

    if (m_camera.m_pitch > 89.f)
        m_camera.m_pitch = 89.f;
    if (m_camera.m_pitch < -89.f)
        m_camera.m_pitch = -89.f;
}

void CameraController::on_mouse_scroll(double yoffset) const
{
    m_camera.m_fov -= yoffset;
    if (m_camera.m_fov < 1.f)
        m_camera.m_fov = 1.f;
    if (m_camera.m_fov > 45.f)
        m_camera.m_fov = 45.f;
}

void CameraController::on_key_press(Action const action, float const delta) const
{
    float const velocity = m_speed * delta;
    glm::vec3 front = {
        cos(glm::radians(m_camera.m_yaw)) * cos(glm::radians(m_camera.m_pitch)),
        sin(glm::radians(m_camera.m_pitch)),
        sin(glm::radians(m_camera.m_yaw)) * cos(glm::radians(m_camera.m_pitch))};
    front = glm::normalize(front);

    constexpr glm::vec3 up{0.f, 1.f, 0.f};
    auto const right = glm::normalize(glm::cross(front, up));

    if (action == Action::MOVE_UP) {
        m_camera.m_position += up * velocity;
    } else if (action == Action::MOVE_DOWN) {
        m_camera.m_position -= up * velocity;
    } else if (action == Action::MOVE_LEFT) {
        m_camera.m_position -= right * velocity;
    } else if (action == Action::MOVE_RIGHT) {
        m_camera.m_position += right * velocity;
    }
}

void CameraController::setup_event_handlers(EventDispatcher& dispatcher) const
{
    dispatcher.register_handler(EventType::MOUSE_MOVE, [this](Event const& event) {
        static bool first_mouse = true;
        static double last_x, last_y;

        double xpos = event.mouse_button_event.x;
        double ypos = event.mouse_button_event.y;

        if (first_mouse) {
            last_x = xpos;
            last_y = ypos;
            first_mouse = false;
        }

        double xoffset = xpos - last_x;
        double yoffset = last_y - ypos;

        last_x = xpos;
        last_y = ypos;

        on_mouse_movement(xoffset, yoffset);
    });

    dispatcher.register_handler(EventType::MOUSE_SCROLL, [this](Event const& event) {
        on_mouse_scroll(event.mouse_scroll_event.yoffset);
    });

    dispatcher.register_handler(EventType::KEY_PRESS, [this](Event const& event) {
        switch (event.key_event.key_code) {
        case GLFW_KEY_W:
            on_key_press(Action::MOVE_UP, event.time_delta);
            break;
        case GLFW_KEY_S:
            on_key_press(Action::MOVE_DOWN, event.time_delta);
            break;
        case GLFW_KEY_A:
            on_key_press(Action::MOVE_LEFT, event.time_delta);
            break;
        case GLFW_KEY_D:
            on_key_press(Action::MOVE_RIGHT, event.time_delta);
            break;
        default:
            break;
        }
    });

    // TODO: add resize event
    // dispatcher.register_handler(EventType::WINDOW_RESIZE, [this](Event const& event) {
    //
    // });
}
