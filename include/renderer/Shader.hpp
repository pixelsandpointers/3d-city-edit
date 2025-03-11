#pragma once

#include <functional>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

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

struct UniformLocations {
    // Vertex
    int model{-1};
    int view{-1};
    int projection{-1};

    // Fragment
    int texture_diffuse{-1};
    int texture_opacity{-1};
    int light_direction{-1};
    int light_color{-1};
    int light_power{-1};
    int camera_pos{-1};
    int ambient_strength{-1};
    int specularity_factor{-1};
    int shininess{-1};
    int gamma{-1};

    int tex{-1};
    int color{-1};
    int id{-1};
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
    std::function<void(UniformLocations&, std::function<void(int&, char const*)>)> uniform_caching_function;
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

    static Shader lighting;
    static Shader albedo;
    static Shader post_process_outline;
    static Shader picking;
    static Shader const& get_shader_for_mode(ViewingMode);
    static void init();

    Shader() = default;
    Shader(ShaderSource);
    Shader(Shader const&) = delete;
    Shader(Shader&&);
    Shader& operator=(Shader&&);

    unsigned int m_id;
    UniformLocations uniform_locations;
    void use() const;

    template <typename T>
    void set_uniform(int location, T const& value) const
    {
        if (location < 0) {
            return;
        }

        if constexpr (std::is_same_v<T, bool>) {
            glUniform1i(location, static_cast<int>(value));
        } else if constexpr (std::is_same_v<T, int>) {
            glUniform1i(location, value);
        } else if constexpr (std::is_same_v<T, unsigned int>) {
            glUniform1ui(location, value);
        } else if constexpr (std::is_same_v<T, float>) {
            glUniform1f(location, value);
        } else if constexpr (std::is_same_v<T, glm::mat4>) {
            glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
        } else if constexpr (std::is_same_v<T, glm::vec3>) {
            glUniform3fv(location, 1, glm::value_ptr(value));
        } else if constexpr (std::is_same_v<T, glm::vec4>) {
            glUniform4fv(location, 1, glm::value_ptr(value));
        }
    }

private:
    void check_compile_errors(unsigned int shader, ShadingStage stage) const;
    unsigned int compile_shader(ShadingStage stage, char const* source) const;
    void link_shaders_to_program(unsigned vertex_shader, unsigned fragment_shader);
};
