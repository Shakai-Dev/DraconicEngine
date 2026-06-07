export module tools.asset_pipeline.mesh.mesh_cooker;

import core.stdtypes;

import tools.asset_pipeline.importer;
import tools.asset_pipeline.types.mesh_types;
import tools.asset_pipeline.formats.dmesh_format;
import tools.asset_pipeline.geometry.mesh_sanitizer;
import tools.asset_pipeline.surface.tangent_generator;
import tools.asset_pipeline.optimize.mesh_optimizer;
import tools.asset_pipeline.meshlets.builder;

export namespace draco::tools::assets
{
    class MeshCooker
    {
    public:
        static AssetResult cook(const ImportedMesh& source, DMeshPayload& out_payload);
    };
}