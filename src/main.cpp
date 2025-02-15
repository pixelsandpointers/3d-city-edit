#include "core/AsyncTaskQueue.hpp"
#include "core/Input.hpp"
#include "core/Project.hpp"
#include "imgui_internal.h"
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

void setup_dock_builder()
{
    int const dockspace = ImGui::DockSpaceOverViewport(0, ImGui::GetMainViewport());
    static bool first_time = true;

    if (first_time) {
        first_time = false;

        ImGuiID dockspace_left_20, dockspace_right_80, dockspace_right_20, content_main, content_left_top, content_left_bottom, content_right_top, content_right_bottom;

        // splitting the viewport into subnodes: left_20 is a partition into 20% left, 80% right
        ImGui::DockBuilderSplitNode(dockspace, ImGuiDir_Left, 0.2f, &dockspace_left_20, &dockspace_right_80);
        ImGui::DockBuilderSplitNode(dockspace_left_20, ImGuiDir_Up, 0.7f, &content_left_top, &content_left_bottom);
        ImGui::DockBuilderSplitNode(dockspace_right_80, ImGuiDir_Left, 0.75f, &content_main, &dockspace_right_20);
        ImGui::DockBuilderSplitNode(dockspace_right_20, ImGuiDir_Up, 0.2f, &content_right_top, &content_right_bottom);

        // docking components
        ImGui::DockBuilderDockWindow("Asset Browser", content_left_top);
        ImGui::DockBuilderDockWindow("Shading and Lighting Settings", content_left_bottom);
        ImGui::DockBuilderDockWindow("Viewport", content_main);
        ImGui::DockBuilderDockWindow("Object Details", content_right_top);
        ImGui::DockBuilderDockWindow("Object Tree", content_right_bottom);

        ImGui::DockBuilderFinish(dockspace);
    }
}

int main()
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

    // assuming we set the CWD to root
    // TODO: Replace with some kind of "Open Project" dialog
    auto path = std::filesystem::current_path() / "assets";
    std::cout << path.string().c_str() << std::endl;
    Project::load(path);

    auto project = Project::get_current();

    // Instanciate the object if it didn't get loaded
    // TODO: Remove when objects can be added at runtime

    // This is just some temporary workaround
    auto root_transform = Transform{
        .position = glm::vec3{0.0f, -15000.0f, -4000.0f},
        .orientation = glm::vec3{0.0f},
        .scale = glm::vec3{1.0f},
    };

    auto scene = Node::create("scene", root_transform, NodeLocation::empty());

    if (!project->scene.has_value()) {
        auto model_path = path / "Models/TUD_Innenstadt.FBX";
        auto obj = project->get_model(model_path);

        if (!obj) {
            std::abort();
        }

        scene.children.push_back(*obj);

        project->scene = scene.instanciate();
    }

    if (!project->scene.has_value()) {
        std::abort();
    }

    // INIT UI
    ObjectDetails object_details_pane{};
    ObjectSelectionTree object_selection_tree{};
    ShaderUniformPane shader_uniform_pane{};

    auto asset_browser = AssetBrowser{};
    auto viewport_window = Viewport{};

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

        setup_dock_builder(); // create docking layout before components are rendered

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
    project->store();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}
