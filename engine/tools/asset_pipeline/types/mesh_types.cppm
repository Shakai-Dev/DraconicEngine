module;

#include <string>
#include <vector>
#include <functional>
#include <cmath>

export module tools.asset_pipeline.types.mesh_types;

import core.stdtypes;

export namespace draco::tools::assets
{
    struct ImportedVertex
    {
        f32 px, py, pz;
        f32 nx, ny, nz;

        // tangent + handedness
        f32 tx, ty, tz, tw;

        f32 u, v;

        bool operator==(const ImportedVertex& o) const noexcept
        {
            return px == o.px && py == o.py && pz == o.pz && nx == o.nx && ny == o.ny && nz == o.nz && tx == o.tx && ty == o.ty && tz == o.tz && tw == o.tw && u == o.u && v == o.v;
        }
    };

    struct ImportedVertexHasher
    {
        size_t operator()(const ImportedVertex &v) const noexcept
        {
            size_t h = 0;

            auto hash_combine = [&](auto val)
            {
                h ^= std::hash<decltype(val)>{}(val) + 0x9e3779b9 + (h << 6) + (h >> 2);
            };

            hash_combine(v.px);
            hash_combine(v.py);
            hash_combine(v.pz);

            hash_combine(v.nx);
            hash_combine(v.ny);
            hash_combine(v.nz);

            hash_combine(v.tx);
            hash_combine(v.ty);
            hash_combine(v.tz);
            hash_combine(v.tw);

            hash_combine(v.u);
            hash_combine(v.v);

            return h;
        }
    };

    struct ImportedVertexEqual
    {
        static bool feq(f32 a, f32 b, f32 epsilon = 1e-6f) 
        {
            return std::fabs(a - b) < epsilon;
        }

        bool operator()(const ImportedVertex& a, const ImportedVertex& b) const noexcept
        {
            return feq(a.px, b.px) && feq(a.py, b.py) && feq(a.pz, b.pz) && feq(a.nx, b.nx) && feq(a.ny, b.ny) && feq(a.nz, b.nz) && feq(a.tx, b.tx) && feq(a.ty, b.ty) && feq(a.tz, b.tz) && feq(a.tw, b.tw) && feq(a.u, b.u) && feq(a.v, b.v);
        }
    };

    struct ImportedSubmesh
    {
        std::string name;
        u32 index_start = 0;
        u32 index_count = 0;
        u32 material_index = 0;
    };

    struct ImportedMesh
    {
        std::vector<ImportedVertex> vertices;
        std::vector<u32>            indices;
        std::vector<ImportedSubmesh> submeshes;
        std::vector<std::string>     material_slots;
    };
}
