module;

#include <iostream>
#include <filesystem>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <string>
#include <cmath>

#include <tiny_obj_loader.h>

module tools.asset_pipeline.obj_importer;

import core.stdtypes;
import tools.asset_pipeline.importer;
import tools.asset_pipeline.types.mesh_types;

namespace draco::tools::assets
{
    using Vertex = ImportedVertex;

    AssetResult ObjImporter::import_mesh(const std::filesystem::path& path, ImportedMesh& out_mesh)
    {
        if (!std::filesystem::exists(path))
            return AssetResult::FileNotFound;

        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        bool ok = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.string().c_str());

        if (!ok)
            return AssetResult::ParserError;

        // OBJ loaders often put warnings in err too
        if (!warn.empty())
            std::cerr << "[ObjImporter Warning] " << warn << "\n";

        if (!err.empty())
            std::cerr << "[ObjImporter Error] " << err << "\n";

        // reset output
        out_mesh.vertices.clear();
        out_mesh.indices.clear();
        out_mesh.submeshes.clear();
        out_mesh.material_slots.clear();

        std::unordered_map<Vertex, u32, ImportedVertexHasher, ImportedVertexEqual> vertex_cache;

        for (const auto& shape : shapes)
        {
            ImportedSubmesh submesh;
            submesh.name = shape.name;
            submesh.index_start = static_cast<u32>(out_mesh.indices.size());
            submesh.index_count = 0;
            submesh.material_index = 0;

            size_t index_offset = 0;

            for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++)
            {
                int fv = shape.mesh.num_face_vertices[f];
                int mat_id = shape.mesh.material_ids[f];

                // Simple material grouping hook (no splitting yet, cooker will handle it)
                submesh.material_index = static_cast<u32>(mat_id);

                for (int v = 0; v < fv; v++)
                {
                    const tinyobj::index_t& idx =
                        shape.mesh.indices[index_offset + v];

                    Vertex vert{};

                    // Position
                    vert.px = attrib.vertices[3 * idx.vertex_index + 0];
                    vert.py = attrib.vertices[3 * idx.vertex_index + 1];
                    vert.pz = attrib.vertices[3 * idx.vertex_index + 2];

                    // Normal
                    if (idx.normal_index >= 0)
                    {
                        vert.nx = attrib.normals[3 * idx.normal_index + 0];
                        vert.ny = attrib.normals[3 * idx.normal_index + 1];
                        vert.nz = attrib.normals[3 * idx.normal_index + 2];
                    }

                    // UV
                    if (idx.texcoord_index >= 0)
                    {
                        vert.u = attrib.texcoords[2 * idx.texcoord_index + 0];
                        vert.v = attrib.texcoords[2 * idx.texcoord_index + 1];
                    }

                    // Importer doesn't compute tangents
                    vert.tx = 0.0f;
                    vert.ty = 0.0f;
                    vert.tz = 0.0f;
                    vert.tw = 1.0f;

                    // Dedup
                    // It's safe only for position+uv for now
                    u32 index;

                    auto it = vertex_cache.find(vert);
                    if (it == vertex_cache.end())
                    {
                        index = static_cast<u32>(out_mesh.vertices.size());
                        out_mesh.vertices.push_back(vert);
                        vertex_cache.emplace(vert, index);
                    }
                    else
                    {
                        index = it->second;
                    }

                    out_mesh.indices.push_back(index);
                    submesh.index_count++;
                }

                index_offset += fv;
            }

            out_mesh.submeshes.push_back(submesh);
        }

        return AssetResult::Success;
    }
}
