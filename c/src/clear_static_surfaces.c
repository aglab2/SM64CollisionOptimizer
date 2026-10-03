#include "engine/surface_load.h"

#include "slim_world.h"

extern void clear_spatial_partition(SlimSpatialPartitionCell *cells);

void clear_static_surfaces(void) {
    clear_spatial_partition(&gSlimStaticSurfacePartition[0][0]);
}
