#include "types.h"
#include "math_ex.h"
#include "surface_terrains.h"
#include "engine/surface_collision.h"
#include "engine/surface_load.h"
#include "game/object_list_processor.h"
#include "game/level_update.h"
#include "object_constants.h"
#include "sm64.h"
#include "game/room.h"

#define o gCurrentObject

#define TerrainData s16

#define MAX_REFERENCED_WALLS 4

static s32 check_wall_vw(f32 d00, f32 d01, f32 d11, f32 d20, f32 d21, f32 mult) {
    f32 v = ((d11 * d20) - (d01 * d21));
    if (v < 0.0f || v > mult) {
        return TRUE;
    }

    f32 w = ((d00 * d21) - (d01 * d20));
    if (w < 0.0f || w > mult || v + w > mult) {
        return TRUE;
    }

    return FALSE;
}

static s32 check_wall_edge(Vec3f vert, Vec3f v2, f32 *d00, f32 *d01, f32 *invDenom, f32 *offset, f32 margin_radius) {
    if (FLT_IS_NONZERO(vert[1])) {
        f32 v = (v2[1] / vert[1]);
        if (v < 0.0f || v > 1.0f) {
            return TRUE;
        }

        *d00 = ((vert[0] * v) - v2[0]);
        *d01 = ((vert[2] * v) - v2[2]);
        *invDenom = sqrtf(sqr(*d00) + sqr(*d01));
        *offset = (*invDenom - margin_radius);

        return (*offset > 0.0f);
    }

    return TRUE;
}

struct Find1Result
{
    struct Surface* surf;
    f32 dx, dz;
    int cornerThresholded;
};

static inline struct Find1Result find_wall_collisions_from_list1(struct SurfaceNode *surfaceNode, f32 radius, const Vec3f pos, struct WallCollisionData *data)
{
    const f32 corner_threshold = -0.9f;
    struct Surface *surf;
    f32 offset;

    Vec3f v0, v1, v2;
    f32 d00, d01, d11, d20, d21;
    TerrainData type = SURFACE_DEFAULT;

    const f32 margin_radius = radius - 1.0f;
    struct Find1Result result;
    result.surf = 0;
    f32 best = 1000.f;

    // Stay in this loop until out of walls.
    while (surfaceNode != NULL) {
        surf        = surfaceNode->surface;

        // TODO: Optimize...
        for (int i = 0; i < data->numWalls; i++)
        {
            if (surf == data->walls[i])
                continue;
        }

        surfaceNode = surfaceNode->next;
        type        = surf->type;

        // Exclude a large number of walls immediately to optimize.
        if (pos[1] < surf->lowerY || pos[1] > surf->upperY) continue;

        // Determine if checking for the camera or not.
        if (gCheckingSurfaceCollisionsForCamera) {
            if (surf->flags & SURFACE_FLAG_NO_CAM_COLLISION) continue;
        } else {
            // Ignore camera only surfaces.
            if (type == SURFACE_CAMERA_BOUNDARY) continue;

            // If an object can pass through a vanish cap wall, pass through.
            if (type == SURFACE_VANISH_CAP_WALLS && o != NULL) {
                // If an object can pass through a vanish cap wall, pass through.
                if (o->activeFlags & ACTIVE_FLAG_MOVE_THROUGH_GRATE) continue;
                // If Mario has a vanish cap, pass through the vanish cap wall.
                if (o == gMarioObject && gMarioState->flags & MARIO_VANISH_CAP) continue;
            }
        }

        // Dot of normal and pos, + origin offset
        offset = (surf->normal.x * pos[0])
               + (surf->normal.y * pos[1])
               + (surf->normal.z * pos[2])
               + surf->originOffset;

        // Exclude surfaces outside of the radius.
        if (offset < -radius || offset > radius) continue;

        vec3_diff(v0, surf->vertex2, surf->vertex1);
        vec3_diff(v1, surf->vertex3, surf->vertex1);
        vec3_diff(v2, pos,           surf->vertex1);

        // Face
        d00 = vec3_dot(v0, v0);
        d01 = vec3_dot(v0, v1);
        d11 = vec3_dot(v1, v1);
        d20 = vec3_dot(v2, v0);
        d21 = vec3_dot(v2, v1);

        f32 mult = (d00 * d11) - (d01 * d01);
        if (check_wall_vw(d00, d01, d11, d20, d21, mult)) {
            if (offset < 0) {
                continue;
            }

            // Edge 1-2
            f32 invDenom;
            if (check_wall_edge(v0, v2, &d00, &d01, &invDenom, &offset, margin_radius)) {
                // Edge 1-3
                if (check_wall_edge(v1, v2, &d00, &d01, &invDenom, &offset, margin_radius)) {
                    vec3_diff(v1, surf->vertex3, surf->vertex2);
                    vec3_diff(v2, pos, surf->vertex2);
                    // Edge 2-3
                    if (check_wall_edge(v1, v2, &d00, &d01, &invDenom, &offset, margin_radius)) {
                        continue;
                    }
                }
            }

            f32 priority = invDenom;

            // Check collision
            if (FLT_IS_NONZERO(invDenom)) {
                invDenom = (offset / invDenom);
            }

            // Update pos
            if (priority < best)
            {
                result.dx = (d00 *= invDenom);
                result.dz = (d01 *= invDenom);
                best = priority;
                result.surf = surf;
                if ((d00 * surf->normal.x) + (d01 * surf->normal.z) < (corner_threshold * offset)) {
                    result.cornerThresholded = 1;
                }
            }
        } else {
            f32 priority = offset <= 0.f ? offset + 100.f : offset;
            if (priority < best)
            {
                result.dx = surf->normal.x * (radius - offset);
                result.dz = surf->normal.z * (radius - offset);
                best = priority;
                result.surf = surf;
                result.cornerThresholded = 0;
            }
        }
    }

    return result;
}

/**
 * Iterate through the list of walls until all walls are checked and
 * have given their wall push.
 */
s32 find_wall_collisions_from_list(struct SurfaceNode *surfaceNode, struct WallCollisionData *data) {
    Vec3f pos = { data->x, data->y + data->offsetY, data->z };

    int numCols = 0;
    for (int i = 0; i < MAX_REFERENCED_WALLS; i++)
    {
        struct Find1Result result = find_wall_collisions_from_list1(surfaceNode, data->radius, pos, data);
        if (!result.surf)
            break;

        if (!result.cornerThresholded)
        {
            if (data->numWalls < MAX_REFERENCED_WALLS)
            {
                data->walls[data->numWalls++] = result.surf;
            }
            numCols++;
        }

        pos[0] += result.dx;
        pos[2] += result.dz;
    }

    data->x = pos[0];
    data->z = pos[2];
    return numCols;
}
