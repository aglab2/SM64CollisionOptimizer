#include "sm64.h"
#include "engine/surface_collision.h"

// Horizontal dot product of surface normal
#define hdot_surf(surf, vec) (((surf)->normal.x * (vec)[0]) + ((surf)->normal.z * (vec)[2]))

struct Surface *check_ledge_grab(struct MarioState *m, struct Surface *prevWall, struct Surface *wall, Vec3f intendedPos, Vec3f nextPos, Vec3f ledgePos, struct Surface **ledgeFloor) {
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
