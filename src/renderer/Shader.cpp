#include "renderer/Shader.hpp"

#include <iostream>

/**
 * Implements basic texture mapping, computing
 * the final fragment color by sampling a diffuse texture applied to the geometry.
 * No lighting calculations are performed.
 */
auto const albedo_source = ShaderSource{
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
        in vec4 gl_FragCoord;

        uniform sampler2D texture_diffuse;
        uniform sampler2D texture_opacity;
        uniform float gamma;

        void main() {
            vec3 color = texture(texture_diffuse, TexCoords).rgb;
            vec3 gammaCorrection = pow(color, vec3(1. / gamma));
            FragColor = vec4(gammaCorrection, 1.0f);

            float alpha = texture(texture_opacity, TexCoords).r;
            if (alpha <= 0.01f) {
                discard;
            };
        })",

    .uniform_caching_function = [](UniformLocations& locations, std::function<void(int&, char const*)> cache) {
        cache(locations.model, "model");
        cache(locations.view, "view");
        cache(locations.projection, "projection");

        cache(locations.texture_diffuse, "texture_diffuse");
        cache(locations.texture_opacity, "texture_opacity");
        cache(locations.gamma, "gamma");
    },
};

/*
 * Extends the Phong shading technique by incorporating
 * the more efficient Blinn-Phong specular reflection model. This approach adjusts specular
 * highlights while maintaining visual quality.
 */
auto const lighting_source = ShaderSource{
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
        in vec4 gl_FragCoord;

        uniform sampler2D texture_diffuse;
        uniform sampler2D texture_opacity;
        uniform Light light;
        uniform vec3 cameraPos;
        uniform float ambientStrength;
        uniform float specularityFactor;
        uniform float shininess;
        uniform float gamma;

        out vec4 FragColor;

        void main() {
            vec3 tex = texture(texture_diffuse, TexCoords).rgb;
            vec3 normal = normalize(Normal);
            vec3 lightDir = normalize(-light.direction);

            // ambient
            vec3 ambient = ambientStrength * tex;

            // diffuse
            float diff = max(dot(lightDir, normal), 0.0);
            vec3 diffuse = diff * light.power * tex;

            // specular
            vec3 viewDir = normalize(cameraPos - FragPos);
            vec3 halfDir = normalize(lightDir + viewDir);
            float spec = pow(max(dot(normal, halfDir), 0.0), shininess/4.);
            vec3 specular = spec * specularityFactor * light.power * tex;

            vec3 color = ambient + diffuse * light.color + specular * light.color;

            vec3 gammaCorrection = pow(color, vec3(1. / gamma));
            FragColor = vec4(gammaCorrection, 1.0f);

            float alpha = texture(texture_opacity, TexCoords).r;
            if (alpha <= 0.01f) {
                discard;
            };
        })",

    .uniform_caching_function = [](UniformLocations& locations, std::function<void(int&, char const*)> cache) {
        cache(locations.model, "model");
        cache(locations.view, "view");
        cache(locations.projection, "projection");

        cache(locations.texture_diffuse, "texture_diffuse");
        cache(locations.texture_opacity, "texture_opacity");
        cache(locations.light_direction, "light.direction");
        cache(locations.light_color, "light.color");
        cache(locations.light_power, "light.power");
        cache(locations.camera_pos, "cameraPos");
        cache(locations.ambient_strength, "ambientStrength");
        cache(locations.specularity_factor, "specularityFactor");
        cache(locations.shininess, "shininess");
        cache(locations.gamma, "gamma");
    },
};

// http://geoffprewett.com/blog/software/opengl-outline/
auto const post_process_outline_source = ShaderSource{
    .vertex_shader = R"(
        #version 410 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aTexCoords;

        out vec2 TexCoords;

        void main() {
            TexCoords = aTexCoords;
            gl_Position = vec4(aPos, 0.0f, 1.0f);
        }
    )",

    .fragment_shader = R"(
        #version 410 core
        in vec2 TexCoords;

        uniform sampler2D tex;
        uniform vec3 color;

        out vec4 FragColor;

        #define WIDTH 2

        void main() {
            vec2 texel_size = 1.0f / textureSize(tex, 0).xy;

            float coverage = 0.0f;

            for (int x = -WIDTH; x <= WIDTH; ++x) {
                for (int y = -WIDTH; y <= WIDTH; ++y) {
                    vec2 offset = vec2(x, y) * texel_size;
                    coverage += texture(tex, TexCoords + offset).r;
                }
            }
            coverage /= (WIDTH * 2.0f + 1.0f) * (WIDTH * 2.0f + 1.0f);

            float alpha = coverage >= 0.99f
                ? 0.0f
                : mix(0.0f, 2.0f, coverage);

            FragColor = vec4(color, alpha);
        }
    )",

    .uniform_caching_function = [](UniformLocations& locations, std::function<void(int&, char const*)> cache) {
        cache(locations.tex, "tex");
        cache(locations.color, "color");
        cache(locations.gamma, "gamma");
    },
};

auto const picking_source = ShaderSource{
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
        in vec4 gl_FragCoord;

        uniform uint id;

        void main() {
            FragColor.r = id;
        })",

    .uniform_caching_function = [](UniformLocations& locations, std::function<void(int&, char const*)> cache) {
        cache(locations.model, "model");
        cache(locations.view, "view");
        cache(locations.projection, "projection");

        cache(locations.id, "id");
    },
};

Shader Shader::lighting;
Shader Shader::albedo;
Shader Shader::post_process_outline;
Shader Shader::picking;

void Shader::init()
{
    // Can't init shaders in static variables, because OpenGL is not initialized there.
    Shader::lighting = Shader{lighting_source};
    Shader::albedo = Shader{albedo_source};
    Shader::post_process_outline = Shader{post_process_outline_source};
    Shader::picking = Shader{picking_source};
}

Shader const& Shader::get_shader_for_mode(ViewingMode mode)
{
    Shader const* shader;
    switch (mode) {
    case ViewingMode::ALBEDO:
        shader = &Shader::albedo;
        break;
    case ViewingMode::SOLID:
    case ViewingMode::RENDERED:
        shader = &Shader::lighting;
        break;
    default:
        std::abort();
    };

    assert(shader);

    return *shader;
}

Shader::Shader(ShaderSource source)
{
    // Compile shaders and set up the shader program
    auto const vertex_shader = compile_shader(ShadingStage::VERTEX, source.vertex_shader);
    auto const fragment_shader = compile_shader(ShadingStage::FRAGMENT, source.fragment_shader);

    link_shaders_to_program(vertex_shader, fragment_shader);

    // Cleanup shaders as they are already linked into our program
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    source.uniform_caching_function(uniform_locations, [&](int& location, char const* name) {
        location = glGetUniformLocation(m_id, name);
    });
}

Shader::Shader(Shader&& old)
    : m_id{old.m_id}
    , uniform_locations{old.uniform_locations}
{
    old.m_id = 0;
}

Shader& Shader::operator=(Shader&& other)
{
    if (this != &other) {
        m_id = other.m_id;
        uniform_locations = other.uniform_locations;
        other.m_id = 0;
    }

    return *this;
}

void Shader::use() const
{
    glUseProgram(m_id);
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
