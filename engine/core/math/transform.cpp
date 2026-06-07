module;

#include <cmath>

#include <bx/math.h>

module core.math.transform;

import core.stdtypes;

namespace draco::math
{
    Transform make_transform()
    {
        Transform t;

        t.position[0] = 0.0f;
        t.position[1] = 0.0f;
        t.position[2] = 0.0f;

        t.rotation[0] = 0.0f;
        t.rotation[1] = 0.0f;
        t.rotation[2] = 0.0f;

        t.scale[0] = 1.0f;
        t.scale[1] = 1.0f;
        t.scale[2] = 1.0f;

        return t;
    }

    void set_position(Transform& t, f32 x, f32 y, f32 z)
    {
        t.position[0] = x;
        t.position[1] = y;
        t.position[2] = z;
    }

    void set_rotation(Transform& t, f32 x, f32 y, f32 z)
    {
        t.rotation[0] = x;
        t.rotation[1] = y;
        t.rotation[2] = z;
    }

    void set_scale(Transform& t, f32 x, f32 y, f32 z)
    {
        t.scale[0] = x;
        t.scale[1] = y;
        t.scale[2] = z;
    }

    void compute_matrix(const Transform& t, f32 out[16])
    {
        bx::mtxSRT(out, t.scale[0], t.scale[1], t.scale[2], t.rotation[0], t.rotation[1], t.rotation[2], t.position[0], t.position[1], t.position[2]);
    }
}
