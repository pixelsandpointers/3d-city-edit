target_sources(3d PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Editor.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/Event.cpp
)

add_subdirectory(core)
add_subdirectory(renderer)
add_subdirectory(controller)