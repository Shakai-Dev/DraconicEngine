module;

#include <vector>

export module tools.asset_pipeline.optimize.mesh_optimizer;

import core.stdtypes;
import tools.asset_pipeline.types.mesh_types;

export namespace draco::tools::assets
{
    class MeshOptimizer
    {
    public:
        static void optimize(ImportedMesh& mesh);

    private:
        static void optimize_indices(std::vector<u32>& indices, u32 vertex_count);
    };
}