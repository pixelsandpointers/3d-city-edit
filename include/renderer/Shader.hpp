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
 *   - ALBEDO_SHADING: Only assigns the texture color to the polygon.
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
    ALBEDO_SHADING,
    FLAT_SHADING,
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
    char const* vertex_shader;
    char const* fragment_shader;
};

/**
 * @brief Collection of shader sources for different shading techniques.
 *
 * The `shader_sources` is a constant array containing the GLSL vertex and fragment shader
 * source codes for various shading techniques. Each entry in the array corresponds to a
 * particular `ShadingType` and provides a pair of strings: the vertex shader source and
 * the matching fragment shader source. These shaders define the behavior of the rendering
 * pipeline when a specific shading technique is used.
 *
 * @details The supported shading techniques and corresponding shader implementations include:
 *   - `ShadingType::ALBEDO_SHADING`: Implements basic texture mapping, computing
 *     the final fragment color by sampling a diffuse texture applied to the geometry.
 *     No lighting calculations are performed.
 *   - `ShadingType::FLAT_SHADING`: Provides flat shading by computing face normals,
 *     resulting in a uniform color for each surface, and applies basic lighting models
 *     including diffuse and ambient components. The normals are not interpolated across
 *     the surface.
 *   - `ShadingType::BLINN_PHONG_SHADING`: Extends the Phong shading technique by incorporating
 *     the more efficient Blinn-Phong specular reflection model. This approach adjusts specular
 *     highlights while maintaining visual quality.
 *
 * This array facilitates the selection, usage, and management of GPU shader programs
 * within a rendering engine. The shaders are written in GLSL, version 4.10, and are designed
 * to operate in OpenGL rendering pipelines.
 */
constexpr ShaderSource shader_sources[] = {
    {
        ShadingType::ALBEDO_SHADING,
        R"(
            #version 410 core
            layout (location = 0) in vec3 aPos;
            layout (location = 1) in vec3 aNormal;
            layout (location = 2) in vec2 aTexCoords;

            out vec2 TexCoords;

            uniform mat4 model;
            uniform mat4 view;
            uniform mat4 projection;

            void main()
            {
                TexCoords = aTexCoords;
                gl_Position = projection * view * model * vec4(aPos, 1.0);
            }
        )",
        R"(
        #version 410 core
        out vec4 FragColor;
        in vec2 TexCoords;
        uniform sampler2D texture_diffuse1;
        void main()
        {
            FragColor = texture(texture_diffuse1, TexCoords);
        }
    )",
    },
    {ShadingType::FLAT_SHADING,
        R"(
                #version 410 core
                layout (location = 0) in vec3 aPos;       // Vertex position
                layout (location = 1) in vec3 aNormal;    // Vertex normal
                layout (location = 2) in vec2 aTexCoords; // Texture UV coordinates

                out vec2 TexCoords;              // Pass texture coordinates to the fragment shader
                flat out vec3 FaceNormal;        // Pass the face normal to the fragment shader (not interpolated)

                uniform mat4 model;
                uniform mat4 view;
                uniform mat4 projection;

                void main()
                {
                    // Calculate face normal in world space (flat shading)
                    mat3 normalMatrix = transpose(inverse(mat3(model)));
                    FaceNormal = normalize(normalMatrix * aNormal);

                    // Pass through texture coordinates
                    TexCoords = aTexCoords;

                    // Transform vertex position to clip space
                    gl_Position = projection * view * model * vec4(aPos, 1.0);
                }
            )",
        R"(
        #version 410 core
        struct Light {
            vec3 direction;
            vec3 color;
        };

        in vec2 TexCoords;              // Texture UV coordinates passed from the vertex shader
        flat in vec3 FaceNormal;        // Flat face normal passed from the vertex shader
        out vec4 FragColor;             // Final color output
        
        uniform Light light;                // Directional light
        uniform sampler2D texture_diffuse1; // Diffuse texture
        uniform vec3 view;               // Camera/viewer position in world space
        uniform float ambientStrength;      // Strength of ambient lighting


        void main()
        {
            // Calculate basic lighting
            vec3 lightDir = normalize(-light.direction);        // Assume directional light for simplicity
            vec3 lightColor = light.color;                      // Light color

            // Diffuse
            float diff = max(dot(FaceNormal, lightDir), 0.0);
            vec3 diffuse = diff * light.color;                  // Apply light color to diffuse amount

            // Ambient lighting
            vec3 ambient = ambientStrength * lightColor;

            // Fetch texture color
            vec4 texColor = texture(texture_diffuse1, TexCoords);

            // Combine results: apply lighting to the texture color
            vec3 lighting = (ambient + diffuse) * texColor.rgb;

            // Output final color
            FragColor = vec4(lighting, texColor.a);
}        
)"},
    {ShadingType::BLINN_PHONG_SHADING,
        R"(
        #version 410 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        layout (location = 2) in vec2 aTexCoords;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;

        out vec3 FragPos;
        out vec3 Normal;
        out vec2 TexCoords;

        void main() {
            FragPos = vec3(model * vec4(aPos, 1.0));
            Normal = normalize(mat3(transpose(inverse(model))) * aNormal);
            TexCoords = aTexCoords;

            gl_Position = projection * view * vec4(FragPos, 1.0);
        }
        )",
        R"(
        #version 410 core
        struct Light {
            vec3 direction;
            vec3 color;
        };

        in vec3 FragPos;
        in vec3 Normal;
        in vec2 TexCoords;

        uniform sampler2D texture_diffuse1;
        uniform Light light;
        uniform vec3 cameraPos;
        uniform float ambientStrength;
        uniform bool useBlinn;

        out vec4 FragColor;

        void main() {
            vec3 color = texture(texture_diffuse1, TexCoords).rgb;
            
            // ambient
            vec3 ambient = ambientStrength * color;

            // diffuse
            vec3 lightDir = normalize(-light.direction);
            vec3 normal = normalize(Normal);
            float diff = max(dot(lightDir, normal), 0.0);
            vec3 diffuse = diff * color;

            // specular
            vec3 viewDir = normalize(cameraPos - FragPos);
            vec3 reflectionDir = reflect(-lightDir, normal);
            float spec = 0.f;
            if (useBlinn) {
                vec3 halfDir = normalize(lightDir + viewDir);
                spec = pow(max(dot(normal, halfDir), 0.0), 32.);
            } else {
                vec3 reflectDir = reflect(-lightDir, normal);
                spec = pow(max(dot(viewDir, reflectDir), 0.0), 8.0);
            }
            vec3 specular = light.color * spec;
            FragColor = vec4(ambient + diffuse + specular, 1.0);
        }
        )"}};

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
    Shader(ShadingType type = ShadingType::ALBEDO_SHADING);
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
    void check_compile_errors(unsigned int shader, ShadingStage stage) const;
    unsigned int compile_shader(ShadingStage stage, char const* source) const;
    void link_shaders_to_program(unsigned vertex_shader, unsigned fragment_shader);
};
