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
};

void glfw_error_callback([[maybe_unused]] int error, char const* description)
{
    std::cout << "glfw error: " << description << "\n";
}

void glfw_window_size_callback(GLFWwindow*, int width, int height)
{
    framebuffer.width = width;
    framebuffer.height = height;
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
    Shader shader(ShadingType::ALBEDO_SHADING);

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
    // TODO: find suitable light position
    auto light_position = glm::vec4{0.f, 0.f, 1.f, 1.f};

    auto scene_instance = scene.instanciate();
    scene_instance.compute_transforms();

    while (!glfwWindowShouldClose(window)) {
        auto current_frame = glfwGetTime();
        auto delta_time = current_frame - last_frame;
        last_frame = current_frame;

        Input::update();

        camera_controller.update(delta_time);

        /* Render here */
        /*
         * glClear clears the color and depth buffer
         * enable depth testing utilizing a depth buffer
         * depth function is a depth test checking if the current pixel is closer to the camera than
         * the one in the buffer
         * blending enabled for transparency
         * blend function set to mix the color of the object with the existing pixel in the buffer
         * Final_Color = (Source_Color * Source_Alpha) + (Destination_Color * (1 - Source_Alpha));
         */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LEQUAL);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // draw calls from here
        camera_controller.camera->draw(shader, framebuffer, scene_instance);
        shader.use();

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.m_zoom), static_cast<float>(window_width) / static_cast<float>(window_height), 0.1f, 100000.0f);
        glm::mat4 view = camera.get_view_matrix();
        shader.set_mat4("projection", projection);
        shader.set_mat4("view", view);
        shader.set_vec3("lightColor", glm::vec3(1.0f)); // White light

        // render the loaded model
        scene_instance.traverse([&](auto transform_matrix, auto const& node) {
            shader.set_mat4("model", transform_matrix);
            // set light position relative to transformation matrix of the model
            shader.set_vec3("lightPos", glm::vec3{transform_matrix * light_position});

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
