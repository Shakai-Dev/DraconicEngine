module;

#include <cstddef>

module tools.asset_pipeline.mesh.mesh_cooker;

import tools.asset_pipeline.types.mesh_types;
import tools.asset_pipeline.meshlets.types;
import tools.asset_pipeline.importer;
import tools.asset_pipeline.mesh.pipeline;
import tools.asset_pipeline.formats.dmesh_format;

namespace draco::tools::assets
{
    AssetResult MeshCooker::cook(const ImportedMesh& source, DMeshPayload& out)
    {
        const auto pipeline = MeshPipeline::run(source);

        const ImportedMesh& mesh        = pipeline.mesh;
        const MeshletMesh& meshlets     = pipeline.meshlets;

        out.header.magic[0] = 'D';
        out.header.magic[1] = 'M';
        out.header.magic[2] = 'S';
        out.header.magic[3] = 'H';

        out.header.version = DMeshVersion;

        out.header.vertex_count = static_cast<u32>(mesh.vertices.size());
        out.header.index_count  = static_cast<u32>(mesh.indices.size());
        out.header.submesh_count = static_cast<u32>(mesh.submeshes.size());

        out.header.meshlet_count        = static_cast<u32>(meshlets.meshlets.size());
        out.header.meshlet_vertex_count = static_cast<u32>(meshlets.vertices.size());
        out.header.meshlet_index_count  = static_cast<u32>(meshlets.indices.size());

        out.vertices.resize(mesh.vertices.size());

        for (size_t i = 0; i < mesh.vertices.size(); ++i)
        {
            const auto& v = mesh.vertices[i];

            out.vertices[i] =
            {
                v.px, v.py, v.pz,
                v.nx, v.ny, v.nz,
                v.tx, v.ty, v.tz, v.tw,
                v.u,  v.v
            };
        }

        out.indices = mesh.indices;

        out.submeshes.resize(mesh.submeshes.size());

        for (size_t i = 0; i < mesh.submeshes.size(); ++i)
        {
            const auto& s = mesh.submeshes[i];

            out.submeshes[i] =
            {
                s.index_start,
                s.index_count,
                s.material_index
            };
        }

        // Meshlet metadata
        out.meshlets.resize(meshlets.meshlets.size());

        for (size_t i = 0; i < meshlets.meshlets.size(); ++i)
        {
            const auto& m = meshlets.meshlets[i];

            out.meshlets[i] =
            {
                m.vertex_offset,
                m.vertex_count,
                m.index_offset,
                m.triangle_count * 3 // Convert triangle count to index count since we don't store it
            };
        }

        // Meshlet vertex stream (remapped vertex indices)
        out.meshlet_vertices = meshlets.vertices;

        // Meshlet index stream (8-bit local triangle indices)
        out.meshlet_indices = meshlets.indices;

        return AssetResult::Success;
    }
}
