#include "types.h"

typedef struct {
    s16 wallkickAngle;            // 45 degrees
    s16 numQuarterSteps;          // 4
    f32 normalFloorCeilThreshold; // 0.01                                                                                    
} CollisionConfig;

extern CollisionConfig gCollisionConfig;

