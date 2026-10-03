#include "types.h"
#include "math_ex.h"

#define TerrainData s16
#define Vec3t Vec3s

#define NEAR_ZERO 0.0001f
#define SURFACE_VERTICAL_BUFFER 5

extern s32 gSurfacesAllocated;
extern struct Surface *sSurfacePool;

static inline struct Surface *alloc_surface(void) {

    struct Surface *surface = &sSurfacePool[gSurfacesAllocated];
    gSurfacesAllocated++;

    surface->type = 0;
    surface->force = 0;
    surface->flags = 0;
    surface->room = 0;
    surface->object = NULL;

    return surface;
}

struct Surface *read_surface_data(s16 *vertexData, s16 **vertexIndices) {
    Vec3f v[3];
    Vec3f n;
    Vec3t offset;
    s16 min, max;

    vec3_scale_dest(offset, (*vertexIndices), 3);

    vec3s_copy(v[0], (vertexData + offset[0]));
    vec3s_copy(v[1], (vertexData + offset[1]));
    vec3s_copy(v[2], (vertexData + offset[2]));

    find_vector_perpendicular_to_plane(n, v[0], v[1], v[2]);

    f32 mag = (sqr(n[0]) + sqr(n[1]) + sqr(n[2]));
    // This will never need to be run for custom levels because Fast64 does this step before exporting.
    // assert(mag >= NEAR_ZERO, "Denorm tri was found.");
    if (mag < NEAR_ZERO) {
        return NULL;
    }
    mag = 1.0f / __builtin_sqrtf(mag);
    vec3_scale(n, mag);

    struct Surface *surface = alloc_surface();

    vec3s_copy(surface->vertex1, v[0]);
    vec3s_copy(surface->vertex2, v[1]);
    vec3s_copy(surface->vertex3, v[2]);

    surface->normal.x = n[0];
    surface->normal.y = n[1];
    surface->normal.z = n[2];

    surface->originOffset = -vec3_dot(n, v[0]);

    min_max_3s(v[0][1], v[1][1], v[2][1], &min, &max);
    surface->lowerY = (min - SURFACE_VERTICAL_BUFFER);
    surface->upperY = (max + SURFACE_VERTICAL_BUFFER);

    return surface;
}
