module;

#include <vector>

module rendering.dmesh.dmesh_runtime;

import rendering.rhi;
import rendering.rhi.vertex;
import rendering.mesh;
import rendering.dmesh.meshlet_buffers;

import tools;

namespace draco::rendering::dmesh
{
    DMeshRuntime DMeshRuntimeLoader::load(const tools::assets::DMeshPayload& payload)
    {
        DMeshRuntime out{};

        rhi::VertexLayoutDesc layout_desc{
            .elements =
                {
                    {rhi::Attrib::Position,  3, rhi::AttribType::Float},
                    {rhi::Attrib::Normal,    3, rhi::AttribType::Float},
                    {rhi::Attrib::TexCoord0, 2, rhi::AttribType::Float}
                }
            };

        out.layout = rhi::create_vertex_layout(layout_desc);

        std::vector<rendering::mesh::Vertex> gpu_vertices;
        gpu_vertices.reserve(payload.vertices.size());

        for (const auto &v : payload.vertices)
        {
            gpu_vertices.push_back({v.px, v.py, v.pz, v.nx, v.ny, v.nz, v.u, v.v});
        }

        out.mesh = rendering::mesh::create(gpu_vertices.data(), static_cast<u32>(gpu_vertices.size() * sizeof(rendering::mesh::Vertex)), static_cast<u32>(gpu_vertices.size()), payload.indices, out.layout);

        out.meshlet_count = static_cast<u32>(payload.meshlets.size());

        if (!payload.meshlets.empty())
        {
            // const u32 meshlet_buffer_size = static_cast<u32>(payload.meshlets.size() * sizeof(tools::assets::DMeshlet));

            // out.meshlets.meshlet_buffer = rhi::create_raw_buffer(payload.meshlets.data(), meshlet_buffer_size);
        }

        if (!payload.meshlet_vertices.empty())
        {
            // const u32 size = static_cast<u32>(payload.meshlet_vertices.size() * sizeof(u32));

            // out.meshlets.meshlet_vertices_buffer = rhi::create_index_buffer(payload.meshlet_vertices.data(), static_cast<u32>(payload.meshlet_vertices.size() * sizeof(u32)));
        }

        if (!payload.meshlet_indices.empty())
        {
            // TODO: Make it u16
            // const u32 size = static_cast<u32>(payload.meshlet_indices.size() * sizeof(u8));

            // out.meshlets.meshlet_indices_buffer = rhi::create_raw_buffer(payload.meshlet_indices.data(), size);
        }

        out.loaded = true;
        return out;
    }

    void DMeshRuntimeLoader::destroy(DMeshRuntime& runtime)
    {
        if (rendering::mesh::get(runtime.mesh))
        {
            rendering::mesh::destroy(runtime.mesh);
        }

        if (runtime.meshlets.meshlet_buffer != rhi::InvalidBuffer)
            rhi::destroy_buffer(runtime.meshlets.meshlet_buffer);

        if (runtime.meshlets.meshlet_vertices_buffer != rhi::InvalidBuffer)
            rhi::destroy_buffer(runtime.meshlets.meshlet_vertices_buffer);

        if (runtime.meshlets.meshlet_indices_buffer != rhi::InvalidBuffer)
            rhi::destroy_buffer(runtime.meshlets.meshlet_indices_buffer);

        runtime = {};
    }
}