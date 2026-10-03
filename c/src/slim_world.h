#include "engine/surface_load.h"

#include "config_world.h"

struct SlimPtr
{
    u16 off;
};

extern struct SurfaceNode *sSurfaceNodePool;
static inline struct SurfaceNode *slim_ptr_read(const struct SlimPtr* sp)
{
    return sp->off ? (sSurfaceNodePool + sp->off) : 0;
}

static inline void slim_ptr_write(struct SlimPtr* sp, struct SurfaceNode* surf)
{
    sp->off = surf ? (surf - sSurfaceNodePool) : 0;
}

typedef struct SlimPtr SlimSpatialPartitionCell[3];

STATIC_ASSERT(NUM_CELLS <= 32, "Too many cells");

extern SlimSpatialPartitionCell gSlimStaticSurfacePartition[NUM_CELLS][NUM_CELLS];
extern SlimSpatialPartitionCell gSlimDynamicSurfacePartition[NUM_CELLS][NUM_CELLS];
