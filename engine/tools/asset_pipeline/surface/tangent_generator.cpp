module;

#include <mikktspace.h>

module tools.asset_pipeline.surface.tangent_generator;

import core.stdtypes;
import tools.asset_pipeline.types.mesh_types;

namespace draco::tools::assets
{
    static void generate_tangents_mikk(ImportedMesh& mesh);

    void TangentGenerator::generate(ImportedMesh& mesh)
    {
        generate_tangents_mikk(mesh);
    }

    static void generate_tangents_mikk(ImportedMesh& mesh)
    {
        struct UserData { ImportedMesh* mesh; } ud{ &mesh };

        SMikkTSpaceInterface iface{};

        iface.m_getNumFaces = [](const SMikkTSpaceContext* ctx) -> int {
            return (int)static_cast<UserData*>(ctx->m_pUserData)->mesh->indices.size() / 3;
        };

        iface.m_getNumVerticesOfFace = [](const SMikkTSpaceContext*, const int) -> int {
            return 3;
        };

        iface.m_getPosition = [](const SMikkTSpaceContext* ctx, float out[], int face, int vert)
        {
            auto* m = static_cast<UserData*>(ctx->m_pUserData)->mesh;
            u32 i = m->indices[face * 3 + vert];
            auto& v = m->vertices[i];
            out[0]=v.px; out[1]=v.py; out[2]=v.pz;
        };

        iface.m_getNormal = [](const SMikkTSpaceContext* ctx, float out[], int face, int vert)
        {
            auto* m = static_cast<UserData*>(ctx->m_pUserData)->mesh;
            u32 i = m->indices[face * 3 + vert];
            auto& v = m->vertices[i];
            out[0]=v.nx; out[1]=v.ny; out[2]=v.nz;
        };

        iface.m_getTexCoord = [](const SMikkTSpaceContext* ctx, float out[], int face, int vert)
        {
            auto* m = static_cast<UserData*>(ctx->m_pUserData)->mesh;
            u32 i = m->indices[face * 3 + vert];
            auto& v = m->vertices[i];
            out[0]=v.u; out[1]=v.v;
        };

        iface.m_setTSpaceBasic = [](const SMikkTSpaceContext* ctx, const float t[], float s, int face, int vert)
        {
            auto* m = static_cast<UserData*>(ctx->m_pUserData)->mesh;
            u32 i = m->indices[face * 3 + vert];
            auto& v = m->vertices[i];

            v.tx=t[0]; v.ty=t[1]; v.tz=t[2]; v.tw=s;
        };

        SMikkTSpaceContext ctx{ &iface, &ud };
        genTangSpaceDefault(&ctx);
    }
}