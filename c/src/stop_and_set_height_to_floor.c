#include "types.h"
#include "engine/math_util.h"
#include "game/mario.h"

void stop_and_set_height_to_floor(struct MarioState *m) {
    struct Object *marioObj = m->marioObj;

    mario_set_forward_vel(m, 0.0f);
    m->vel[1] = 0.0f;

    // HackerSM64 2.1: This check fixes the ledgegrab downwarp after being pushed off a ledge.
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
