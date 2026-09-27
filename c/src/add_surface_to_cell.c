#include "engine/surface_load.h"

#include "config_world.h"

extern s32 gSurfaceNodesAllocated;

static inline struct SurfaceNode *alloc_surface_node(void) {
    struct SurfaceNode *node = &sSurfaceNodePool[gSurfaceNodesAllocated];
    gSurfaceNodesAllocated++;

    node->next = NULL;
    return node;
}

void add_surface_to_cell(s32 dynamic, s32 cellX, s32 cellZ, struct Surface *surface) {
    struct SurfaceNode **list;
    s32 priority;
    s32 sortDir = 1; // highest to lowest, then insertion order (water and floors)
    s32 listIndex;
    s16 sortVal;

    if (surface->normal.y > NORMAL_FLOOR_THRESHOLD) {
        sortVal = surface->upperY;
        listIndex = SPATIAL_PARTITION_FLOORS;
    } else if (surface->normal.y < NORMAL_CEIL_THRESHOLD) {
        sortVal = surface->lowerY;
        listIndex = SPATIAL_PARTITION_CEILS;
        sortDir = -1; // lowest to highest, then insertion order
    } else {
        listIndex = SPATIAL_PARTITION_WALLS;
        sortVal = 0;
        sortDir = 0; // insertion order
    }

    s32 surfacePriority = sortVal * sortDir;

    struct SurfaceNode *newNode = alloc_surface_node();
    newNode->surface = surface;

    if (dynamic) {
        list = &gDynamicSurfacePartition[cellZ][cellX][listIndex].next;
#if notyet
        if (sNumCellsUsed >= sizeof(sCellsUsed) / sizeof(struct CellCoords)) {
            sClearAllCells = TRUE;
        } else {
            if (*list == NULL) {
                sCellsUsed[sNumCellsUsed].z = cellZ;
                sCellsUsed[sNumCellsUsed].x = cellX;
                sCellsUsed[sNumCellsUsed].partition = listIndex;
                sNumCellsUsed++;
            }
        }
#endif
    } else {
        list = &gStaticSurfacePartition[cellZ][cellX][listIndex].next;
    }

    if (*list == NULL) {
        *list = newNode;
        return;
    }

    struct SurfaceNode *curNode = *list;

    // Check if surface should be placed at the beginning of the list.
    priority = curNode->surface->upperY * sortDir;
    if (surfacePriority > priority) {
        *list = newNode;
        newNode->next = curNode;
        return;
    }

    // Loop until we find the appropriate place for the surface in the list.
    while (curNode->next != NULL) {
        priority = curNode->next->surface->upperY * sortDir;

        if (surfacePriority > priority) {
            break;
        }

        curNode = curNode->next;
    }

    newNode->next = curNode->next;
    curNode->next = newNode;
}
