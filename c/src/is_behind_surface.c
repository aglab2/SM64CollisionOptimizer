#include "types.h"

s32 is_behind_surface(Vec3f pos, struct Surface *surf) {
    s32 behindSurface = 0;
    // Surface normal
    f32 normX = (surf->vertex2[1] - surf->vertex1[1]) * (surf->vertex3[2] - surf->vertex2[2]) -
                (surf->vertex3[1] - surf->vertex2[1]) * (surf->vertex2[2] - surf->vertex1[2]);
    f32 normY = (surf->vertex2[2] - surf->vertex1[2]) * (surf->vertex3[0] - surf->vertex2[0]) -
                (surf->vertex3[2] - surf->vertex2[2]) * (surf->vertex2[0] - surf->vertex1[0]);
    f32 normZ = (surf->vertex2[0] - surf->vertex1[0]) * (surf->vertex3[1] - surf->vertex2[1]) -
                (surf->vertex3[0] - surf->vertex2[0]) * (surf->vertex2[1] - surf->vertex1[1]);
    f32 dirX = surf->vertex1[0] - pos[0];
    f32 dirY = surf->vertex1[1] - pos[1];
    f32 dirZ = surf->vertex1[2] - pos[2];

    if (dirX * normX + dirY * normY + dirZ * normZ < 0) {
        behindSurface = 1;
    }
    return behindSurface;
}
