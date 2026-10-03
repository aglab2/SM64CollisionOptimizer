#include "types.h"
#include "sm64.h"
#include "engine/math_util.h"
#include "game/mario.h"
#include "game/mario_step.h"

s32 stationary_ground_step(struct MarioState *m) {
    struct Object *marioObj = m->marioObj;
    u32 stepResult = GROUND_STEP_NONE;

    mario_set_forward_vel(m, 0.0f);

    u32 takeStep = (mario_update_moving_sand(m) | mario_update_windy_ground(m));
    if (takeStep) {
        stepResult = perform_ground_step(m);
    } else {
        // HackerSM64 2.1: This check prevents the downwarps that plagued stationary actions.
#if notyet
        if (m->pos[1] <= m->floorHeight + 160.0f) {
#endif
            m->pos[1] = m->floorHeight;
#if notyet
        }
#endif

        vec3f_copy(marioObj->header.gfx.pos, m->pos);
        vec3s_set(marioObj->header.gfx.angle, 0, m->faceAngle[1], 0);
    }

    return stepResult;
}
