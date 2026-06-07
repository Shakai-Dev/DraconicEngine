module;

#include <vector>
#include <meshoptimizer.h>

module tools.asset_pipeline.optimize.mesh_optimizer;

import core.stdtypes;
import tools.asset_pipeline.types.mesh_types;

namespace draco::tools::assets
{
    void MeshOptimizer::optimize_indices(std::vector<u32>& indices, u32 vertex_count)
    {
        if (indices.empty()) return;

        meshopt_optimizeVertexCache(indices.data(), indices.data(), indices.size(), vertex_count);
    }

    void MeshOptimizer::optimize(ImportedMesh& mesh)
    {
        optimize_indices(mesh.indices, (u32)mesh.vertices.size());
    }
}