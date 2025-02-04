#include "core/AsyncTaskQueue.hpp"
#include "core/Input.hpp"
#include "core/Project.hpp"
#include "renderer/Camera.hpp"
#include "renderer/Shader.hpp"
#include "ui/AssetBrowser.hpp"
#include "ui/ObjectDetails.hpp"
#include "ui/ObjectSelectionTree.hpp"
#include "ui/ShaderUniformPane.hpp"
#include "ui/Viewport.hpp"

#include <filesystem>
#include <glfw.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

auto framebuffer = Framebuffer::get_default(1920, 1080);

void glfw_error_callback([[maybe_unused]] int error, char const* description)
{
    std::cout << "glfw error: " << description << "\n";
}

void glfw_window_size_callback(GLFWwindow*, int width, int height)
{
    framebuffer.resize(width, height);
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
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true); // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();

    Input::init(window);
    AsyncTaskQueue::init();

    // assuming we set the CWD to root
    auto path = std::filesystem::current_path() / "assets";
    std::cout << path.string().c_str() << std::endl;
    Project::load(path);
    auto project = Project::get_current();
    path.append("Models/TUD_Innenstadt.FBX");

    // INIT UI
    ObjectDetails object_details_pane{};
    ObjectSelectionTree object_selection_tree{};
    ShaderUniformPane shader_uniform_pane{};

    auto obj = project->get_model(path);

    auto asset_browser = AssetBrowser{};
    auto viewport_window = Viewport{};

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

    project->scene = scene.instanciate();
    project->scene->compute_transforms();
    auto last_frame = glfwGetTime();

    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window)) {
        auto current_frame = glfwGetTime();
        auto delta_time = current_frame - last_frame;
        last_frame = current_frame;

        Input::update();

        project->rebuild_fs_cache_timed(current_frame);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render GUI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());

        object_details_pane.render();
        object_selection_tree.render();
        shader_uniform_pane.render();
        asset_browser.render();
        viewport_window.render(delta_time, shader_uniform_pane);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        Input::late_update();
        AsyncTaskQueue::main.run();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    AsyncTaskQueue::shutdown();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}
