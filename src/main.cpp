#include "core/AssetManager.hpp"
#include "core/CameraController.hpp"
#include "core/Input.hpp"
#include "renderer/Camera.hpp"
#include "renderer/Shader.hpp"

#include <filesystem>
#include <glfw.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>

Framebuffer framebuffer{
    .id = 0,
    .width = 640,
    .height = 480,
    .aspect = 640.f / 480.f};

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

    auto* window = glfwCreateWindow(framebuffer.width, framebuffer.height, "Hello World", nullptr, nullptr);
    if (!window) {
        std::cout << "glfwCreateWindow() failed\n";
        glfwTerminate();
        return -1;
    }

    glfwSetWindowSizeCallback(window, glfw_window_size_callback);

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

    Input::init(window);

    // assuming we set the CWD to root
    auto path = std::filesystem::current_path();
    std::cout << path.string().c_str() << std::endl;
    path.append("assets/Models/TUD_Innenstadt.FBX");

    auto camera_controller = CameraController{CameraController::Type::ORBIT, glm::vec3{0.f, 0.f, -3.f}};
    auto obj = AssetManager::get_model(path);
    if (!obj) {
        std::abort();
    }
    Shader shader{ShadingType::BLINN_PHONG_SHADING};

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

    while (!glfwWindowShouldClose(window)) {
        auto current_frame = glfwGetTime();
        auto delta_time = current_frame - last_frame;
        last_frame = current_frame;

        Input::update();

        camera_controller.update(delta_time);

        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        camera_controller.camera->draw(shader, framebuffer, scene_instance);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow(); // Show demo window! :)

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