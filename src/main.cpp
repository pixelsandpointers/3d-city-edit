#include "core/AssetManager.hpp"
#include "renderer/Camera.hpp"
#include "renderer/Shader.hpp"

// clang-format off
#include <glad/glad.h>
#include <GLFW/glfw3.h>
// clang-format on
#include <filesystem>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

void glfw_error_callback([[maybe_unused]] int error, char const* description)
{
    std::cout << "glfw error: " << description << "\n";
}

int main()
{
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit()) {
        std::cout << "glfwInit() failed\n";
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHintString(GLFW_X11_CLASS_NAME, "3d");
    glfwWindowHintString(GLFW_WAYLAND_APP_ID, "3d");

    constexpr int window_width = 640;
    constexpr int window_height = 480;

    auto* window = glfwCreateWindow(window_width, window_height, "Hello World", nullptr, nullptr);
    if (!window) {
        std::cout << "glfwCreateWindow() failed\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true); // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();

    // assuming we set the CWD to root
    auto path = std::filesystem::current_path();
    std::cout << path.string().c_str() << std::endl;
    path.append("assets/Models/TUD_Innenstadt.FBX");

    Camera camera{glm::vec3(0.f, 0.f, -3.f)};
    Shader shader{"src/shader/obj.vert", "src/shader/obj.frag"};

    auto obj = AssetManager::get_model(path);
    if (!obj) {
        std::abort();
    }

    auto root_transform = Transform{
        .position = glm::vec3{0.0f, -18000.0f, -20000.0f},
        .orientation = glm::vec3{0.0f},
        .scale = glm::vec3{1.0f},
    };
    auto scene = Node::create("scene", root_transform);
    scene.children.push_back(*obj);

    auto scene_instance = scene.instanciate();
    scene_instance.compute_transforms();

    while (!glfwWindowShouldClose(window)) {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        shader.use();

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.m_zoom), static_cast<float>(window_width) / static_cast<float>(window_height), 0.1f, 100000.0f);
        glm::mat4 view = camera.get_view_matrix();
        shader.set_mat4("projection", projection);
        shader.set_mat4("view", view);

        // render the loaded model
        scene_instance.traverse([&](auto transform_matrix, auto const& node) {
            shader.set_mat4("model", transform_matrix);
            for (auto const& mesh : node.meshes) {
                mesh.draw(shader);
            }
        });

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow(); // Show demo window! :)

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}
