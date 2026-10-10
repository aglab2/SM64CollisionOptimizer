#include "cfg.h"

const struct CollisionConfig read_vertex_data /*gCollisionConfig*/ = {
    .wallkickAngle = 0x2000,
    .numQuarterSteps = 4,
    .normalFloorCeilThreshold = 0.05f,
    .wallkickAngleExt = 0x4000,
    .impreciseCollision = 1,
};
