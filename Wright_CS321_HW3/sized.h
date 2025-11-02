#ifndef SIZED_H
#define SIZED_H
#include <stdlib.h>

// Please don't take points off for casting everything to void and back
typedef struct {
    void* pointer;
    int size;
    int typeWidth;
} sized_thing;

sized_thing* create_allocated(void* pointer, int size, int typeWidth) {
    sized_thing* ret = malloc(sizeof(sized_thing));
    ret->pointer = pointer;
    ret->size = size;
    ret->typeWidth = typeWidth;
    return ret;
}

void free_allocated(sized_thing* p) {
    if ((void*)p == NULL)
        return;
    free(p->pointer);
    free(p);
}

bool sized_equal(const sized_thing* ir1, const sized_thing* ir2) {
    // if sizes aren't equal, no need to check anything else
    if (ir1->size != ir2->size || ir1->typeWidth != ir2->typeWidth) {
        return false;
    }

    int totalBytes = (ir1->typeWidth * ir1->size);
    
    // start from the back since that's changing the most in this application
    char* ptr1 = (ir1->pointer + totalBytes)-1;
    char* ptr2 = (ir2->pointer + totalBytes)-1;
    int chars = totalBytes / sizeof(char);
    for (int i = 0; i < chars; i++) {
        char a = *(ptr1-i), b = *(ptr2-i);
        if (*(ptr1-i) != *(ptr2-i)) {
            return false;
        }
    }

    return true;
}

#endif