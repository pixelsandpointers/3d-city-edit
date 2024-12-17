#include "core/CameraController.hpp"

#include "core/Input.hpp"
#include <glm/ext/scalar_constants.hpp>

CameraController::CameraController(Type type, glm::vec3 camera_position)
    : type(type)
{
    auto direction = glm::vec3{0.0f, 0.0f, -1.0f};
    camera = std::make_unique<Camera>(camera_position, camera_position + direction);
}

void CameraController::update(float delta_time)
{
    switch (type) {
    case Type::FREECAM:
        update_freecam(delta_time);
        break;
    case Type::ORBIT:
        update_orbit(delta_time);
        break;
    }
}

std::pair<float, float> direction_to_yaw_pitch(glm::vec3 direction)
{
    float pitch = std::asin(direction.y);
    float yaw = std::atan2(direction.z, direction.x);

    return std::make_pair(yaw, pitch);
}

glm::vec3 yaw_pitch_to_direction(float yaw, float pitch)
{
    return glm::vec3{
        std::cos(yaw) * std::cos(pitch),
        std::sin(pitch),
        std::sin(yaw) * std::cos(pitch),
    };
}

void CameraController::update_freecam(float delta_time)
{
    auto forward = glm::normalize(camera->target - camera->position);
    auto right = glm::normalize(glm::cross(forward, camera->up));
    auto camera_up = glm::cross(right, forward);

    // Movement
    auto movement_scalar = speed * delta_time;

    if (Input::key_pressed(GLFW_KEY_W)) {
        camera->position += forward * movement_scalar;
    }
    if (Input::key_pressed(GLFW_KEY_S)) {
        camera->position -= forward * movement_scalar;
    }
    if (Input::key_pressed(GLFW_KEY_D)) {
        camera->position += right * movement_scalar;
    }
    if (Input::key_pressed(GLFW_KEY_A)) {
        camera->position -= right * movement_scalar;
    }
    if (Input::key_pressed(GLFW_KEY_SPACE)) {
        camera->position += camera_up * movement_scalar;
    }
    if (Input::key_pressed(GLFW_KEY_LEFT_SHIFT)) {
        camera->position -= camera_up * movement_scalar;
    }

    // Look
    auto [yaw, pitch] = direction_to_yaw_pitch(forward);

    if (Input::button_pressed(GLFW_MOUSE_BUTTON_MIDDLE)) {
        auto look_scalar = look_sensitivity * delta_time;
        pitch -= Input::cursor_delta().y * look_scalar;
        yaw += Input::cursor_delta().x * look_scalar;
    }

    auto const max_pitch = glm::half_pi<float>() - 0.001f;
    pitch = std::clamp(pitch, -max_pitch, max_pitch);

    camera->target = camera->position + yaw_pitch_to_direction(yaw, pitch);
}

void CameraController::update_orbit(float delta_time)
{
    // TODO: Implement orbit controller
}
