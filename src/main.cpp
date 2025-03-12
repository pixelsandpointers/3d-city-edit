#include "core/AsyncTaskQueue.hpp"
#include "core/Input.hpp"
#include "core/Project.hpp"
#include "imgui_internal.h"
#include "renderer/Camera.hpp"
#include "renderer/Shader.hpp"
#include "ui/AssetBrowser.hpp"
#include "ui/ObjectDetails.hpp"
#include "ui/ObjectSelectionTree.hpp"
#include "ui/Performance.hpp"
#include "ui/SettingsPane.hpp"
#include "ui/Viewport.hpp"

#include <ImGuizmo.h>
#include <filesystem>
#include <glfw.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

auto framebuffer = Framebuffer::get_default(1920, 1080);
auto focus_on_scene = false;

void glfw_error_callback([[maybe_unused]] int error, char const* description)
{
    std::cout << "glfw error: " << description << "\n";
}

void glfw_window_size_callback(GLFWwindow*, int width, int height)
{
    framebuffer.resize(width, height);
}

void setup_dock_builder()
{
    auto dockspace = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
    static bool first_time = true;

    if (first_time) {
        first_time = false;

        auto dockspace_left = ImGui::DockBuilderSplitNode(dockspace, ImGuiDir_Left, 0.2f, nullptr, &dockspace);
        auto dockspace_right = ImGui::DockBuilderSplitNode(dockspace, ImGuiDir_Right, 0.25f, nullptr, &dockspace);
        auto dockspace_right_bottom = ImGui::DockBuilderSplitNode(dockspace_right, ImGuiDir_Down, 0.4f, nullptr, &dockspace_right);

        ImGui::DockBuilderDockWindow("Asset Browser", dockspace_left);
        ImGui::DockBuilderDockWindow("Settings", dockspace_left);
        ImGui::DockBuilderDockWindow("Viewport", dockspace);
        ImGui::DockBuilderDockWindow("Object Tree", dockspace_right);
        ImGui::DockBuilderDockWindow("Object Details", dockspace_right_bottom);

        ImGui::DockBuilderFinish(dockspace);
    }
}

int main(int argc, char** argv)
{
    glfwSetErrorCallback(glfw_error_callback);

    if (!glfwInit()) {
        std::cout << "glfwInit() failed\n";
        return -1;
    }
    glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
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

    Shader::init();

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr; // workaround: disable imgui.ini
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true); // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();

    Input::init(window);
    AsyncTaskQueue::init();

    auto input_path = std::filesystem::current_path();

    if (argc == 2) {
        input_path = std::filesystem::path{argv[1]};
        if (input_path.is_relative()) {
            input_path = std::filesystem::canonical(std::filesystem::current_path() / input_path);
        }
    }

    if (std::filesystem::is_directory(input_path)) {
        auto project = Project::load(input_path);
        if (!project->scene) {
            project->scene = std::make_unique<InstancedNode>();
        }
    } else if (std::filesystem::is_regular_file(input_path)) {
        auto project = Project::load(std::filesystem::current_path());
        if (!project->scene) {
            project->scene = std::make_unique<InstancedNode>();
            auto obj = project->get_model(input_path);
            if (obj) {
                project->scene->children.push_back(obj->instantiate());
                focus_on_scene = true;
            }
        }
    }

    // TODO: Add with some kind of "Open Project" dialog

    auto project = Project::get_current();

    if (!project || !project->scene) {
        std::abort();
    }

    // INIT UI
    ObjectDetails object_details_pane{};
    ObjectSelectionTree object_selection_tree{};
    SettingsPane settings_pane{};

    auto asset_browser = AssetBrowser{};
    auto viewport_window = Viewport{};
    auto performance_window = Performance{};

    project->scene->compute_transforms();
    auto last_frame = glfwGetTime();

    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window)) {
        auto current_frame = glfwGetTime();
        auto delta_time = current_frame - last_frame;
        last_frame = current_frame;

        Input::update();

        project->update(current_frame);

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render GUI
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuizmo::BeginFrame();

        setup_dock_builder(); // create docking layout before components are rendered

        object_details_pane.render(viewport_window.camera_controller());
        object_selection_tree.render();
        asset_browser.render();
        settings_pane.render();
        viewport_window.render(delta_time);

        // MSVC sets _DEBUG in debug builds, clang sets NDEBUG in release builds
#if defined(_DEBUG) or not defined(NDEBUG)
        performance_window.render(delta_time);
#endif

        if (focus_on_scene) {
            focus_on_scene = false;
            viewport_window.camera_controller().focus_on(*project->scene);
        }

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
    project->store();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}
