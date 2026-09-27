#include "types.h"
#include "engine/surface_collision.h"
#include "engine/surface_load.h"
#include "config_world.h"

s32 find_wall_collisions_from_list(struct SurfaceNode *surfaceNode, struct WallCollisionData *data);

s32 find_wall_collisions(struct WallCollisionData *colData) {
    struct SurfaceNode *node;
    s32 numCollisions = 0;
    s32 x = colData->x;
    s32 z = colData->z;

    colData->numWalls = 0;

    if (is_outside_level_bounds(x, z)) {
        return numCollisions;
    }

    // World (level) consists of a 16x16 grid. Find where the collision is on the grid (round toward -inf)
    s32 minCellX = GET_CELL_COORD(x - colData->radius);
    s32 minCellZ = GET_CELL_COORD(z - colData->radius);
    s32 maxCellX = GET_CELL_COORD(x + colData->radius);
    s32 maxCellZ = GET_CELL_COORD(z + colData->radius);

    for (s32 cellX = minCellX; cellX <= maxCellX; cellX++) {
        for (s32 cellZ = minCellZ; cellZ <= maxCellZ; cellZ++) {
            if (1) {
                // Check for surfaces belonging to objects.
                node = gDynamicSurfacePartition[cellZ][cellX][SPATIAL_PARTITION_WALLS].next;
                numCollisions += find_wall_collisions_from_list(node, colData);
            }

            // Check for surfaces that are a part of level geometry.
            node = gStaticSurfacePartition[cellZ][cellX][SPATIAL_PARTITION_WALLS].next;
            numCollisions += find_wall_collisions_from_list(node, colData);
        }
    }

    return numCollisions;
}
