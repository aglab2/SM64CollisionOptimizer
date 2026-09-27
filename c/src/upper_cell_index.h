#include "config_world.h"
#include "PR/ultratypes.h"

#include "math_ex.h"

static inline __attribute__((always_inline)) s32 upper_cell_index(s32 coord) {
    // Move from range [-LEVEL_BOUNDARY_MAX, LEVEL_BOUNDARY_MAX) to [0, 2 * LEVEL_BOUNDARY_MAX)
    coord += LEVEL_BOUNDARY_MAX;
    if (coord < 0) {
        coord = 0;
    }

    // [0, NUM_CELLS)
    s32 index = coord / CELL_SIZE;

    // Potentially < 0, but since lower index is >= 0, not exploitable
    return MIN((NUM_CELLS - 1), index);
}
