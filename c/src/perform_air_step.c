
#include "sm64.h"
#include "engine/math_util.h"
#include "game/mario.h"
#include "cfg.h"

extern s32 perform_air_quarter_step(struct MarioState *m, Vec3f intendedPos, u32 stepArg);
extern void apply_gravity(struct MarioState *m);
extern void apply_vertical_wind(struct MarioState *m);

static inline void set_mario_wall(struct MarioState *m, struct Surface *wall)
{ m->wall = wall; }

s32 perform_air_step(struct MarioState *m, u32 stepArg) {
    Vec3f intendedPos;
    s32 i;
    s32 quarterStepResult;
    s32 stepResult = AIR_STEP_NONE;

    set_mario_wall(m, NULL);

    for (i = 0; i < gCollisionConfig.numQuarterSteps; i++) {
        intendedPos[0] = m->pos[0] + m->vel[0] / (f32)gCollisionConfig.numQuarterSteps;
        intendedPos[1] = m->pos[1] + m->vel[1] / (f32)gCollisionConfig.numQuarterSteps;
        intendedPos[2] = m->pos[2] + m->vel[2] / (f32)gCollisionConfig.numQuarterSteps;

        quarterStepResult = perform_air_quarter_step(m, intendedPos, stepArg);

        if (quarterStepResult != AIR_STEP_NONE) {
            stepResult = quarterStepResult;
        }

        if (quarterStepResult == AIR_STEP_LANDED || quarterStepResult == AIR_STEP_GRABBED_LEDGE
            || quarterStepResult == AIR_STEP_GRABBED_CEILING
            || quarterStepResult == AIR_STEP_HIT_LAVA_WALL) {
            break;
        }
    }

    if (m->vel[1] >= 0.0f) {
        m->peakHeight = m->pos[1];
    }

    m->terrainSoundAddend = mario_get_terrain_sound_addend(m);

    if (m->action != ACT_FLYING) {
        apply_gravity(m);
    }
    apply_vertical_wind(m);

    vec3f_copy(m->marioObj->header.gfx.pos, m->pos);
    vec3s_set(m->marioObj->header.gfx.angle, 0, m->faceAngle[1], 0);

    return stepResult;
}
