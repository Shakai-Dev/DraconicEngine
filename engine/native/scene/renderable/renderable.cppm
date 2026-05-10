export module scene.renderable;

import rendering.mesh;
import rendering.material;
import scene.transform;

export namespace draco::scene::renderable
{
    struct Renderable
    {
        draco::rendering::mesh::MeshHandle mesh;

        transform::Transform transform;

        draco::rendering::material::Material material;
    };
}