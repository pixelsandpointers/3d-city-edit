#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.inl>

/**
 * @brief Encapsulates uniforms used in a shader program for rendering.
 *
 * The `Uniforms` struct defines a set of common parameters, such as ambient lighting strength
 * and shading preferences, which can be adjusted for different rendering styles and effects.
 * This configuration helps control the appearance of rendered objects within a scene.
 *
 * @details The uniforms include:
 *   - `ambient_strength`: A floating-point value specifying the intensity of global ambient
 *     lighting applied on objects. This affects the base overall brightness independent of
 *     direct light sources.
 *   - `specularity_factor`: A floating-point value scaling the intensity of specular reflection
 *     on the object. Will be clamped in range [0, 1].
 *   - `shininess`: A floating-point value for the specularity exponent.
 *   - `gamma`: A floating-point value for the gamma correction.
 *
 * Nested inside the struct is the `Light` structure, which represents the properties of a
 * directional light source used in the scene. This includes attributes to define:
 *   - `direction`: A 4-component vector representing the direction of the light in the scene
 *     (typically specified in world space). The fourth component allows flexibility for certain
 *     implementations requiring homogeneous coordinates.
 *   - `color`: A 3-component vector defining the RGB composition of the light's color. Each
 *     component ranges from 0.0 to 1.0 and determines the relative intensity of the light's
 *     red, green, and blue output.
 *   - `power`: A constant floating-point value for the light intensity.
 *
 * This struct is intended to be updated dynamically and passed as uniform data to a GPU shader
 * program to influence the appearance of geometry during rendering.
 */
struct Uniforms {
    float ambient_strength{0.5f};
    float specularity_factor{0.25f};
    float shininess{16.f};
    float gamma{2.2f};
    struct {
        glm::vec4 direction{8.f, 0.f, 0.f, 0.f};
        glm::vec3 color{0.7f, 0.4f, 0.1f};
        float power{0.8f};
    } light;
};

/**
 * @brief Defines the types of shading techniques available in rendering.
 *
 * The `ViewingMode` enumeration categorizes commonly used shading methods in
 * graphical rendering. Each value corresponds to a specific shading algorithm
 * utilized to achieve different visual effects during the rendering process.
 *
 * @details The available shading types include:
 *   - ALBEDO: Only assigns the texture color to the polygon.
 *   - SOLID: A shading technique for grayscale and diffuse factor.
 *   - RENDERED: An extension of the Phong shading model that incorporates
 *     a more computationally efficient specular reflection calculation.
 *
 * This enumeration is commonly employed for selecting and managing the shading
 * pipeline in rendering engines, influencing the appearance of graphical objects.
 */
enum class ViewingMode {
    ALBEDO,
    SOLID,
    RENDERED,
};

/**
 * @brief Represents a shader source associated with a specific shading type.
 *
 * The `ShaderSource` structure encapsulates the shader information required for
 * a shading technique. It associates a shading type with its corresponding shader
 * source code, stored as a pair of vertex and fragment shader string literals.
 *
 * @details The structure provides the following:
 *   - Shader Code: A pair of constant character string pointers, where the first element
 *     represents the vertex shader source and the second represents the fragment shader source.
 *
 * This structure is essential for organizing and using shader programs for various
 * shading implementations in a rendering pipeline.
 */
struct ShaderSource {
    char const* vertex_shader;
    char const* fragment_shader;
};

/**
 * @brief Represents a programmable graphics processing unit (GPU) shader.
 *
 * The `Shader` class encapsulates the functionality of a GPU shader, which is used
 * in the rendering pipeline to perform computations related to vertex processing,
 * fragment generation, or other graphical effects. Shaders are written in GLSL.
 *
 * @details This class provides mechanisms for creating, compiling, linking, and
 * managing shaders. Shaders are categorized into specific stages of the rendering pipeline,
 * such as vertex and fragment shaders. Each stage performs
 * specific tasks, such as transforming vertex data, calculating pixel colors, or performing
 * general-purpose computations on the GPU.
 *
 * Typical functionalities of the Shader class include:
 *   - Loading and compiling shader source code.
 *   - Managing shader programs by linking different shader stages.
 *   - Sending and managing uniform variables to control shader behavior.
 */
class Shader {
public:
    enum class ShadingStage {
        VERTEX,
        FRAGMENT,
        PROGRAM,
    };
    Shader(char const* vertex_path, char const* fragment_path);

    Shader(ViewingMode mode);
    unsigned int m_id;
    void use() const;
    void set_bool(char const* name, bool value) const;
    void set_int(char const* name, int value) const;
    void set_float(char const* name, float value) const;
    void set_mat2(char const* name, glm::mat2 const& matrix) const;
    void set_mat3(char const* name, glm::mat3 const& matrix) const;
    void set_mat4(char const* name, glm::mat4 const& matrix) const;
    void set_vec2(char const* name, glm::vec2 const& vector) const;
    void set_vec3(char const* name, glm::vec3 const& vector) const;
    void set_vec4(char const* name, glm::vec4 const& vector) const;

private:
    void check_compile_errors(unsigned int shader, ShadingStage stage) const;
    unsigned int compile_shader(ShadingStage stage, char const* source) const;
    void link_shaders_to_program(unsigned vertex_shader, unsigned fragment_shader);
};
