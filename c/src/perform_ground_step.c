#include "sm64.h"
#include "engine/math_util.h"
#include "game/mario.h"
#include "game/mario_step.h"
#include "cfg.h"

extern s32 perform_ground_quarter_step(struct MarioState *m, Vec3f nextPos);
static inline void set_mario_wall(struct MarioState *m, struct Surface *wall)
{ m->wall = wall; }

s32 perform_ground_step(struct MarioState *m) {
    s32 i;
    u32 stepResult;
    Vec3f intendedPos;

    set_mario_wall(m, NULL);

    for (i = 0; i < gCollisionConfig.numQuarterSteps; i++) {
        intendedPos[0] = m->pos[0] + m->floor->normal.y * (m->vel[0] / (f32)gCollisionConfig.numQuarterSteps);
        intendedPos[2] = m->pos[2] + m->floor->normal.y * (m->vel[2] / (f32)gCollisionConfig.numQuarterSteps);
        intendedPos[1] = m->pos[1];

        stepResult = perform_ground_quarter_step(m, intendedPos);
        if (stepResult == GROUND_STEP_LEFT_GROUND || stepResult == GROUND_STEP_HIT_WALL_STOP_QSTEPS) {
            break;
        }
    }

    m->terrainSoundAddend = mario_get_terrain_sound_addend(m);
    vec3f_copy(m->marioObj->header.gfx.pos, m->pos);
    vec3s_set(m->marioObj->header.gfx.angle, 0, m->faceAngle[1], 0);

    if (stepResult == GROUND_STEP_HIT_WALL_CONTINUE_QSTEPS) {
        stepResult = GROUND_STEP_HIT_WALL;
    }
    return stepResult;
}
