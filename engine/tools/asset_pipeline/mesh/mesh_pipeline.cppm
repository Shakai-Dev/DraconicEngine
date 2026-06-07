export module tools.asset_pipeline.mesh.pipeline;

import tools.asset_pipeline.types.mesh_types;
import tools.asset_pipeline.meshlets.types;

export namespace draco::tools::assets
{
    struct PipelineOutput
    {
        ImportedMesh mesh;
        MeshletMesh meshlets;
    };

    class MeshPipeline
    {
    public:
        static PipelineOutput run(const ImportedMesh& source);
    };
}