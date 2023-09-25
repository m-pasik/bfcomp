#ifndef COMPILER_H
#define COMPILER_H

#include <errno.h>

#define ENOCODE 1001
#define EUNCLOSED 1002

char *compile(char *code);

#endif
