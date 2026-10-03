#include "sm64.h"
#include "engine/math_util.h"
#include "engine/surface_collision.h"
#include "game/camera.h"
#include "game/mario_step.h"
#include "game/object_helpers.h"

#ifndef SURFACE_YAW
#define SURFACE_YAW(s) atan2s((s)->normal.z, (s)->normal.x)
#endif

extern void resolve_and_return_wall_collisions_ex(Vec3f pos, f32 offset, f32 radius, struct WallCollisionData *collisionData);

static inline f32 find_mario_ceil(Vec3f pos, f32 height, struct Surface **ceil)
{ return find_ceil(pos[0], MAX(height, pos[1]) + 3.0f, pos[2], ceil); }
static inline void set_mario_floor(struct MarioState *m, struct Surface *floor, f32 floorHeight)
{ m->floor = floor; m->floorHeight = floorHeight; }
static inline void set_mario_wall(struct MarioState *m, struct Surface *wall)
{ m->wall = wall; }

s32 perform_ground_quarter_step(struct MarioState *m, Vec3f nextPos) {
    struct WallCollisionData lowerWall, upperWall;
    struct Surface *ceil, *floor;

    s16 i;
    s16 wallDYaw;
    s32 oldWallDYaw;

    resolve_and_return_wall_collisions_ex(nextPos, 30.0f, 24.0f, &lowerWall);
    resolve_and_return_wall_collisions_ex(nextPos, 60.0f, 50.0f, &upperWall);

    f32 floorHeight = find_floor(nextPos[0], nextPos[1], nextPos[2], &floor);
    f32 ceilHeight = find_mario_ceil(nextPos, floorHeight, &ceil);

    f32 waterLevel = find_water_level(nextPos[0], nextPos[2]);

    if (floor == NULL) {
        return GROUND_STEP_HIT_WALL_STOP_QSTEPS;
    }

    if ((m->action & ACT_FLAG_RIDING_SHELL) && floorHeight < waterLevel) {
        floorHeight = waterLevel;
        floor = &gWaterSurfacePseudoFloor;
        floor->originOffset = -floorHeight;
    }

    if (nextPos[1] > floorHeight + 100.0f) {
        if (nextPos[1] + 160.0f >= ceilHeight) {
            return GROUND_STEP_HIT_WALL_STOP_QSTEPS;
        }

        vec3f_copy(m->pos, nextPos);
        set_mario_floor(m, floor, floorHeight);
        return GROUND_STEP_LEFT_GROUND;
    }

    if (floorHeight + 160.0f >= ceilHeight) {
        return GROUND_STEP_HIT_WALL_STOP_QSTEPS;
    }

    vec3f_set(m->pos, nextPos[0], floorHeight, nextPos[2]);

    set_mario_floor(m, floor, floorHeight);

    if (m->wall != NULL) {
        s16 wallYaw = SURFACE_YAW(m->wall);
        oldWallDYaw = abs_angle_diff(wallYaw, m->faceAngle[1]);
    } else {
        oldWallDYaw = 0x0;
    }
    for (i = 0; i < upperWall.numWalls; i++) {
        wallDYaw = abs_angle_diff(SURFACE_YAW(upperWall.walls[i]), m->faceAngle[1]);
        if (wallDYaw > oldWallDYaw) {
            oldWallDYaw = wallDYaw;
            set_mario_wall(m, upperWall.walls[i]);
        }

        if (wallDYaw >= DEGREES(60) && wallDYaw <= DEGREES(120)) {
            continue;
        }

        return GROUND_STEP_HIT_WALL_CONTINUE_QSTEPS;
    }

    return GROUND_STEP_NONE;
}
