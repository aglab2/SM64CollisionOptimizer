#include "types.h"

#define TerrainData s16
#define RoomData s8

extern s32 surface_has_force(s16 surfaceType);
extern s32 surf_has_no_cam_collision(s16 surfaceType);

extern struct Surface *read_surface_data(s16 *vertexData, s16 **vertexIndices);
extern void add_surface(struct Surface *surface, s32 dynamic);

void load_static_surfaces(TerrainData **data, TerrainData *vertexData, s32 surfaceType, RoomData **surfaceRooms) {
    s32 i;
    struct Surface *surface;
    RoomData room = 0;
    s16 hasForce = surface_has_force(surfaceType);
    s32 flags = surf_has_no_cam_collision(surfaceType);

    s32 numSurfaces = *(*data)++;

    for (i = 0; i < numSurfaces; i++) {
        if (*surfaceRooms != NULL) {
            room = *(*surfaceRooms)++;
        }

        surface = read_surface_data(vertexData, data);
        if (surface != NULL) {
            surface->room = room;
            surface->type = surfaceType;
            surface->flags = flags;

            if (hasForce) {
                surface->force = *(*data + 3);
            } else {
                surface->force = 0;
            }

            add_surface(surface, FALSE);
        }

        *data += 3;
        if (hasForce) {
            (*data)++;
        }
    }
}
