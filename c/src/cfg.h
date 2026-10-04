#pragma once

#include "types.h"

struct CollisionConfig {
    s16 wallkickAngle;            // 45 degrees
    s16 numQuarterSteps;          // 4
    f32 normalFloorCeilThreshold; // 0.01                                                                                    
};

extern const struct CollisionConfig gCollisionConfig;
