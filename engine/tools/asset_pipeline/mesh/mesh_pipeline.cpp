module;

#include <vector>
#include <utility>

module tools.asset_pipeline.mesh.pipeline;

import core.stdtypes;
import tools.asset_pipeline.geometry.mesh_sanitizer;
import tools.asset_pipeline.surface.tangent_generator;
import tools.asset_pipeline.optimize.mesh_optimizer;
import tools.asset_pipeline.optimize.adjacency_builder;
import tools.asset_pipeline.meshlets.builder;

namespace draco::tools::assets
{
    PipelineOutput MeshPipeline::run(const ImportedMesh& source)
    {
        auto clean = MeshSanitizer::sanitize(source);

        ImportedMesh mesh = clean.mesh;

        TangentGenerator::generate(mesh);

        MeshOptimizer::optimize(mesh);

        std::vector<u32> adjacency;
        build_adjacency(mesh, adjacency);

        MeshletMesh meshlets = MeshletBuilder::build(mesh);

        return
        {
            std::move(mesh),
            std::move(meshlets)
        };
    }
}