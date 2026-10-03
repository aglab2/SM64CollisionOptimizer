#include "engine/surface_load.h"

#include "slim_world.h"

void clear_spatial_partition(SlimSpatialPartitionCell *cells) {
    register s32 i = NUM_CELLS * NUM_CELLS;

    while (i--) {
        (*cells)[SPATIAL_PARTITION_FLOORS] = (struct SlimPtr){};
        (*cells)[SPATIAL_PARTITION_CEILS] = (struct SlimPtr){};
        (*cells)[SPATIAL_PARTITION_WALLS] = (struct SlimPtr){};

        cells++;
    }
}
