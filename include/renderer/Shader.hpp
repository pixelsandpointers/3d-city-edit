#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.inl>
#include <string>

/**
 * @brief Defines the types of shading techniques available in rendering.
 *
 * The `ShadingType` enumeration categorizes commonly used shading methods in
 * graphical rendering. Each value corresponds to a specific shading algorithm
 * utilized to achieve different visual effects during the rendering process.
 *
 * @details The available shading types include:
 *   - FLAT_SHADING: A shading technique where a single color is applied per polygon.
 *   - PHONG_SHADING: A method where colors are interpolated based on vertex normals,
 *     providing smooth shading across surfaces.
 *   - BLINN_PHONG_SHADING: An extension of the Phong shading model that incorporates
 *     a more computationally efficient specular reflection calculation.
 *
 * This enumeration is commonly employed for selecting and managing the shading
 * pipeline in rendering engines, influencing the appearance of graphical objects.
 */
enum class ShadingType {
    FLAT_SHADING,
    PHONG_SHADING,
    BLINN_PHONG_SHADING,
};

/**
 * @brief Represents a shader source associated with a specific shading type.
 *
 * The `ShaderSource` structure encapsulates the shader information required for
 * a shading technique. It associates a shading type with its corresponding shader
 * source code, stored as a pair of vertex and fragment shader string literals.
 *
 * @details The structure provides the following:
 *   - ShadingType: An enumeration representing the type of shading (e.g., Flat, Phong, Blinn-Phong).
 *   - Shader Code: A pair of constant character string pointers, where the first element
 *     represents the vertex shader source and the second represents the fragment shader source.
 *
 * This structure is essential for organizing and using shader programs for various
 * shading implementations in a rendering pipeline.
 */
struct ShaderSource {
    ShadingType type;
    std::pair<char const*, char const*> shader_code;
};

/**
 * @brief Defines an array of shader source data for various shading techniques.
 *
 * The `shader_sources` array contains shader source code for different shading types,
 * including Flat Shading, Phong Shading, and Blinn-Phong Shading. Each shading type is
 * represented as a pair of vertex and fragment shader source strings, encapsulated
 * along with its shading type identifier.
 *
 * @details The available shading types and their corresponding shader source codes:
 *   - Flat Shading:
 *     - Vertex Shader: Transforms vertex positions using the model, view, and
 *       projection matrices.
 *     - Fragment Shader: Outputs a uniform color provided as an input.
 *   - Phong Shading:
 *     - Vertex Shader: Computes world-space positions, normals, and light positions
 *       for the fragment shader.
 *     - Fragment Shader: Implements the Phong shading model, combining diffuse and
 *       specular lighting contributions.
 *   - Blinn-Phong Shading:
 *     - Vertex Shader: Similar to Phong Shading, computes world-space positions,
 *       normals, and light positions.
 *     - Fragment Shader: Implements the Blinn-Phong shading model, combining
 *       diffuse and Blinn-Phong specular lighting contributions.
 *
 * Each shading type utilizes GLSL version 4.10 Shader Language.
 */
constexpr ShaderSource shader_sources[] = {
    {ShadingType::FLAT_SHADING,
        std::make_pair(
            R"(
            #version 410 core
            layout (location = 0) in vec3 aPos;
            uniform mat4 model;
            uniform mat4 view;
            uniform mat4 projection;
            void main() {
                gl_Position = projection * view * model * vec4(aPos, 1.0);
            }
        )",
            R"(
            #version 410 core
            out vec4 FragColor;
            uniform vec3 color;
            void main() {
                FragColor = vec4(color, 1.0);
            }
        )")},
    {ShadingType::PHONG_SHADING,
        std::make_pair(
            R"(
        #version 410 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        uniform vec3 lightPos;

        out vec3 FragPos;
        out vec3 Normal;
        out vec3 LightPos;

        void main() {
            FragPos = vec3(model * vec4(aPos, 1.0));
            Normal = mat3(transpose(inverse(model))) * aNormal;
            LightPos = lightPos;
            gl_Position = projection * view * vec4(FragPos, 1.0);
        }
        )",
            R"(
        #version 410 core
        in vec3 FragPos;
        in vec3 Normal;
        in vec3 LightPos;

        uniform vec3 lightColor;
        uniform vec3 viewPos;
        uniform vec3 objectColor;

        out vec4 FragColor;

        void main() {
            vec3 norm = normalize(Normal);
            vec3 lightDir = normalize(LightPos - FragPos);

            // Diffuse shading
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = diff * lightColor;

            // Specular shading
            vec3 viewDir = normalize(viewPos - FragPos);
            vec3 reflectDir = reflect(-lightDir, norm);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
            vec3 specular = lightColor * spec;

            // Combine
            vec3 result = (diffuse + specular) * objectColor;
            FragColor = vec4(result, 1.0);
        }
        )")},
    {ShadingType::BLINN_PHONG_SHADING,
        std::make_pair(
            R"(
        #version 410 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        uniform vec3 lightPos;

        out vec3 FragPos;
        out vec3 Normal;
        out vec3 LightPos;

        void main() {
            FragPos = vec3(model * vec4(aPos, 1.0));
            Normal = mat3(transpose(inverse(model))) * aNormal;
            LightPos = lightPos;
            gl_Position = projection * view * vec4(FragPos, 1.0);
        }
        )",
            R"(
        #version 410 core
        in vec3 FragPos;
        in vec3 Normal;
        in vec3 LightPos;

        uniform vec3 lightColor;
        uniform vec3 viewPos;
        uniform vec3 objectColor;

        out vec4 FragColor;

        void main() {
            vec3 norm = normalize(Normal);
            vec3 lightDir = normalize(LightPos - FragPos);

            // Diffuse shading
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = diff * lightColor;

            // Blinn-Phong specular shading
            vec3 viewDir = normalize(viewPos - FragPos);
            vec3 halfwayDir = normalize(lightDir + viewDir);
            float spec = pow(max(dot(norm, halfwayDir), 0.0), 32.0);
            vec3 specular = lightColor * spec;

            // Combine
            vec3 result = (diffuse + specular) * objectColor;
            FragColor = vec4(result, 1.0);
        }
        )")}};

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
    Shader(ShadingType type);
    Shader(char const* vertex_path, char const* fragment_path);
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
    void check_compile_errors(unsigned int shader, std::string type) const;
};
