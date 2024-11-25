#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// Default camera values
constexpr float YAW = -90.0f;
constexpr float PITCH = 0.0f;
constexpr float SPEED = 2.5f;
constexpr float SENSITIVITY = 0.1f;
constexpr float ZOOM = 45.0f;

// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera {
public:
    // camera Attributes
    glm::vec3 m_position;
    glm::vec3 m_front;
    glm::vec3 m_up;
    glm::vec3 m_right;
    glm::vec3 m_world_up;
    // euler Angles
    float m_yaw;
    float m_pitch;
    // camera options
    float m_movement_speed;
    float m_mouse_sensitivity;
    float m_zoom;

    // constructor with vectors
    Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);
    // constructor with scalar values
    Camera(float pos_x, float pos_y, float pos_z, float up_x, float up_y, float up_z, float yaw, float pitch);

    // returns the view matrix calculated using Euler Angles and the LookAt Matrix
    glm::mat4 get_view_matrix() const;

    // processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
    void process_keyboard(Camera_Movement direction, float delta_time);

    // processes input received from a mouse input system. Expects the offset value in both the x and y direction.
    void process_mouse_movement(float xoffset, float yoffset, GLboolean constrain_pitch = true);

    // processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
    void process_mouse_scroll(float yoffset);

private:
    // calculates the front vector from the Camera's (updated) Euler Angles
    void update_camera_vectors();
};

#endif // CAMERA_HPP
