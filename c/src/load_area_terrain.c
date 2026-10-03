#include "types.h"

#include "game/object_list_processor.h"
#include "game/macro_special_objects.h"
#include "game/room.h"
#include "engine/surface_load.h"

#include "slim_world.h"

#define TerrainData s16
#define RoomData s8

extern void clear_spatial_partition(SlimSpatialPartitionCell *cells);
extern void load_static_surfaces(TerrainData **data, TerrainData *vertexData, s32 surfaceType, RoomData **surfaceRooms);
extern void load_environmental_regions(s16 **data);

#include "surface_terrains.h"

static inline void clear_static_surfaces(void) {
    clear_spatial_partition(&gSlimStaticSurfacePartition[0][0]);
}

/**
 * Read the data for vertices for reference by triangles.
 */
static inline s16 *read_vertex_data(s16 **data) {
    s32 numVertices;
    UNUSED u8 filler[16];
    s16 *vertexData;

    numVertices = *(*data);
    (*data)++;

    vertexData = *data;
    *data += 3 * numVertices;

    return vertexData;
}

void load_area_terrain(s16 index, s16 *data, s8 *surfaceRooms, s16 *macroObjects) {
    s16 terrainLoadType;
    s16 *vertexData;
    UNUSED u8 filler[4];

    // Initialize the data for this.
    gEnvironmentRegions = NULL;
    gSurfaceNodesAllocated = 1; // 0 is a sentinel
    gSurfacesAllocated = 0;

    clear_static_surfaces();

    // A while loop iterating through each section of the level data. Sections of data
    // are prefixed by a terrain "type." This type is reused for surfaces as the surface
    // type.
    while (TRUE) {
        terrainLoadType = *data;
        data++;

        if (TERRAIN_LOAD_IS_SURFACE_TYPE_LOW(terrainLoadType)) {
            load_static_surfaces(&data, vertexData, terrainLoadType, &surfaceRooms);
        } else if (terrainLoadType == TERRAIN_LOAD_VERTICES) {
            vertexData = read_vertex_data(&data);
        } else if (terrainLoadType == TERRAIN_LOAD_OBJECTS) {
            spawn_special_objects(index, &data);
        } else if (terrainLoadType == TERRAIN_LOAD_ENVIRONMENT) {
            load_environmental_regions(&data);
        } else if (terrainLoadType == TERRAIN_LOAD_CONTINUE) {
            continue;
        } else if (terrainLoadType == TERRAIN_LOAD_END) {
            break;
        } else if (TERRAIN_LOAD_IS_SURFACE_TYPE_HIGH(terrainLoadType)) {
            load_static_surfaces(&data, vertexData, terrainLoadType, &surfaceRooms);
            continue;
        }
    }

    if (macroObjects != NULL && *macroObjects != -1) {
        // If the first macro object presetID is within the range [0, 29].
        // Generally an early spawning method, every object is in BBH (the first level).
        if (0 <= *macroObjects && *macroObjects < 30) {
            spawn_macro_objects_hardcoded(index, macroObjects);
        }
        // A more general version that can spawn more objects.
        else {
            spawn_macro_objects(index, macroObjects);
        }
    }

    gNumStaticSurfaceNodes = gSurfaceNodesAllocated;
    gNumStaticSurfaces = gSurfacesAllocated;
}
