#include "sm64.h"
#include "engine/math_util.h"
#include "engine/surface_collision.h"
#include "game/camera.h"
#include "game/mario_step.h"
#include "game/object_helpers.h"

#define SURFACE_YAW(s) atan2s((s)->normal.z, (s)->normal.x)
#define WALL_KICK_DEGREES 45
#define MARIO_AIR_HIT_WALL MARIO_UNKNOWN_30
#define AIR_STEP_HIT_CEILING 7

extern struct Surface *resolve_and_return_wall_collisions(Vec3f pos, f32 offset, f32 radius, struct WallCollisionData *collisionData);

static inline void set_mario_wall(struct MarioState *m, struct Surface *wall)
{ m->wall = wall; }
static inline f32 find_mario_ceil(Vec3f pos, f32 height, struct Surface **ceil)
{ return find_ceil(pos[0], MAX(height, pos[1]) + 3.0f, pos[2], ceil); }

s32 read_vertex_data(struct MarioState *m, struct WallCollisionData *wallData) { /*bonk_or_hit_lava_wall*/
    s16 i;
    s16 wallDYaw;
    s32 oldWallDYaw;
    s32 result = AIR_STEP_NONE;

    if (m->wall != NULL) {
        s16 wallYaw = SURFACE_YAW(m->wall);
        oldWallDYaw = abs_angle_diff(wallYaw, m->faceAngle[1]);
    } else {
        oldWallDYaw = 0x0;
    }

    for (i = 0; i < wallData->numWalls; i++) {
        if (wallData->walls[i] != NULL) {
            if (wallData->walls[i]->type == SURFACE_BURNING) {
                set_mario_wall(m, wallData->walls[i]);
                return AIR_STEP_HIT_LAVA_WALL;
            }

            // Update wall reference (bonked wall) only if the new wall has a better facing angle
            wallDYaw = abs_angle_diff(SURFACE_YAW(wallData->walls[i]), m->faceAngle[1]);
            if (wallDYaw > oldWallDYaw) {
                oldWallDYaw = wallDYaw;
                set_mario_wall(m, wallData->walls[i]);

                if (wallDYaw > DEGREES(180 - WALL_KICK_DEGREES)) {
                    m->flags |= MARIO_AIR_HIT_WALL;
                    result = AIR_STEP_HIT_WALL;
                }
            }
        }
    }

    return result;
}
