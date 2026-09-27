#include "types.h"

#include "game/room.h"

void load_environmental_regions(s16 **data) {
    s32 numRegions;
    s32 i;

    gEnvironmentRegions = *data;
    numRegions = *(*data)++;

    if (numRegions > 20) {
    }

    for (i = 0; i < numRegions; i++) {
        UNUSED s16 val, loX, loZ, hiX, hiZ;
        s16 height;

        val = *(*data)++;

        loX = *(*data)++;
        hiX = *(*data)++;
        loZ = *(*data)++;
        hiZ = *(*data)++;

        height = *(*data)++;

        gEnvironmentLevels[i] = height;
    }
}
