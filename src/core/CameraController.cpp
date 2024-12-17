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
    auto movement_scalar = movement_speed * delta_time;

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
        auto look_scalar = rotation_speed * delta_time;
        pitch -= Input::cursor_delta().y * look_scalar;
        yaw += Input::cursor_delta().x * look_scalar;
    }

    auto const max_pitch = glm::half_pi<float>() - 0.001f;
    pitch = std::clamp(pitch, -max_pitch, max_pitch);

    camera->target = camera->position + yaw_pitch_to_direction(yaw, pitch);
}

// Turntable orbit (y axis keeps pointing up) with blender controls
void CameraController::update_orbit(float delta_time)
{
    // As viewed by the target torwards the camera
    auto forward = camera->position - camera->target;
    auto forward_normalized = glm::normalize(forward);
    auto right = glm::normalize(glm::cross(forward_normalized, camera->up));

    // Panning
    if (Input::button_pressed(GLFW_MOUSE_BUTTON_MIDDLE) && Input::key_pressed(GLFW_KEY_LEFT_SHIFT)) {
        auto panning_scalar = movement_speed * delta_time;

        auto screen_up = glm::cross(forward_normalized, right);
        camera->position -= screen_up * glm::vec3(Input::cursor_delta().y * panning_scalar);
        camera->target -= screen_up * glm::vec3(Input::cursor_delta().y * panning_scalar);

        camera->position += right * glm::vec3(Input::cursor_delta().x * panning_scalar);
        camera->target += right * glm::vec3(Input::cursor_delta().x * panning_scalar);
    }

    // Rotation
    if (Input::button_pressed(GLFW_MOUSE_BUTTON_MIDDLE) && !Input::key_pressed(GLFW_KEY_LEFT_SHIFT)) {
        auto rotation_scalar = rotation_speed * delta_time;

        auto [yaw, pitch] = direction_to_yaw_pitch(forward_normalized);
        yaw += Input::cursor_delta().x * rotation_scalar;
        pitch += Input::cursor_delta().y * rotation_scalar;

        auto const max_pitch = glm::half_pi<float>() - 0.001f;
        pitch = std::clamp(pitch, -max_pitch, max_pitch);

        camera->position = camera->target + yaw_pitch_to_direction(yaw, pitch) * glm::vec3{glm::length(forward)};
    }

    // Zoom
    auto scroll_y = static_cast<float>(Input::scroll_delta().y);
    if (scroll_y != 0.0) {
        auto zoom_step = zoom_speed * std::sqrt(glm::length(forward)) * -scroll_y;
        auto min_distance = 1.0f;
        auto camera_distance = std::abs(glm::distance(camera->position, camera->target));
        if (camera_distance + zoom_step < min_distance) {
            zoom_step = camera_distance - min_distance;
        }
        camera->position += forward_normalized * glm::vec3{zoom_step};
    }
}
