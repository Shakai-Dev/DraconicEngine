module;

#include <cstring>
#include <cmath>

#include <bx/math.h>

export module scene.transform;

import core.stdtypes;

export namespace draco::scene::transform
{
    struct Transform
    {
        f32 position[3] = { 0.0f, 0.0f, 0.0f };
        f32 rotation[3] = { 0.0f, 0.0f, 0.0f }; // Euler (radians)
        f32 scale[3] = { 1.0f, 1.0f, 1.0f };

        bool dirty = true;
    };

    // Creates a default identity transform
    Transform make_transform();

    // Recompute matrix from transform (column-major, bx compatible)
    void compute_matrix(const Transform& t, f32 out[16]);

    // Helpers
    void set_position(Transform& t, f32 x, f32 y, f32 z);
    void set_rotation(Transform& t, f32 x, f32 y, f32 z);
    void set_scale(Transform& t, f32 x, f32 y, f32 z);

    void mark_dirty(Transform& t);
}