#include "types.h"
#include "math_ex.h"
#include "surface_terrains.h"
#include "engine/surface_collision.h"
#include "engine/surface_load.h"
#include "game/object_list_processor.h"
#include "game/level_update.h"
#include "object_constants.h"
#include "sm64.h"
#include "game/room.h"

#include "config_world.h"

#define o gCurrentObject

#define TerrainData s16

struct Find1Result
{
    struct Surface* surf;
    f32 dx, dz;
    int cornerThresholded;
    int edge;
};

struct Find1Context
{
    struct Find1Result result;
    f32 best;
};

extern void find_wall_collisions_from_list(struct Find1Context* ctx, struct SurfaceNode *surfaceNode, f32 radius, const Vec3f pos, const f32 margin_radius);

#undef max_3
struct Find1Result max_3(const Vec3f pos, f32 radius, const f32 margin_radius) /*find_wall_best*/
{
    f32 x = pos[0];
    f32 z = pos[2];

    struct Find1Context ctx;
    ctx.result.surf = NULL;
    ctx.best = 1000.f;

    if (is_outside_level_bounds(x, z)) {
        return ctx.result;
    }

    s32 minCellX = GET_CELL_COORD(x - radius);
    s32 minCellZ = GET_CELL_COORD(z - radius);
    s32 maxCellX = GET_CELL_COORD(x + radius);
    s32 maxCellZ = GET_CELL_COORD(z + radius);

    for (s32 cellX = minCellX; cellX <= maxCellX; cellX++) {
        for (s32 cellZ = minCellZ; cellZ <= maxCellZ; cellZ++) {
            if (1) {
                // Check for surfaces belonging to objects.
                struct SurfaceNode *node = gDynamicSurfacePartition[cellZ][cellX][SPATIAL_PARTITION_WALLS].next;
                find_wall_collisions_from_list(&ctx, node, radius, pos, margin_radius);
            }

            // Check for surfaces that are a part of level geometry.
            struct SurfaceNode *node = gStaticSurfacePartition[cellZ][cellX][SPATIAL_PARTITION_WALLS].next;
            find_wall_collisions_from_list(&ctx, node, radius, pos, margin_radius);
        }
    }

    return ctx.result;
}
