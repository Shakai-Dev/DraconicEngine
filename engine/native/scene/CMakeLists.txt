add_modules_library(camera)

target_sources(camera
    PUBLIC
        FILE_SET CXX_MODULES FILES
            camera/camera_controller.cppm

    PRIVATE
        camera/camera_controller.cpp
)

target_link_libraries(camera
    PUBLIC
        input
        renderer
        bx
)

add_modules_library(transform)

target_sources(transform
    PUBLIC
        FILE_SET CXX_MODULES FILES
            transform/transform.cppm

    PRIVATE
        transform/transform.cpp
)

target_link_libraries(transform
    PUBLIC
        bx
)

add_modules_library(renderable)

target_sources(renderable
    PUBLIC
        FILE_SET CXX_MODULES FILES
            renderable/renderable.cppm
)

target_link_libraries(renderable
    PUBLIC
        mesh
        transform
        material
)

add_library(scene)

target_sources(scene
    PUBLIC
        FILE_SET CXX_MODULES FILES
            scene.cppm
)

target_link_libraries(scene
    PUBLIC
        renderable
)