#ifndef LIMITS_H
#define LIMITS_H
#include "sized.h"
#include <stdlib.h>

typedef struct {
    sized_thing* min; // start from this int representation
    sized_thing* max; // quit when the int rep equals this
    sized_thing* target; // quit when the whole int rep equals this
    int index;
} limits;

limits* create_limits(int len, sized_thing* target) {
    limits* ret = malloc(sizeof(limits));
    ret->min = create_allocated(malloc(sizeof(short) * len), len, sizeof(short));
    ret->max = create_allocated(malloc(sizeof(short) * len), len, sizeof(short));
    ret->target = target;
    return ret;
}

void free_limits(limits* p) {
    if ((void*)p == (void*)0)
        return;
    free_allocated(p->min);
    free_allocated(p->max);
    free(p);
}
#endif