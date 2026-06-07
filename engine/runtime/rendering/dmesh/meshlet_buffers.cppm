export module rendering.dmesh.meshlet_buffers;

import core.stdtypes;
import rendering.rhi;

export namespace draco::rendering::dmesh
{
    struct MeshletBuffers
    {
        rhi::BufferHandle meshlet_buffer;
        rhi::BufferHandle meshlet_vertices_buffer;
        rhi::BufferHandle meshlet_indices_buffer;

        u32 meshlet_count = 0;

        bool valid() const
        {
            return meshlet_buffer != rhi::InvalidBuffer;
        }
    };
}