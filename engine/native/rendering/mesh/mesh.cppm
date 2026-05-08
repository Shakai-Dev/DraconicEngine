module;

#include <vector>
#include <cstdint>

export module rendering.mesh;

import core.memory;
import rendering.rhi;

export namespace draco::rendering::mesh
{
    using MeshHandle = draco::core::memory::Handle<struct MeshTag>;

    struct Vertex
    {
        float px, py, pz;
        float nx, ny, nz;
        float u, v;
    };

    struct Mesh
    {
        draco::rendering::rhi::BufferHandle vbh;
        draco::rendering::rhi::BufferHandle ibh;

        draco::rendering::rhi::LayoutHandle layout;

        uint32_t vertex_count = 0;
        uint32_t index_count = 0;

        bool valid = false;
    };

    MeshHandle create(
        const void* vertex_data,
        uint32_t vertex_size,
        uint32_t vertex_count,
        const std::vector<uint32_t>& indices,
        draco::rendering::rhi::LayoutHandle layout
    );

    MeshHandle create_cube();
    MeshHandle create_plane(float size);
    MeshHandle create_sphere(int segments, int rings);
    MeshHandle create_cylinder(int segments, float height);
    MeshHandle create_capsule(int segments, int rings, float height);

    void destroy(MeshHandle mesh);
    const Mesh* get(MeshHandle mesh);
}

export namespace draco::rendering::mesh::gen
{
    std::vector<Vertex> cube_vertices();
    std::vector<uint32_t> cube_indices();

    std::vector<Vertex> plane_vertices(float size);
    std::vector<uint32_t> plane_indices();

    std::vector<Vertex> sphere_vertices(int segments, int rings);
    std::vector<uint32_t> sphere_indices(int segments, int rings);

    std::vector<Vertex> cylinder_vertices(int segments, float height);
    std::vector<uint32_t> cylinder_indices(int segments);

    std::vector<Vertex> capsule_vertices(int segments, int rings, float height);
    std::vector<uint32_t> capsule_indices(int segments, int rings);
}
