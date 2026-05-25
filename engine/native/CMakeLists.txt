add_modules_library(core SHARED)
add_modules_library(input)
add_modules_library(scene)

add_subdirectory(thirdparty SYSTEM)
add_subdirectory(platform)

target_link_libraries(core PUBLIC definitions math io memory)
target_link_libraries(input PRIVATE SDL3::SDL3-static core)
target_link_libraries(scene PUBLIC renderable)

add_subdirectory(rendering)
