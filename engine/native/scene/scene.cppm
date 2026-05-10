module;

#include <vector>

export module scene;

import scene.renderable;

export namespace draco::scene
{
    struct Scene
    {
        std::vector<renderable::Renderable> renderables;
    };
}