add_modules_library(camera)
add_modules_library(transform_component)
add_modules_library(renderable)

target_link_libraries(camera PUBLIC Core Input Rendering bx)
target_link_libraries(transform_component PUBLIC Core)
target_link_libraries(renderable PUBLIC Core Mesh transform_component Material)
