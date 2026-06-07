export module rendering.dmesh.dmesh_runtime;

import core.stdtypes;

import rendering.rhi;
import rendering.mesh;
import rendering.dmesh.meshlet_buffers;

import tools;

export namespace draco::rendering::dmesh
{
     // TODO: Reintroduce tangent support once mesh system adopts PBR vertex layout in the future
    
    struct DMeshRuntime
    {
        rendering::mesh::MeshHandle mesh;
        MeshletBuffers meshlets;

        rhi::LayoutHandle layout;

        u32 meshlet_count = 0;
        bool loaded = false;
    };

    class DMeshRuntimeLoader
    {
    public:
        static DMeshRuntime load(const tools::assets::DMeshPayload& payload);
        static void destroy(DMeshRuntime& runtime);
    };
}