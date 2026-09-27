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
