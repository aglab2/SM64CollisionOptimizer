#include "types.h"
#include "surface_terrains.h"
#include "game/memory.h"
#include "behavior_data.h"

#define TerrainData s16
#define RoomData s8

extern s32 surface_has_force(s16 surfaceType);
extern s32 surf_has_no_cam_collision(s16 surfaceType);
extern struct Surface *read_surface_data(s16 *vertexData, s16 **vertexIndices);
extern void add_surface(struct Surface *surface, s32 dynamic);

void load_object_surfaces(TerrainData **data, TerrainData *vertexData, struct Object *o) {
    s32 i;

    s32 surfaceType = *(*data)++;
    s32 numSurfaces = *(*data)++;

    s32 hasForce = surface_has_force(surfaceType);

    s32 flags = surf_has_no_cam_collision(surfaceType) | SURFACE_FLAG_DYNAMIC;

    // The DDD warp is initially loaded at the origin and moved to the proper
    // position in paintings.c and doesn't update its room, so set it here.
    RoomData room = (o->behavior == segmented_to_virtual(bhvDddWarp)) ? 5 : 0;

    for (i = 0; i < numSurfaces; i++) {
        struct Surface *surface = read_surface_data(vertexData, data);

        if (surface != NULL) {
            surface->object = o;
            surface->type = surfaceType;

            if (hasForce) {
                surface->force = *(*data + 3);
            } else {
                surface->force = 0;
            }

            surface->flags |= flags;
            surface->room = room;
            add_surface(surface, TRUE);
        }

        if (hasForce) {
            *data += 4;
        } else {
            *data += 3;
        }
    }
}
