add_modules_library(rhi)

target_sources(rhi
    PUBLIC
        FILE_SET CXX_MODULES FILES
            rhi/vertex.cppm
            rhi/rhi.cppm
            rhi/uniform_registry.cppm
    PRIVATE
        rhi/rhi_bgfx.cpp
)

target_link_libraries(rhi
    PUBLIC
        core
        memory
        bgfx
        bx
)

add_modules_library(rendergraph)

target_sources(rendergraph
    PUBLIC
        FILE_SET CXX_MODULES FILES
            rendergraph/rendergraph.cppm
    PRIVATE
        rendergraph/rendergraph.cpp
)

target_link_libraries(rendergraph
    PUBLIC
        rhi
        bx
)

add_modules_library(renderer)

target_sources(renderer
    PUBLIC
        FILE_SET CXX_MODULES FILES
            renderer/renderer.cppm
    PRIVATE
        renderer/renderer.cpp
)

add_modules_library(mesh)

target_sources(mesh
    PUBLIC
        FILE_SET CXX_MODULES FILES
            mesh/mesh.cppm
    PRIVATE
        mesh/mesh.cpp
)

target_link_libraries(mesh
    PUBLIC
    memory
    rhi
)

add_modules_library(material)

target_sources(material
    PUBLIC
        FILE_SET CXX_MODULES FILES
            material/material.cppm
)

target_link_libraries(material
    PUBLIC
    rhi
)

target_link_libraries(renderer
    PUBLIC
        core
        rhi
        rendergraph
        mesh
        scene
        transform
        bgfx
        bx
)