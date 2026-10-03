
#include "types.h"
#include "math_ex.h"
#include "surface_terrains.h"
#include "engine/surface_load.h"
#include "game/room.h"
#include "config_world.h"

typedef s16 SurfaceType;

void add_ceil_margin(s32 *x, s32 *z, Vec3s target1, Vec3s target2, f32 margin) {
    register f32 diff_x = target1[0] - *x + target2[0] - *x;
    register f32 diff_z = target1[2] - *z + target2[2] - *z;
    register f32 invDenom = margin / __builtin_sqrtf(sqr(diff_x) + sqr(diff_z));

    *x += diff_x * invDenom;
    *z += diff_z * invDenom;
}
