module;

#include <vector>
#include <meshoptimizer.h>

module tools.asset_pipeline.meshlets.builder;

import core.stdtypes;
import tools.asset_pipeline.types.mesh_types;
import tools.asset_pipeline.meshlets.types;

namespace draco::tools::assets
{
    MeshletMesh MeshletBuilder::build(const ImportedMesh& mesh)
    {
        MeshletMesh out;
        build_meshlets(mesh, out);
        return out;
    }

    void MeshletBuilder::build_meshlets(const ImportedMesh& mesh, MeshletMesh& out)
    {
        constexpr u32 max_vertices  = 64;
        constexpr u32 max_triangles = 124;

        const size_t max_meshlets = meshopt_buildMeshletsBound(mesh.indices.size(), max_vertices, max_triangles);

        std::vector<meshopt_Meshlet> meshlets(max_meshlets);

        std::vector<unsigned int> meshlet_vertices(mesh.indices.size());
        std::vector<unsigned char> meshlet_triangles(mesh.indices.size());

        const float* positions = reinterpret_cast<const float*>(mesh.vertices.data());

        const size_t meshlet_count = meshopt_buildMeshlets(meshlets.data(), meshlet_vertices.data(), meshlet_triangles.data(), mesh.indices.data(), mesh.indices.size(), positions, mesh.vertices.size(), sizeof(ImportedVertex), max_vertices, max_triangles, 0.5f);

        out.meshlets.resize(meshlet_count);

        for (size_t i = 0; i < meshlet_count; ++i)
        {
            const meshopt_Meshlet& src = meshlets[i];
            Meshlet& dst = out.meshlets[i];

            dst.vertex_offset = src.vertex_offset;
            dst.vertex_count  = src.vertex_count;

            dst.index_offset  = src.triangle_offset;

            // Note: Meshopt stores triangles (not indices), so *3 is correct
            dst.triangle_count = src.triangle_count;
        }

        // Shrink to actual used data
        out.vertices.assign(meshlet_vertices.begin(), meshlet_vertices.begin() + mesh.indices.size());
        out.indices.assign(meshlet_triangles.begin(), meshlet_triangles.begin() + mesh.indices.size());
    }
}
