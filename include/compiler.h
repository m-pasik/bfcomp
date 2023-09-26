#ifndef COMPILER_H
#define COMPILER_H

#include <errno.h>

#define ENOCODE 1001
#define EUNCLOSED 1002

/*
 * Takes brainfuck code and returns assembly.
 *
 * In case of an error writes it to errno.
 * EINVAL if string wasn't provided.
 * ENOCODE if the provided string contains no brainfuck code.
 * EUNCLOSED if brackets were not closed.
 * ENOMEM if memory allocation failed.
 *
 * @param   code    String with brainfuck code.
 * @return          Compiled NASM assembly.
 */
char *compile(char *code);

#endif
