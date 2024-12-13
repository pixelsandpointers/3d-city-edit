#include "Editor.hpp"

Editor::Editor(int width, int height)
    : m_window_width(width)
    , m_window_height(height)
    , m_running(false)
{
}

bool Editor::init()
{
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return EXIT_FAILURE;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHintString(GLFW_X11_CLASS_NAME, "3d");
    glfwWindowHintString(GLFW_WAYLAND_APP_ID, "3d");

    m_window = glfwCreateWindow(m_window_width, m_window_height, "3D Editor", nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(m_window);

    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cout << "Failed to initialize GLAD\n";
        return EXIT_FAILURE;
    }

    // setup imgui
    setup_imgui();

    // setup callbacks
    glfwSetWindowUserPointer(m_window, this);

    // setup uber-camera
    m_uber_camera = std::make_unique<Camera>(glm::vec3(0.f, 0.f, -3.f));
    m_uber_camera_controller = std::make_unique<CameraController>(*m_uber_camera, m_event_dispatcher);

    glfwSetScrollCallback(m_window, [](GLFWwindow* window, double xoffset, double yoffset) {
        if (auto* editor = static_cast<Editor*>(glfwGetWindowUserPointer(window))) {
            Event event{EventType::MOUSE_SCROLL};
            event.mouse_scroll_event.xoffset = xoffset;
            event.mouse_scroll_event.yoffset = yoffset;
            event.time_delta = editor->m_time_delta;
            editor->m_event_dispatcher.dispatch(event);
        }
    });

    glfwSetKeyCallback(m_window,[](GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (auto editor = static_cast<Editor*>(glfwGetWindowUserPointer(window))) {
            Event event{EventType::KEY_PRESS};
            event.key_event.key_code = key;
            event.time_delta = editor->m_time_delta;
            editor->m_event_dispatcher.dispatch(event);

            // close editor
            if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
                glfwSetWindowShouldClose(window, GL_TRUE);
        }
    });

    return true;
}
void Editor::run()
{
    render();
}
void Editor::shutdown() const
{
    glfwDestroyWindow(m_window);
    glfwTerminate();
}
void Editor::setup_imgui() const
{
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(m_window, true); // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();
}
void Editor::render()
{
    // additional setup
    stbi_set_flip_vertically_on_load(true);
    auto path = std::filesystem::current_path();
    std::cout << path.c_str() << std::endl;
    path.append("assets/objects/backpack/backpack.obj");

    Model obj{path.string()};
    Shader shader{"src/shader/obj.vert", "src/shader/obj.frag"};

    float last_frame = 0.0f;

    while (!glfwWindowShouldClose(m_window)) {
        float current_frame = glfwGetTime();
        m_time_delta = current_frame - last_frame;
        last_frame = current_frame;

        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);

        shader.use();

        // view/projection transformations
        glm::mat4 projection = m_uber_camera->get_projection_matrix(m_window_width / m_window_height, 0.1f, 100.f);
        glm::mat4 view = m_uber_camera->get_view_matrix();
        shader.set_mat4("projection", projection);
        shader.set_mat4("view", view);

        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, -20.0f)); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f)); // it's a bit too big for our scene, so scale it down
        shader.set_mat4("model", model);
        obj.draw(shader);

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::ShowDemoWindow(); // Show demo window! :)

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        /* Swap front and back buffers */
        glfwSwapBuffers(m_window);

        /* Poll for and process events */
        glfwPollEvents();
    }
}