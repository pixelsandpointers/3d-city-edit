#include "core/AssetManager.hpp"
#include "core/CameraController.hpp"
#include "core/Input.hpp"
#include "renderer/Camera.hpp"
#include "renderer/Shader.hpp"
#include "ui/ObjectDetails.hpp"
#include "ui/ObjectSelectionTree.hpp"
#include "ui/ShaderUniformPane.hpp"

#include <filesystem>
#include <glfw.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

Framebuffer framebuffer{
    .id = 0,
    .width = 1920,
    .height = 1080,
    .aspect = 1920.f / 1080.f};

void glfw_error_callback([[maybe_unused]] int error, char const* description)
{
    std::cout << "glfw error: " << description << "\n";
}

void glfw_window_size_callback(GLFWwindow*, int width, int height)
{
    framebuffer.width = width;
    framebuffer.height = height;
    framebuffer.aspect = width / static_cast<float>(height);
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

    auto* window = glfwCreateWindow(framebuffer.width, framebuffer.height, "3D Street Editor", nullptr, nullptr);
    if (!window) {
        std::cout << "glfwCreateWindow() failed\n";
        glfwTerminate();
        return -1;
    }

    glfwSetWindowSizeCallback(window, glfw_window_size_callback);

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
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

    Input::init(window);

    // assuming we set the CWD to root
    auto path = std::filesystem::current_path();
    std::cout << path.string().c_str() << std::endl;
    path.append("assets/Models/TUD_Innenstadt.FBX");

    // INIT UI
    ObjectDetails object_details_pane{};
    ObjectSelectionTree object_selection_tree{};
    ShaderUniformPane shader_uniform_pane{};
    auto viewing_mode = shader_uniform_pane.viewing_mode;

    auto camera_controller = CameraController{CameraController::Type::FREECAM, glm::vec3{0.f, 0.f, -3.f}};
    auto obj = AssetManager::get_model(path);
    if (!obj) {
        std::abort();
    }

    auto root_transform = Transform{
        .position = glm::vec3{0.0f, -15000.0f, -4000.0f},
        .orientation = glm::vec3{0.0f},
        .scale = glm::vec3{1.0f},
    };

    auto scene = Node::create("scene", root_transform);
    scene.children.push_back(*obj);

    auto scene_instance = scene.instanciate();
    scene_instance.compute_transforms();
    auto last_frame = glfwGetTime();

    // to adjust the shader values, introduce a map here with the uniforms being passed to the draw call,
    // so they can be adjusted in global scope e.g. the GUI
    Shader shader(viewing_mode);

    while (!glfwWindowShouldClose(window)) {
        auto current_frame = glfwGetTime();
        auto delta_time = current_frame - last_frame;
        last_frame = current_frame;

        // Update shader if we change it in the UI
        if (viewing_mode != shader_uniform_pane.viewing_mode) {
            viewing_mode = shader_uniform_pane.viewing_mode;
            shader = Shader(viewing_mode);
        }

        Input::update();

        camera_controller.update(delta_time);

        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        if (shader_uniform_pane.draw_wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        camera_controller.camera->draw(shader, shader_uniform_pane.uniforms, framebuffer, scene_instance);

        // Render GUI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        object_details_pane.render();
        object_selection_tree.render(scene_instance);
        shader_uniform_pane.render();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        Input::late_update();

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
