#include "types.h"
#include "game/object_list_processor.h"

#include "engine/surface_load.h"

#include "slim_world.h"

extern void clear_spatial_partition(SlimSpatialPartitionCell *cells);

void clear_dynamic_surfaces(void) {
    if (!(gTimeStopState & TIME_STOP_ACTIVE)) {
        gSurfacesAllocated = gNumStaticSurfaces;
        gSurfaceNodesAllocated = gNumStaticSurfaceNodes;

        clear_spatial_partition(&gSlimDynamicSurfacePartition[0][0]);
    }
}
