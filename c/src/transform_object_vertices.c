#include "types.h"

#include "game/object_list_processor.h"
#include "game/object_helpers.h"

#include "math_ex.h"

#define TerrainData s16
#define o gCurrentObject

#define /*0x0A0*/ O_POS_INDEX                 0x06
#define /*0x0D0*/ O_FACE_ANGLE_INDEX                            0x12

extern void build_object_transform_from_pos_and_angle(struct Object *obj, s16 posIndex, s16 angleIndex);

void transform_object_vertices(TerrainData **data, TerrainData *vertexData) {
    Mat4 *objectTransform = &o->transform;

    register s32 numVertices = *(*data)++;

    register TerrainData *vertices = *data;

    if (o->header.gfx.throwMatrix == NULL) {
        o->header.gfx.throwMatrix = objectTransform;
        build_object_transform_from_pos_and_angle(o, O_POS_INDEX, O_FACE_ANGLE_INDEX);
    }

    Mat4 transform;
    apply_object_scale_to_matrix(gCurrentObject, transform, *objectTransform);

    // Go through all vertices, rotating and translating them to transform the object.
    Vec3f pos;
    while (numVertices--) {
        vec3s_to_vec3f(pos, vertices);
        vertices += 3;

        //! No bounds check on vertex data
        linear_mtxf_mul_vec3_and_translate(transform, vertexData, pos);

        vertexData += 3;
    }

    *data = vertices;
}
