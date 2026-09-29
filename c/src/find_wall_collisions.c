#include "types.h"
#include "engine/surface_collision.h"
#include "engine/surface_load.h"
#include "config_world.h"

s32 find_wall_collisions_from_list(struct WallCollisionData *data);

s32 find_wall_collisions(struct WallCollisionData *colData) {
    s32 numCollisions = 0;
    colData->numWalls = 0;

    return find_wall_collisions_from_list(colData);
}