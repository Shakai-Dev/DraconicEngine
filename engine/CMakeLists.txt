add_subdirectory(thirdparty SYSTEM)

add_modules_library(platform)
add_modules_library(core SHARED)
add_modules_library(input)
add_modules_library(tools)
add_modules_library(runtime)

target_link_libraries(platform PUBLIC platform_impl)
target_link_libraries(core PUBLIC definitions math io memory)
target_link_libraries(input PRIVATE SDL3::SDL3-static platform core)
target_link_libraries(tools PUBLIC core asset_pipeline)
target_link_libraries(runtime PUBLIC platform core input scene rendering tools)