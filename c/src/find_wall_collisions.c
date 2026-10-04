#include "types.h"
#include "engine/surface_collision.h"
#include "engine/surface_load.h"
#include "config_world.h"
#include "cfg.h"

#define MAX_REFERENCED_WALLS 4

struct Find1Result
{
    struct Surface* surf;
    f32 dx, dz;
    int cornerThresholded;
    int edge;
};

struct Find1Result find_wall_best(const Vec3f pos, f32 radius, const f32 margin_radius);

/**
 * Iterate through the list of walls until all walls are checked and
 * have given their wall push.
 */
s32 find_wall_collisions(struct WallCollisionData *data) {
    s32 numCollisions = 0;
    data->numWalls = 0;

    struct Surface* reported_surfaces[100];
    int reported_surfaces_count = 0;
    f32 radius = data->radius;
    f32 margin_radius = data->radius - 1.0f;
    Vec3f pos = { data->x, data->y + data->offsetY, data->z };

    int numCols = 0;
    while (reported_surfaces_count < 100)
    {
        struct Find1Result result = find_wall_best(pos, radius, margin_radius);
        if (!result.surf)
            break;

        result.surf->flags |= 0x80;
        reported_surfaces[reported_surfaces_count++] = result.surf;

        if (!result.cornerThresholded)
        {
            if (data->numWalls < MAX_REFERENCED_WALLS)
            {
                data->walls[data->numWalls++] = result.surf;
            }
            numCols++;
        }
        
        if (result.edge)
        {
            margin_radius += gCollisionConfig.normalFloorCeilThreshold;
        }

        pos[0] += result.dx;
        pos[2] += result.dz;
    }

    for (int i = 0; i < reported_surfaces_count; i++)
    {
        reported_surfaces[i]->flags &= ~0x80;
    }

    data->x = pos[0];
    data->z = pos[2];
    return numCols;
}
