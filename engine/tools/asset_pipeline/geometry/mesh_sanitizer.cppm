export module tools.asset_pipeline.geometry.mesh_sanitizer;

import core.stdtypes;
import tools.asset_pipeline.types.mesh_types;

export namespace draco::tools::assets
{
    struct CleanMesh
    {
        ImportedMesh mesh;
    };

    class MeshSanitizer
    {
    public:
        static CleanMesh sanitize(const ImportedMesh& input);

    private:
        static void validate_vertices(ImportedMesh& mesh);
        static void normalize_submeshes(ImportedMesh& mesh);
        static void fix_indices(ImportedMesh& mesh);
        static void validate_material_slots(ImportedMesh& mesh);
        static void deduplicate_global(ImportedMesh& mesh);
    };
}