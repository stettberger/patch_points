#ifndef _PATCH_POINT_H_
#define _PATCH_POINT_H_

#include <stdbool.h>

struct patch_point {
    const char *name;
    char jump_to_block;
    signed int jump_offset;
    char *jump_ptr;
    // Link element
    struct patch_point * next;
};

struct patch_point_value {
    const char *name;
    bool value;
    struct patch_point_value *next;
};

typedef struct {
    struct patch_point *points;
    struct patch_point_value *values;
} patch_point_list;

void patch_point_set(patch_point_list *ppl, const char *name, bool on);
bool patch_point_get(patch_point_list *ppl, const char *name);

// This is needed by the patch_point macro
__attribute__((noinline))
__attribute__((fastcall))
int
__patch_point(patch_point_list *ppl, const char *name);

#define patch_point(ppl, name) if (__patch_point(ppl, name) == 23)
#define patch_point_disable(ppl, name) patch_point_set(ppl, name, 0)
#define patch_point_enable(ppl, name) patch_point_set(ppl, name, 1)

#endif /* _PATCH_POINT_H_ */
