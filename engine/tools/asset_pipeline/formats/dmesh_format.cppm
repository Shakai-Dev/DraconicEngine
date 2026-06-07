module;

#include <vector>
#include <filesystem>
#include <fstream>

export module tools.asset_pipeline.formats.dmesh_format;

import core.stdtypes;

export namespace draco::tools::assets
{
    inline constexpr u32 DMeshVersion = 1;

    struct DMeshHeader
    {
        char magic[4] = {'D', 'M', 'S', 'H'};

        u32 version = DMeshVersion;

        u32 vertex_count = 0;
        u32 index_count = 0;
        u32 submesh_count = 0;
        u32 meshlet_count = 0;

        u32 meshlet_vertex_count = 0;
        u32 meshlet_index_count = 0;

        f32 aabb_min[3] = {0.0f, 0.0f, 0.0f};
        f32 aabb_max[3] = {0.0f, 0.0f, 0.0f};

        f32 sphere_center[3] = {0.0f, 0.0f, 0.0f};
        f32 sphere_radius = 0.0f;
    };

    struct DMeshSubmesh
    {
        u32 index_start = 0;
        u32 index_count = 0;
        u32 material_index = 0;
    };

    struct DMeshlet
    {
        u32 vertex_offset;
        u32 vertex_count;

        u32 index_offset;
        u32 triangle_count;

        f32 center[3];
        f32 radius;
    };

    struct CookedVertex
    {
        f32 px, py, pz;

        f32 nx, ny, nz;

        // tangent + handedness
        f32 tx, ty, tz, tw;

        f32 u, v;
    };

    struct DMeshPayload
    {
        DMeshHeader header;

        std::vector<CookedVertex> vertices;
        std::vector<u32> indices;

        std::vector<DMeshSubmesh> submeshes;

        std::vector<DMeshlet> meshlets;

        std::vector<u32> meshlet_vertices;
        std::vector<u8> meshlet_indices;
    };

    [[nodiscard]] 
    inline bool serialize_dmesh( const std::filesystem::path& path, const DMeshPayload& payload)
    {
        std::ofstream file(path, std::ios::binary | std::ios::trunc);

        if (!file.is_open())
            return false;

        file.write(reinterpret_cast<const char*>(&payload.header), sizeof(DMeshHeader));

        if (!payload.vertices.empty())
        {
            file.write(reinterpret_cast<const char*>(payload.vertices.data()), static_cast<std::streamsize>(payload.vertices.size() * sizeof(CookedVertex)));
        }

        if (!payload.indices.empty())
        {
            file.write(reinterpret_cast<const char*>(payload.indices.data()), static_cast<std::streamsize>(payload.indices.size() * sizeof(u32)));
        }

        if (!payload.submeshes.empty())
        {
            file.write(reinterpret_cast<const char*>(payload.submeshes.data()), static_cast<std::streamsize>(payload.submeshes.size() * sizeof(DMeshSubmesh)));
        }

        if (!payload.meshlets.empty())
        {
            file.write(reinterpret_cast<const char *>(payload.meshlets.data()), static_cast<std::streamsize>(payload.meshlets.size() * sizeof(DMeshlet)));
        }

        if (!payload.meshlet_vertices.empty())
        {
            file.write(reinterpret_cast<const char *>(payload.meshlet_vertices.data()), static_cast<std::streamsize>(payload.meshlet_vertices.size() * sizeof(u32)));
        }

        if (!payload.meshlet_indices.empty())
        {
            file.write(reinterpret_cast<const char *>(payload.meshlet_indices.data()), static_cast<std::streamsize>(payload.meshlet_indices.size() * sizeof(u8)));
        }

        return file.good();
    }

    [[nodiscard]]
    inline bool deserialize_dmesh(const std::filesystem::path& path, DMeshPayload& out_payload)
    {
        std::ifstream file(path, std::ios::binary);

        if (!file.is_open())
            return false;

        file.read(reinterpret_cast<char*>(&out_payload.header), sizeof(DMeshHeader));

        if (!file.good())
            return false;

        const auto& header = out_payload.header;

        if (header.magic[0] != 'D' || header.magic[1] != 'M' || header.magic[2] != 'S' || header.magic[3] != 'H')
        {
            return false;
        }

        if (header.version != DMeshVersion)
            return false;

        out_payload.vertices.resize(header.vertex_count);

        if (header.vertex_count > 0)
        {
            file.read(reinterpret_cast<char*>(out_payload.vertices.data()), static_cast<std::streamsize>(header.vertex_count * sizeof(CookedVertex)));
        }

        out_payload.indices.resize(header.index_count);

        if (header.index_count > 0)
        {
            file.read(reinterpret_cast<char*>(out_payload.indices.data()), static_cast<std::streamsize>(header.index_count * sizeof(u32)));
        }
  
        out_payload.submeshes.resize(header.submesh_count);

        if (header.submesh_count > 0)
        {
            file.read(reinterpret_cast<char*>(out_payload.submeshes.data()), static_cast<std::streamsize>(header.submesh_count * sizeof(DMeshSubmesh)));
        }

        out_payload.meshlets.resize(header.meshlet_count);

        if (header.meshlet_count > 0)
        {
            file.read(reinterpret_cast<char *>(out_payload.meshlets.data()), static_cast<std::streamsize>(header.meshlet_count * sizeof(DMeshlet)));
        }

        out_payload.meshlet_vertices.resize(header.meshlet_vertex_count);

        if (header.meshlet_vertex_count > 0)
        {
            file.read(reinterpret_cast<char *>(out_payload.meshlet_vertices.data()), static_cast<std::streamsize>(header.meshlet_vertex_count * sizeof(u32)));
        }

        out_payload.meshlet_indices.resize(header.meshlet_index_count);

        if (header.meshlet_index_count > 0)
        {
            file.read(reinterpret_cast<char *>(out_payload.meshlet_indices.data()), static_cast<std::streamsize>(header.meshlet_index_count * sizeof(u8)));
        }

        return file.good();
    }
}
