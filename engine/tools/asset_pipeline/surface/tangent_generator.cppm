export module tools.asset_pipeline.surface.tangent_generator;

import core.stdtypes;
import tools.asset_pipeline.types.mesh_types;

export namespace draco::tools::assets
{
    class TangentGenerator
    {
    public:
        static void generate(ImportedMesh& mesh);
    };
}