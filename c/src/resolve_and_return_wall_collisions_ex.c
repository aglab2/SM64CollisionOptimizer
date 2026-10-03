#include "sm64.h"
#include "engine/math_util.h"
#include "engine/surface_collision.h"
#include "game/camera.h"
#include "game/mario_step.h"
#include "game/object_helpers.h"

void resolve_and_return_wall_collisions_ex(Vec3f pos, f32 offset, f32 radius, struct WallCollisionData *collisionData) {
    collisionData->x = pos[0];
    collisionData->y = pos[1];
    collisionData->z = pos[2];
    collisionData->radius = radius;
    collisionData->offsetY = offset;

    find_wall_collisions(collisionData);

    pos[0] = collisionData->x;
    pos[1] = collisionData->y;
    pos[2] = collisionData->z;
}
