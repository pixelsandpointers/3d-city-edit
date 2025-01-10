#include "renderer/Shader.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_map>

/**
 * @brief Collection of shader sources for different shading techniques.
 *
 * The `shader_sources` is a constant array containing the GLSL vertex and fragment shader
 * source codes for various shading techniques. Each entry in the array corresponds to a
 * particular `ViewingMode` and provides a pair of strings: the vertex shader source and
 * the matching fragment shader source. These shaders define the behavior of the rendering
 * pipeline when a specific shading technique is used.
 *
 * @details The supported shading techniques and corresponding shader implementations include:
 *   - `ViewingMode::ALBEDO_SHADING`: Implements basic texture mapping, computing
 *     the final fragment color by sampling a diffuse texture applied to the geometry.
 *     No lighting calculations are performed.
 *   - `ViewingMode::FLAT_SHADING`: Provides flat shading by computing face normals,
 *     resulting in a uniform color for each surface, and applies basic lighting models
 *     including diffuse and ambient components. The normals are not interpolated across
 *     the surface.
 *   - `ViewingMode::BLINN_PHONG_SHADING`: Extends the Phong shading technique by incorporating
 *     the more efficient Blinn-Phong specular reflection model. This approach adjusts specular
 *     highlights while maintaining visual quality.
 *
 * This array facilitates the selection, usage, and management of GPU shader programs
 * within a rendering engine. The shaders are written in GLSL, version 4.10, and are designed
 * to operate in OpenGL rendering pipelines.
 */
std::unordered_map<ViewingMode, ShaderSource> const shader_sources{
    {ViewingMode::ALBEDO,
        ShaderSource{
            .vertex_shader = R"(
            #version 410 core
            layout (location = 0) in vec3 aPos;
            layout (location = 1) in vec3 aNormal;
            layout (location = 2) in vec2 aTexCoords;

            out vec2 TexCoords;

            uniform mat4 model;
            uniform mat4 view;
            uniform mat4 projection;

            void main() {
                TexCoords = aTexCoords;
                gl_Position = projection * view * model * vec4(aPos, 1.0);
            })",

            .fragment_shader = R"(
            #version 410 core
            out vec4 FragColor;
            in vec2 TexCoords;
            uniform sampler2D texture_diffuse1;
            void main() {
                FragColor = texture(texture_diffuse1, TexCoords);
            })"}},
    {
        ViewingMode::SOLID,
        ShaderSource{
            .vertex_shader = R"(
            #version 410 core
            layout (location = 0) in vec3 aPos;       // Vertex position
            layout (location = 1) in vec3 aNormal;    // Vertex normal
            layout (location = 2) in vec2 aTexCoords; // Texture UV coordinates

            out vec3 FlatNormal;                 // Pass the face normal to the fragment shader (not interpolated)
            uniform mat4 model;
            uniform mat4 view;
            uniform mat4 projection;

            void main() {
                // Calculate face normal in world space (flat shading)
                mat3 normalMatrix = transpose(inverse(mat3(model)));
                FlatNormal = normalize(normalMatrix * aNormal);
                // Transform vertex position to clip space
                gl_Position = projection * view * model * vec4(aPos, 1.0);
            })",

            .fragment_shader = R"(
            #version 410 core
            struct Light {
                vec3 direction;
                vec3 color;
            };
            in vec3 FlatNormal;
            uniform Light light; // Direction of the light source (normalized)
            out vec4 FragColor;

            void main() {
                // Calculate grayscale intensity using the Lambertian reflectance model
                float brightness = max(dot(normalize(FlatNormal), normalize(light.direction)), 0.0);
                // Output as grayscale
                FragColor = vec4(vec3(brightness), 1.0);
            })"},
    },
    {
        ViewingMode::RENDERED,
        ShaderSource{
            .vertex_shader = R"(
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
            })",

            .fragment_shader = R"(
            #version 410 core
            struct Light {
                vec3 direction;
                vec3 color;
                float power;
            };

            in vec3 FragPos;
            in vec3 Normal;
            in vec2 TexCoords;

            uniform sampler2D texture_diffuse1;
            uniform Light light;
            uniform vec3 cameraPos;
            uniform float ambientStrength;
            uniform float shininess;
            uniform float gamma;

            out vec4 FragColor;

            void main() {
                vec3 tex = texture(texture_diffuse1, TexCoords).rgb;
                vec3 normal = normalize(Normal);
                float dist = dot(light.direction, light.direction);
                float spec = 0.;
                vec3 lightDir = normalize(-light.direction);
                float lambertian = max(dot(lightDir, normal), 0.);

                // ambient
                vec3 ambient = ambientStrength * tex;

                // diffuse
                float diff = max(dot(lightDir, normal), 0.0);
                vec3 diffuse = diff * tex;

                // specular
                vec3 viewDir = normalize(cameraPos - FragPos);
                vec3 reflectionDir = reflect(-lightDir, normal);
                vec3 halfDir = normalize(lightDir + viewDir);
                spec = pow(max(dot(normal, halfDir), 0.0), shininess/4.);
                vec3 specular = light.color * spec;
                vec3 color = ambient + diffuse * lambertian * light.color * light.power / dist + spec * light.color * light.power / dist;
                vec3 gammaCorrection = pow(color, vec3(1. / gamma));
                FragColor = vec4(gammaCorrection, 1.);
            })",
        },
    },
};

Shader::Shader(ViewingMode mode)
{
    // Extract vertex and fragment shader code
    char const* vertex_code = shader_sources.at(mode).vertex_shader;
    char const* fragment_code = shader_sources.at(mode).fragment_shader;

    // Compile shaders and set up the shader program
    auto const vertex_shader = compile_shader(ShadingStage::VERTEX, vertex_code);
    auto const fragment_shader = compile_shader(ShadingStage::FRAGMENT, fragment_code);

    link_shaders_to_program(vertex_shader, fragment_shader);

    // Cleanup shaders as they are already linked into our program
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

Shader::Shader(char const* vertex_path, char const* fragment_path)
{
    std::string vertex_code;
    std::string fragment_code;
    std::ifstream vertex_shader_file;
    std::ifstream fragment_shader_file;

    vertex_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fragment_shader_file.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        vertex_shader_file.open(vertex_path);
        fragment_shader_file.open(fragment_path);
        std::stringstream vertex_shader_stream, fragment_shader_stream;

        vertex_shader_stream << vertex_shader_file.rdbuf();
        fragment_shader_stream << fragment_shader_file.rdbuf();

        vertex_shader_file.close();
        fragment_shader_file.close();

        vertex_code = vertex_shader_stream.str();
        fragment_code = fragment_shader_stream.str();
    } catch (std::ifstream::failure e) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESS\n";
    }

    char const* vertex_source = vertex_code.c_str();
    char const* fragment_source = fragment_code.c_str();

    // Compile shaders and set up the shader program
    auto const vertex_shader = compile_shader(ShadingStage::VERTEX, vertex_source);
    auto const fragment_shader = compile_shader(ShadingStage::FRAGMENT, fragment_source);

    link_shaders_to_program(vertex_shader, fragment_shader);

    // Cleanup shaders as they are already linked into our program
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

void Shader::use() const
{
    glUseProgram(m_id);
}

void Shader::set_bool(char const* name, bool value) const
{
    glUniform1i(glGetUniformLocation(m_id, name), static_cast<int>(value));
}

void Shader::set_int(char const* name, int value) const
{
    glUniform1i(glGetUniformLocation(m_id, name), value);
}

void Shader::set_float(char const* name, float value) const
{
    glUniform1f(glGetUniformLocation(m_id, name), value);
}

void Shader::set_mat2(char const* name, glm::mat2 const& matrix) const
{
    glUniformMatrix2fv(glGetUniformLocation(m_id, name), 1, GL_FALSE, glm::value_ptr(matrix));
}

void Shader::set_mat3(char const* name, glm::mat3 const& matrix) const
{
    glUniformMatrix3fv(glGetUniformLocation(m_id, name), 1, GL_FALSE, glm::value_ptr(matrix));
}

void Shader::set_mat4(char const* name, glm::mat4 const& matrix) const
{
    glUniformMatrix4fv(glGetUniformLocation(m_id, name), 1, GL_FALSE, glm::value_ptr(matrix));
}

void Shader::set_vec2(char const* name, glm::vec2 const& vector) const
{
    glUniform2fv(glGetUniformLocation(m_id, name), 1, glm::value_ptr(vector));
}

void Shader::set_vec3(char const* name, glm::vec3 const& vector) const
{
    glUniform3fv(glGetUniformLocation(m_id, name), 1, glm::value_ptr(vector));
}

void Shader::set_vec4(char const* name, glm::vec4 const& vector) const
{
    glUniform4fv(glGetUniformLocation(m_id, name), 1, glm::value_ptr(vector));
}

void Shader::check_compile_errors(unsigned int shader, ShadingStage stage) const
{
    GLint success;
    char log[1024];
    if (stage != ShadingStage::PROGRAM) {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, nullptr, log);
            std::cout << "ERROR::SHADER_COMPILATION_ERRORS of type:\n"
                      << log << "\n ---" << std::endl;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, nullptr, log);
            std::cout << "ERROR::SHADER_LINKING_ERRORS\n"
                      << log << "\n ---" << std::endl;
        }
    }
}

unsigned int Shader::compile_shader(ShadingStage stage, char const* source) const
{
    GLuint type = 0;
    switch (stage) {
    case ShadingStage::VERTEX:
        type = GL_VERTEX_SHADER;
        break;
    case ShadingStage::FRAGMENT:
        type = GL_FRAGMENT_SHADER;
        break;
    default:
        std::cerr << "Invalid shader type.\n";
    }

    auto shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    check_compile_errors(shader, stage);
    return shader;
}

void Shader::link_shaders_to_program(unsigned const vertex_shader, unsigned const fragment_shader)
{
    m_id = glCreateProgram();
    glAttachShader(m_id, vertex_shader);
    glAttachShader(m_id, fragment_shader);
    glLinkProgram(m_id);
    check_compile_errors(m_id, ShadingStage::PROGRAM); // not happy with this -- suggestions are welcome
}
