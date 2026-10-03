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

extern void resolve_and_return_wall_collisions_ex(Vec3f pos, f32 offset, f32 radius, struct WallCollisionData *collisionData);

static inline void set_mario_floor(struct MarioState *m, struct Surface *floor, f32 floorHeight)
{ m->floor = floor; m->floorHeight = floorHeight; }
static inline void set_mario_wall(struct MarioState *m, struct Surface *wall)
{ m->wall = wall; }
static inline f32 find_mario_ceil(Vec3f pos, f32 height, struct Surface **ceil)
{ return find_ceil(pos[0], MAX(height, pos[1]) + 3.0f, pos[2], ceil); }

extern s32 bonk_or_hit_lava_wall(struct MarioState *m, struct WallCollisionData *wallData);

#if 1
#define hdot_surf(surf, vec) (((surf)->normal.x * (vec)[0]) + ((surf)->normal.z * (vec)[2]))
static struct Surface *check_ledge_grab(struct MarioState *m, struct Surface *prevWall, struct Surface *wall, Vec3f intendedPos, Vec3f nextPos, Vec3f ledgePos, struct Surface **ledgeFloor) {
    struct Surface *returnedWall = wall;
    if (m->vel[1] > 0.0f || wall == NULL) {
        return NULL;
    }

    if (prevWall == NULL) {
        prevWall = wall;
    }

    // Return the already grabbed wall if Mario is moving into it more than the newly tested wall.
    if (hdot_surf(prevWall, m->vel) < hdot_surf(wall, m->vel)) {
        returnedWall = prevWall;
    }

    // Only ledge grab if the wall displaced Mario in the opposite direction of his velocity.
    // hdot(displacement, vel).
    if (
        ((nextPos[0] - intendedPos[0]) * m->vel[0]) + ((nextPos[2] - intendedPos[2]) * m->vel[2]) > 0.0f
    ) {
        returnedWall = prevWall;
    }

    ledgePos[0] = nextPos[0] - (wall->normal.x * 60.0f);
    ledgePos[2] = nextPos[2] - (wall->normal.z * 60.0f);
    ledgePos[1] = find_floor(ledgePos[0], nextPos[1] + 160.0f, ledgePos[2], ledgeFloor);

    if (ledgeFloor == NULL
        || (*ledgeFloor) == NULL
        || ledgePos[1] < nextPos[1] + 100.0f
#ifdef DONT_LEDGE_GRAB_STEEP_SLOPES
        || (*ledgeFloor)->normal.y < COS25 // H64 TODO: check if floor is actually slippery
#endif
    ) {
        return NULL;
    }

    return returnedWall;
}
#else
extern struct Surface *check_ledge_grab(struct MarioState *m, struct Surface *prevWall, struct Surface *wall, Vec3f intendedPos, Vec3f nextPos, Vec3f ledgePos, struct Surface **ledgeFloor);
#endif

s32 perform_air_quarter_step(struct MarioState *m, Vec3f intendedPos, u32 stepArg) {
    s16 i;
    s32 stepResult = AIR_STEP_NONE;

    Vec3f nextPos, ledgePos;
    struct WallCollisionData upperWall, lowerWall;
    struct Surface *ceil, *floor, *ledgeFloor;
    struct Surface *grabbedWall = NULL;

    vec3f_copy(nextPos, intendedPos);

    resolve_and_return_wall_collisions_ex(nextPos, 150.0f, 50.0f, &upperWall);
    resolve_and_return_wall_collisions_ex(nextPos, 30.0f, 50.0f, &lowerWall);

    f32 floorHeight = find_floor(nextPos[0], nextPos[1], nextPos[2], &floor);
    f32 ceilHeight = find_mario_ceil(nextPos, floorHeight, &ceil);

    f32 waterLevel = find_water_level(nextPos[0], nextPos[2]);

    //! The water pseudo floor is not referenced when your intended qstep is
    // out of bounds, so it won't detect you as landing.

    if (floor == NULL) {
        if (nextPos[1] <= m->floorHeight) {
            m->pos[1] = m->floorHeight;
            return AIR_STEP_LANDED;
        }

        m->pos[1] = nextPos[1];
        return AIR_STEP_HIT_WALL;
    }

    if ((m->action & ACT_FLAG_RIDING_SHELL) && floorHeight < waterLevel) {
        floorHeight = waterLevel;
        floor = &gWaterSurfacePseudoFloor;
        floor->originOffset = -floorHeight;
    }

    //! This check uses f32, but findFloor uses short (overflow jumps)
    if (nextPos[1] <= floorHeight) {
        if (ceilHeight - floorHeight > 160.0f) {
            m->pos[0] = nextPos[0];
            m->pos[2] = nextPos[2];
            set_mario_floor(m, floor, floorHeight);
        }

        //! When ceilHeight - floorHeight <= 160, the step result says that
        // Mario landed, but his movement is cancelled and his referenced floor
        // isn't updated (pedro spots)
        m->pos[1] = floorHeight;
        return AIR_STEP_LANDED;
    }

    if (nextPos[1] + 160.0f > ceilHeight) {
        if (m->vel[1] >= 0.0f) {
            m->vel[1] = 0.0f;

#ifdef HANGING_FIX
            // Grab ceiling unless they just were grabbing a ceiling
            if (!(m->prevAction & ACT_FLAG_HANGING) && ceil != NULL && ceil->type == SURFACE_HANGABLE) {
#else
            if ((stepArg & AIR_STEP_CHECK_HANG) && ceil != NULL && ceil->type == SURFACE_HANGABLE) {
#endif
                return AIR_STEP_GRABBED_CEILING;
            }

            return AIR_STEP_NONE;
        }

        //! Potential subframe downwarp->upwarp?
        if (nextPos[1] <= m->floorHeight) {
            m->pos[1] = m->floorHeight;
            return AIR_STEP_LANDED;
        }

        m->pos[1] = nextPos[1];
        return AIR_STEP_HIT_CEILING;
    }

    //! When the wall is not completely vertical or there is a slight wall
    // misalignment, you can activate these conditions in unexpected situations

    if ((stepArg & AIR_STEP_CHECK_LEDGE_GRAB) && upperWall.numWalls == 0 && lowerWall.numWalls != 0) {
        for (i = 0; i < lowerWall.numWalls; i++) {
            grabbedWall = check_ledge_grab(m, grabbedWall, lowerWall.walls[i], intendedPos, nextPos, ledgePos, &ledgeFloor);
            if (grabbedWall != NULL) {
                stepResult = AIR_STEP_GRABBED_LEDGE;
            }
        }

        if (stepResult == AIR_STEP_GRABBED_LEDGE && grabbedWall != NULL && ledgeFloor != NULL) {
            vec3f_copy(m->pos, ledgePos);
            set_mario_floor(m, floor, ledgePos[1]);
            m->faceAngle[0] = 0x0;
            m->faceAngle[1] = SURFACE_YAW(grabbedWall) + 0x8000;
        } else {
            vec3f_copy(m->pos, nextPos);
            set_mario_floor(m, floor, floorHeight);
        }
        return stepResult;
    }

    vec3f_copy(m->pos, nextPos);
    set_mario_floor(m, floor, floorHeight);

    if (upperWall.numWalls > 0) {
        stepResult  = bonk_or_hit_lava_wall(m, &upperWall);
        if (stepResult != AIR_STEP_NONE) {
            return stepResult;
        }
    }

    return (lowerWall.numWalls > 0) ? bonk_or_hit_lava_wall(m, &lowerWall) : AIR_STEP_NONE;
}
