#pragma once

#define sqr(x) ({         \
    __auto_type _x = (x); \
    _x * _x; })

// Get the minimum / maximum of a set of numbers
#undef MIN
#define MIN(a, b) ({      \
    __auto_type _a = (a); \
    __auto_type _b = (b); \
    _a < _b ? _a : _b; })

#undef MAX
#define MAX(a, b) ({      \
    __auto_type _a = (a); \
    __auto_type _b = (b); \
    _a > _b ? _a : _b; })

#define min_3(a, b, c) MIN(MIN(a, b), c)

#define max_3(a, b, c) MAX(MAX(a, b), c)

#define min_3f min_3
#define min_3i min_3
#define min_3s min_3

#define max_3f max_3
#define max_3i max_3
#define max_3s max_3

// Set vector 'dst' to the sum of vectors 'src1' and 'src2'
#define vec2_sum(dst, src1, src2) {         \
    __auto_type _x = (src1)[0] + (src2)[0]; \
    __auto_type _y = (src1)[1] + (src2)[1]; \
    (dst)[0] = _x;                          \
    (dst)[1] = _y;                          \
}
#define vec3_sum(dst, src1, src2) {         \
    __auto_type _x = (src1)[0] + (src2)[0]; \
    __auto_type _y = (src1)[1] + (src2)[1]; \
    __auto_type _z = (src1)[2] + (src2)[2]; \
    (dst)[0] = _x;                          \
    (dst)[1] = _y;                          \
    (dst)[2] = _z;                          \
}
#define vec4_sum(dst, src1, src2) {         \
    __auto_type _x = (src1)[0] + (src2)[0]; \
    __auto_type _y = (src1)[1] + (src2)[1]; \
    __auto_type _z = (src1)[2] + (src2)[2]; \
    __auto_type _w = (src1)[3] + (src2)[3]; \
    (dst)[0] = _x;                          \
    (dst)[1] = _y;                          \
    (dst)[2] = _z;                          \
    (dst)[3] = _w;                          \
}

#define vec3f_sum vec3_sum
#define vec3i_sum vec3_sum
#define vec3s_sum vec3_sum

#define vec3_diff(dst, src1, src2) {        \
    __auto_type _x = (src1)[0] - (src2)[0]; \
    __auto_type _y = (src1)[1] - (src2)[1]; \
    __auto_type _z = (src1)[2] - (src2)[2]; \
    (dst)[0] = _x;                          \
    (dst)[1] = _y;                          \
    (dst)[2] = _z;                          \
}

// Add the vector 'src' to vector 'dst'
#define vec2_add(dst, src) vec2_sum((dst), (dst), (src))
#define vec3_add(dst, src) vec3_sum((dst), (dst), (src))
#define vec4_add(dst, src) vec4_sum((dst), (dst), (src))

// Get the maximum and minimum of three numbers at the same time.
#define min_max_3_func(a, b, c, min, max) { \
    if (b < a) {                            \
        *max = a;                           \
        *min = b;                           \
    } else {                                \
        *min = a;                           \
        *max = b;                           \
    }                                       \
    if (c < *min) *min = c;                 \
    if (c > *max) *max = c;                 \
}

static inline __attribute__((always_inline)) void min_max_3f(f32 a, f32 b, f32 c, f32 *min, f32 *max) { min_max_3_func(a, b, c, min, max); }
static inline __attribute__((always_inline)) void min_max_3i(s32 a, s32 b, s32 c, s32 *min, s32 *max) { min_max_3_func(a, b, c, min, max); }
static inline __attribute__((always_inline)) void min_max_3s(s16 a, s16 b, s16 c, s16 *min, s16 *max) { min_max_3_func(a, b, c, min, max); }

#define vec3_scale_dest(dst, src, x) {  \
    __auto_type _x = (src)[0] * (x);    \
    __auto_type _y = (src)[1] * (x);    \
    __auto_type _z = (src)[2] * (x);    \
    (dst)[0] = _x;                      \
    (dst)[1] = _y;                      \
    (dst)[2] = _z;                      \
}

#define vec3_copy(dst, src) {           \
    __auto_type _x = (src)[0];          \
    __auto_type _y = (src)[1];          \
    __auto_type _z = (src)[2];          \
    (dst)[0] = _x;                      \
    (dst)[1] = _y;                      \
    (dst)[2] = _z;                      \
}
#define vec3f_copy vec3_copy
#define vec3i_copy vec3_copy
#define vec3s_copy vec3_copy

#define vec3s_to_vec3f vec3_copy

#define vec3_scale_dest(dst, src, x) {  \
    __auto_type _x = (src)[0] * (x);    \
    __auto_type _y = (src)[1] * (x);    \
    __auto_type _z = (src)[2] * (x);    \
    (dst)[0] = _x;                      \
    (dst)[1] = _y;                      \
    (dst)[2] = _z;                      \
}
#define vec3_scale(dst, x) vec3_scale_dest(dst, dst, x)

#define vec2_dot(a, b)       (((a)[0] * (b)[0]) + ((a)[1] * (b)[1]))
#define vec3_dot(a, b)      (vec2_dot((a), (b)) + ((a)[2] * (b)[2]))

#define find_vector_perpendicular_to_plane(dest, a, b, c) {                                     \
    (dest)[0] = ((b)[1] - (a)[1]) * ((c)[2] - (b)[2]) - ((c)[1] - (b)[1]) * ((b)[2] - (a)[2]);  \
    (dest)[1] = ((b)[2] - (a)[2]) * ((c)[0] - (b)[0]) - ((c)[2] - (b)[2]) * ((b)[0] - (a)[0]);  \
    (dest)[2] = ((b)[0] - (a)[0]) * ((c)[1] - (b)[1]) - ((c)[0] - (b)[0]) * ((b)[1] - (a)[1]);  \
}

// Transform the vector 'srcV' by the matrix 'mtx' and store the result in 'dstV'. Ignores translation.
#define linear_mtxf_mul_vec3(mtx, dstV, srcV) {                                                         \
    __auto_type _x = ((mtx)[0][0] * (srcV)[0]) + ((mtx)[1][0] * (srcV)[1]) + ((mtx)[2][0] * (srcV)[2]); \
    __auto_type _y = ((mtx)[0][1] * (srcV)[0]) + ((mtx)[1][1] * (srcV)[1]) + ((mtx)[2][1] * (srcV)[2]); \
    __auto_type _z = ((mtx)[0][2] * (srcV)[0]) + ((mtx)[1][2] * (srcV)[1]) + ((mtx)[2][2] * (srcV)[2]); \
    (dstV)[0] = _x;                                                                                     \
    (dstV)[1] = _y;                                                                                     \
    (dstV)[2] = _z;                                                                                     \
}

// Transform the vector 'srcV' by the matrix 'mtx' including translation, and store the result in 'dstV'
#define linear_mtxf_mul_vec3_and_translate(mtx, dstV, srcV) { \
    linear_mtxf_mul_vec3((mtx), (dstV), (srcV));              \
    vec3_add((dstV), (mtx)[3]);                               \
}

// Absolute value
#define ABS(x) ({         \
    __auto_type _x = (x); \
    _x > 0 ? _x : -_x; })
#define absi ABS
#define abss ABS
#define absf ABS

#define NEAR_ZERO   0.0001f
#define FLT_IS_NONZERO(x) (absf(x) > NEAR_ZERO)
