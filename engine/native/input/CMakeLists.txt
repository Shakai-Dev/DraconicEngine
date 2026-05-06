add_library(input)

target_sources(input
    PUBLIC
        FILE_SET CXX_MODULES FILES input.cppm
    PRIVATE
        input.cpp
)

target_link_libraries(input PRIVATE SDL3::SDL3-static)