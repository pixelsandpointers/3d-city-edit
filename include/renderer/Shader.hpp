#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.inl>
#include <string>

class Shader {
public:
    unsigned int m_id;

    Shader(char const* vertex_path, char const* fragment_path);
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
