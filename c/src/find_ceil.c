/**
 * Find the lowest ceiling above a given position and return the height.
 */
f32 find_ceil(f32 posX, f32 posY, f32 posZ, struct Surface **pceil) {
    f32 height        = CELL_HEIGHT_LIMIT;
    f32 dynamicHeight = CELL_HEIGHT_LIMIT;
    s32 x = posX;
    s32 y = posY;
    s32 z = posZ;
    *pceil = NULL;

    if (is_outside_level_bounds(x, z)) {
        return height;
    }

    // Each level is split into cells to limit load, find the appropriate cell.
    s32 cellX = GET_CELL_COORD(x);
    s32 cellZ = GET_CELL_COORD(z);

    struct SurfaceNode *surfaceList;
    struct Surface *ceil = NULL;
    struct Surface *dynamicCeil = NULL;

    s32 includeDynamic = 1;

    if (includeDynamic) {
        // Check for surfaces belonging to objects.
        surfaceList = gDynamicSurfacePartition[cellZ][cellX][SPATIAL_PARTITION_CEILS];
        dynamicCeil = find_ceil_from_list(surfaceList, x, y, z, &dynamicHeight);

        // In the next check, only check for ceilings lower than the previous check.
        height = dynamicHeight;
    }

    // Check for surfaces that are a part of level geometry.
    surfaceList = gStaticSurfacePartition[cellZ][cellX][SPATIAL_PARTITION_CEILS];
    ceil = find_ceil_from_list(surfaceList, x, y, z, &height);

    // Use the lower ceiling.
    if (includeDynamic && height >= dynamicHeight) {
        ceil   = dynamicCeil;
        height = dynamicHeight;
    }

    // Return the ceiling.
    *pceil = ceil;

    return height;
}
