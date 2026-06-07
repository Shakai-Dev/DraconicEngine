add_modules_library(rendering)
add_modules_library(scene)

target_link_libraries(rendering PUBLIC rhi rendergraph render_mesh material quad_renderer renderer dmesh)
target_link_libraries(scene PUBLIC renderable transform_component camera)
