module;

#include <filesystem>

export module tools.asset_pipeline.obj_importer;

import tools.asset_pipeline.importer;
import tools.asset_pipeline.types.mesh_types;

export namespace draco::tools::assets
{
    class ObjImporter final : public IImporter
    {
    public:
        ObjImporter() = default;
        ~ObjImporter() override = default;

        AssetResult import_mesh(const std::filesystem::path& path, ImportedMesh& out_mesh) override;
    };
}
