#include "renderer/Shader.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

Shader::Shader(ShadingType type)
{
    // Find the shader source corresponding to the provided ShadingType
    auto const it = std::ranges::find_if(shader_sources,
        [type](ShaderSource const& source) {
            return source.type == type;
        });

    if (it == std::end(shader_sources)) {
        throw std::invalid_argument("Invalid ShadingType provided.");
    }

    // Extract vertex and fragment shader code
    char const* vertex_code = it->vertex_shader;
    char const* fragment_code = it->fragment_shader;

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
