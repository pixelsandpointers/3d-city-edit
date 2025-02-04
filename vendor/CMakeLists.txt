find_package(OpenGL REQUIRED)
target_link_libraries(3d OpenGL::GL)

target_include_directories(3d SYSTEM PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(3d PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/glad/glad.c)

include(FetchContent)

find_package(glfw3 CONFIG)
if (NOT glfw3_FOUND)
    FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw
        GIT_TAG 3.4
    )
    FetchContent_MakeAvailable(glfw)
endif()
target_link_libraries(3d glfw)

find_package(glm CONFIG)
if (glm_FOUND)
    target_link_libraries(3d glm::glm)
else()
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm
        GIT_TAG 1.0.1
    )
    FetchContent_MakeAvailable(glm)
    target_link_libraries(3d glm)
endif()

find_package(imgui CONFIG)
if (imgui_FOUND)
    target_link_libraries(3d imgui::imgui)
else()
    FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui
        GIT_TAG v1.91.5-docking
    )
    FetchContent_MakeAvailable(imgui)

    add_library(imgui INTERFACE)
    target_include_directories(imgui INTERFACE "${imgui_SOURCE_DIR}" "${imgui_SOURCE_DIR}/backends")
    target_sources(imgui INTERFACE
        "${imgui_SOURCE_DIR}/imgui.cpp"
        "${imgui_SOURCE_DIR}/imgui_demo.cpp"
        "${imgui_SOURCE_DIR}/imgui_draw.cpp"
        "${imgui_SOURCE_DIR}/imgui_tables.cpp"
        "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
        "${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp"
        "${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp"
    )
    target_link_libraries(3d imgui)
endif()

add_library(stb stb_image.cpp)
target_include_directories(stb PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(3d stb)

find_package(assimp CONFIG)
if (assimp_FOUND)
    target_link_libraries(3d assimp::assimp)
else ()
    FetchContent_Declare(assimp
        GIT_REPOSITORY
        https://github.com/assimp/assimp.git
        GIT_TAG v5.4.3
    )
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(ASSIMP_INJECT_DEBUG_POSTFIX OFF CACHE BOOL "" FORCE)
    set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(assimp)
    target_include_directories(3d INTERFACE libs/assimp-src/include)
    target_link_libraries(3d assimp::assimp)
endif ()
