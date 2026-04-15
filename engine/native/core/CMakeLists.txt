add_modules_library(definitions)
add_modules_library(math)
add_modules_library(io)

target_link_libraries(math PUBLIC definitions)
target_link_libraries(io PUBLIC definitions stb_image)

add_library(image_loader SHARED)

target_sources(image_loader
    PUBLIC
    FILE_SET CXX_MODULES FILES
        io/image_loader.cppm
)

target_sources(image_loader
    PRIVATE
    io/image_loader.cpp
)

target_link_libraries(image_loader 
    PRIVATE 
    stb_image
)