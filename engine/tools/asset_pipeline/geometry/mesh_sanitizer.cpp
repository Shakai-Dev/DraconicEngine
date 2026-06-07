module;

#include <vector>
#include <unordered_map>
#include <algorithm>

module tools.asset_pipeline.geometry.mesh_sanitizer;

import core.stdtypes;
import tools.asset_pipeline.types.mesh_types;

namespace draco::tools::assets
{
    void MeshSanitizer::deduplicate_global(ImportedMesh& mesh)
    {
        std::unordered_map<ImportedVertex, u32, ImportedVertexHasher, ImportedVertexEqual> cache;
        std::vector<ImportedVertex> new_vertices;
        std::vector<u32> new_indices;

        for (u32 idx : mesh.indices)
        {
            const auto& v = mesh.vertices[idx];

            auto it = cache.find(v);
            if (it == cache.end())
            {
                u32 new_index = (u32)new_vertices.size();
                new_vertices.push_back(v);
                cache[v] = new_index;
                new_indices.push_back(new_index);
            }
            else
            {
                new_indices.push_back(it->second);
            }
        }

        mesh.vertices = std::move(new_vertices);
        mesh.indices  = std::move(new_indices);
    }

    void MeshSanitizer::fix_indices(ImportedMesh& mesh)
    {
        mesh.indices.erase(std::remove_if(mesh.indices.begin(), mesh.indices.end(), [&](u32 i) { return i >= mesh.vertices.size(); }), mesh.indices.end());
    }

    void MeshSanitizer::normalize_submeshes(ImportedMesh& mesh)
    {
        for (auto& s : mesh.submeshes)
        {
            if (s.index_start + s.index_count > mesh.indices.size())
                s.index_count = mesh.indices.size() - s.index_start;
        }
    }

    void MeshSanitizer::validate_vertices(ImportedMesh&)
    {
        // TODO: Ensure no NaNs, Infs, etc
    }

    void MeshSanitizer::validate_material_slots(ImportedMesh&)
    {
        // TODO: Ensure indices exist in material_slots
    }

    CleanMesh MeshSanitizer::sanitize(const ImportedMesh& input)
    {
        ImportedMesh mesh = input;

        validate_vertices(mesh);
        fix_indices(mesh);
        deduplicate_global(mesh);
        normalize_submeshes(mesh);
        validate_material_slots(mesh);

        return { mesh };
    }
}