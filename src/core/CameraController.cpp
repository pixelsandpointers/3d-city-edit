#include "core/CameraController.hpp"

#include "core/Input.hpp"
#include <glm/ext/scalar_constants.hpp>

CameraController::CameraController(Type type, glm::vec3 camera_position)
    : type(type)
{
    auto direction = glm::vec3{0.0f, 0.0f, -1.0f};
    camera = std::make_unique<Camera>(camera_position, camera_position + direction);
}

void CameraController::update(float delta_time, bool handle_input, bool handle_scroll)
{
    if (m_animation.active) {
        update_animation(delta_time);
        return;
    }

    if (!handle_input) {
        return;
    }

    switch (type) {
    case Type::FREECAM:
        update_freecam(delta_time, handle_scroll);
        break;
    case Type::BLENDER:
        update_blender(delta_time, handle_scroll);
        break;
    case Type::UNITY:
        update_unity(delta_time, handle_scroll);
        break;
    }
}

void CameraController::animate_to(glm::vec3 position, glm::vec3 target, float duration)
{
    m_animation.active = true;
    m_animation.position = position;
    m_animation.target = target;
    m_animation.remaining_time = duration;
}

void CameraController::focus_on(InstancedNode const& node)
{
    auto const duration = 0.3f;

    auto const world_center = glm::vec3{node.model_matrix * glm::vec4{0.0f, 0.0f, 0.0f, 1.0f}};
    auto radius = 0.0f;
    node.traverse([&](glm::mat4 model_matrix, Node const& node) {
        for (auto const& mesh : node.meshes) {
            auto const world_aabb_min = model_matrix * glm::vec4{mesh.aabb.min, 1.0f};
            auto const world_aabb_max = model_matrix * glm::vec4{mesh.aabb.max, 1.0f};
            radius = std::max(radius, std::abs(world_center.x - world_aabb_min.x));
            radius = std::max(radius, std::abs(world_center.y - world_aabb_min.y));
            radius = std::max(radius, std::abs(world_center.z - world_aabb_min.z));
            radius = std::max(radius, std::abs(world_center.x - world_aabb_max.x));
            radius = std::max(radius, std::abs(world_center.y - world_aabb_max.y));
            radius = std::max(radius, std::abs(world_center.z - world_aabb_max.z));
        }
    });

    if (radius < 0.1f) {
        return;
    }

    auto const distance = radius * 1.5f;

    auto const look_direction = glm::normalize(camera->target - camera->position);

    switch (type) {
    case Type::FREECAM:
    case Type::UNITY: {
        auto const target_position = world_center - look_direction * distance;
        animate_to(target_position, target_position + look_direction, duration);
        break;
    }
    case Type::BLENDER: {
    }
        animate_to(world_center - look_direction * distance, world_center, duration);
    }
}

void CameraController::update_animation(float delta_time)
{
    if (!m_animation.active) {
        return;
    }

    auto factor = delta_time / m_animation.remaining_time;
    m_animation.remaining_time -= delta_time;
    if (factor >= 1.0f) {
        factor = 1.0f;
        m_animation.active = false;
    }

    camera->target = factor * m_animation.target + (1 - factor) * camera->target;
    camera->position = factor * m_animation.position + (1 - factor) * camera->position;
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

void CameraController::update_freecam(float delta_time, bool)
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
void CameraController::update_blender(float delta_time, bool handle_scroll)
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
    if (handle_scroll && scroll_y != 0.0) {
        auto zoom_step = zoom_speed * std::sqrt(glm::length(forward)) * -scroll_y;
        auto min_distance = 1.0f;
        auto camera_distance = std::abs(glm::distance(camera->position, camera->target));
        if (camera_distance + zoom_step < min_distance) {
            zoom_step = camera_distance - min_distance;
        }
        camera->position += forward_normalized * glm::vec3{zoom_step};
    }
}

void CameraController::update_unity(float delta_time, bool handle_scroll)
{
    // As viewed by the target torwards the camera
    auto forward = glm::normalize(camera->target - camera->position);
    auto right = glm::normalize(glm::cross(forward, camera->up));

    // Movement
    if (Input::button_pressed(GLFW_MOUSE_BUTTON_RIGHT)) {
        auto movement_scalar = movement_speed * delta_time;

        if (Input::key_pressed(GLFW_KEY_LEFT_SHIFT)) {
            movement_scalar *= 4;
        }

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
    }

    // Panning
    if (Input::button_pressed(GLFW_MOUSE_BUTTON_MIDDLE)) {
        auto panning_scalar = movement_speed * delta_time;

        auto screen_up = glm::cross(forward, right);
        camera->position -= screen_up * glm::vec3(Input::cursor_delta().y * panning_scalar);
        camera->position -= right * glm::vec3(Input::cursor_delta().x * panning_scalar);
    }

    // Rotation
    auto [yaw, pitch] = direction_to_yaw_pitch(forward);

    if (Input::button_pressed(GLFW_MOUSE_BUTTON_RIGHT)) {
        auto look_scalar = rotation_speed * delta_time;
        pitch -= Input::cursor_delta().y * look_scalar;
        yaw += Input::cursor_delta().x * look_scalar;

        auto const max_pitch = glm::half_pi<float>() - 0.001f;
        pitch = std::clamp(pitch, -max_pitch, max_pitch);
    }

    // Zoom
    auto scroll_y = static_cast<float>(Input::scroll_delta().y);
    if (handle_scroll && scroll_y != 0.0) {
        // FIXME: I have no clue how zooming works in Unity
        camera->position += forward * scroll_y * zoom_speed * 20.0f;
    }

    camera->target = camera->position + yaw_pitch_to_direction(yaw, pitch);
}
