module;

#include <unordered_map>
#include <vector>

export module tools.asset_pipeline.optimize.adjacency_builder;

import core.stdtypes;
import tools.asset_pipeline.types.mesh_types;

export namespace draco::tools::assets
{
    struct EdgeKey
    {
        u32 a, b;

        bool operator==(const EdgeKey& o) const noexcept
        {
            return a == o.a && b == o.b;
        }
    };

    struct EdgeHasher
    {
        size_t operator()(const EdgeKey& e) const noexcept
        {
            return (size_t)e.a * 73856093 ^ (size_t)e.b * 19349663;
        }
    };

    void build_adjacency(const ImportedMesh& mesh, std::vector<u32>& out_adjacent_indices);
}