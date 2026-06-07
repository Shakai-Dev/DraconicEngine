export module tools.asset_pipeline.meshlets.builder;

import core.stdtypes;
import tools.asset_pipeline.types.mesh_types;
import tools.asset_pipeline.meshlets.types;

export namespace draco::tools::assets
{
    class MeshletBuilder
    {
    public:
        static MeshletMesh build(const ImportedMesh& mesh);

    private:
        static void build_meshlets(const ImportedMesh& mesh, MeshletMesh& out);
    };
}