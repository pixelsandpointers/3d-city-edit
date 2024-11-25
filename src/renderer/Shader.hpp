#ifndef SHADER_HPP
#define SHADER_HPP

#include <glad/glad.h>
#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.inl"

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

class Shader {
public:
    unsigned int m_id;

    Shader(char const* vertex_path, char const* fragment_path);
    void use() const;
    void set_bool(std::string const& name, bool value) const;
    void set_int(std::string const& name, int value) const;
    void set_float(std::string const& name, float value) const;
    void set_mat2(std::string const& name, glm::mat2 const& matrix) const;
    void set_mat3(std::string const& name, glm::mat3 const& matrix) const;
    void set_mat4(std::string const& name, glm::mat4 const& matrix) const;
    void set_vec2(std::string const& name, glm::vec2 const& vector) const;
    void set_vec3(std::string const& name, glm::vec3 const& vector) const;
    void set_vec4(std::string const& name, glm::vec4 const& vector) const;

private:
    void check_compile_errors(unsigned int shader, std::string type) const;
};

#endif // SHADER_HPP
