#include "engine/surface_load.h"

#include "config_world.h"

void _start(SpatialPartitionCell *cells) {
    register s32 i = NUM_CELLS * NUM_CELLS;

    while (i--) {
        (*cells)[SPATIAL_PARTITION_FLOORS].next = NULL;
        (*cells)[SPATIAL_PARTITION_CEILS].next = NULL;
        (*cells)[SPATIAL_PARTITION_WALLS].next = NULL;

        cells++;
    }
}
