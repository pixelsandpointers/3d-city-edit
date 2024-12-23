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
    char const* vertex_code = it->shader_code.first;
    char const* fragment_code = it->shader_code.second;

    // Compile shaders and set up the shader program
    unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_code, nullptr);
    glCompileShader(vertex_shader);
    check_compile_errors(vertex_shader, "VERTEX");

    unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_code, nullptr);
    glCompileShader(fragment_shader);
    check_compile_errors(fragment_shader, "FRAGMENT");

    m_id = glCreateProgram();
    glAttachShader(m_id, vertex_shader);
    glAttachShader(m_id, fragment_shader);
    glLinkProgram(m_id);
    check_compile_errors(m_id, "PROGRAM");

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

    GLint vertex, fragment;

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
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESS" << std::endl;
    }

    char const* vertex_source = vertex_code.c_str();
    char const* fragment_source = fragment_code.c_str();

    // compile
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertex_source, NULL);
    glCompileShader(vertex);
    check_compile_errors(vertex, "VERTEX");

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragment_source, NULL);
    glCompileShader(fragment);
    check_compile_errors(fragment, "FRAGMENT");

    m_id = glCreateProgram();
    glAttachShader(m_id, vertex);
    glAttachShader(m_id, fragment);
    glLinkProgram(m_id);
    check_compile_errors(m_id, "PROGRAM");

    glDeleteShader(vertex);
    glDeleteShader(fragment);
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

void Shader::check_compile_errors(unsigned int shader, std::string type) const
{
    GLint success;
    char log[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, log);
            std::cout << "ERROR::SHADER_COMPILATION_ERRORS of type: " << type << "\n"
                      << log << "\n ---" << std::endl;
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, log);
            std::cout << "ERROR::SHADER_LINKING_ERRORS of type: " << type << "\n"
                      << log << "\n ---" << std::endl;
        }
    }
}
