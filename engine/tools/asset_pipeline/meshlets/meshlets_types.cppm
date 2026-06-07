module;

#include <vector>

export module tools.asset_pipeline.meshlets.types;

import core.stdtypes;

export namespace draco::tools::assets
{
    struct MeshletBounds
    {
        f32 center[3];
        f32 radius;
    };

    struct Meshlet
    {
        u32 vertex_offset;
        u32 vertex_count;

        u32 index_offset;
        u32 triangle_count;

        MeshletBounds bounds;
    };

    struct MeshletMesh
    {
        std::vector<Meshlet> meshlets;

        // Global vertex references
        std::vector<u32> vertices;

        // Local meshlet indices
        std::vector<u8> indices;
    };
}