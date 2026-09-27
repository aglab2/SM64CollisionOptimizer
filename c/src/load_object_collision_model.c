#include "types.h"

#include "game/object_list_processor.h"
#include "engine/graph_node.h"

#include "object_constants.h"
#include "object_fields.h"
#include "surface_terrains.h"

void load_object_collision_model(void) {
    UNUSED u8 filler[4];
    s16 vertexData[600];

    s16 *collisionData = gCurrentObject->collisionData;
    f32 marioDist = gCurrentObject->oDistanceToMario;
    f32 tangibleDist = gCurrentObject->oCollisionDistance;

    // On an object's first frame, the distance is set to 19000.0f.
    // If the distance hasn't been updated, update it now.
    if (gCurrentObject->oDistanceToMario == 19000.0f) {
        marioDist = dist_between_objects(gCurrentObject, gMarioObject);
    }

    // If the object collision is supposed to be loaded more than the
    // drawing distance of 4000, extend the drawing range.
    if (gCurrentObject->oCollisionDistance > 4000.0f) {
        gCurrentObject->oDrawingDistance = gCurrentObject->oCollisionDistance;
    }

    // Update if no Time Stop, in range, and in the current room.
    if (!(gTimeStopState & TIME_STOP_ACTIVE) && marioDist < tangibleDist
        && !(gCurrentObject->activeFlags & ACTIVE_FLAG_IN_DIFFERENT_ROOM)) {
        collisionData++;
        transform_object_vertices(&collisionData, vertexData);

        // TERRAIN_LOAD_CONTINUE acts as an "end" to the terrain data.
        while (*collisionData != TERRAIN_LOAD_CONTINUE) {
            load_object_surfaces(&collisionData, vertexData);
        }
    }

    if (marioDist < gCurrentObject->oDrawingDistance) {
        gCurrentObject->header.gfx.node.flags |= GRAPH_RENDER_ACTIVE;
    } else {
        gCurrentObject->header.gfx.node.flags &= ~GRAPH_RENDER_ACTIVE;
    }
}
