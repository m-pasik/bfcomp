#ifndef DEFINES_H
#define DEFINES_H

#include <errno.h>

#define MEMERRV(ptr)     \
    if (!ptr) {         \
        errno = ENOMEM; \
        return;    \
    }

#define MEMERRN(ptr)     \
    if (!ptr) {         \
        errno = ENOMEM; \
        return NULL;    \
    }

#define MEMERRVF(ptr0, ptr1)    \
    if (!ptr0) {                \
        free(ptr1);             \
        errno = ENOMEM;         \
        return;                 \
    }

#define MEMERRNF(ptr0, ptr1)    \
    if (!ptr0) {                \
        free(ptr1);             \
        errno = ENOMEM;         \
        return NULL;                 \
    }

#endif
