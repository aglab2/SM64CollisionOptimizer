#include "engine/surface_load.h"

extern void clear_spatial_partition(SpatialPartitionCell *cells);

void clear_static_surfaces(void) {
    clear_spatial_partition(&gStaticSurfacePartition[0][0]);
}
