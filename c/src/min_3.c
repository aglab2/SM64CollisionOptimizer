#include "PR/ultratypes.h"

s16 min_3(s16 a0, s16 a1, s16 a2) {
    if (a1 < a0) {
        a0 = a1;
    }

    if (a2 < a0) {
        a0 = a2;
    }

    return a0;
}
