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
    uint8_t ID;

    Shader(char const* vertexPath, char const* fragmentPath);
    void use() const;
    void setBool(std::string const& name, bool value) const;
    void setInt(std::string const& name, int value) const;
    void setFloat(std::string const& name, float value) const;
    void setMat2(std::string const& name, glm::mat2 const& matrix) const;
    void setMat3(std::string const& name, glm::mat3 const& matrix) const;
    void setMat4(std::string const& name, glm::mat4 const& matrix) const;
    void setVec2(std::string const& name, glm::vec2 const& vector) const;
    void setVec3(std::string const& name, glm::vec3 const& vector) const;
    void setVec4(std::string const& name, glm::vec4 const& vector) const;

private:
    void checkCompileErrors(unsigned int shader, std::string type) const;
};

#endif // SHADER_HPP
