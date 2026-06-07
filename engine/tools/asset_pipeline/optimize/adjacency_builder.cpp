module;

#include <unordered_map>
#include <vector>

module tools.asset_pipeline.optimize.adjacency_builder;

import core.stdtypes;
import tools.asset_pipeline.types.mesh_types;

namespace draco::tools::assets
{
    void build_adjacency(const ImportedMesh& mesh, std::vector<u32>& out_adjacent_indices)
    {
        const u32 index_count = (u32)mesh.indices.size();
        out_adjacent_indices.resize(index_count, ~0u);

        std::unordered_map<EdgeKey, u32, EdgeHasher> edge_map;

        auto add_edge = [&](u32 a, u32 b, u32 tri_index)
        {
            EdgeKey key{ std::min(a,b), std::max(a,b) };

            auto it = edge_map.find(key);
            if (it == edge_map.end())
            {
                edge_map[key] = tri_index;
            }
            else
            {
                // Link triangles across shared edge
                out_adjacent_indices[tri_index] = it->second;
                out_adjacent_indices[it->second] = tri_index;
            }
        };

        for (u32 i = 0; i < index_count; i += 3)
        {
            u32 i0 = mesh.indices[i + 0];
            u32 i1 = mesh.indices[i + 1];
            u32 i2 = mesh.indices[i + 2];

            u32 tri_id = i;

            add_edge(i0, i1, tri_id);
            add_edge(i1, i2, tri_id);
            add_edge(i2, i0, tri_id);
        }
    }
}