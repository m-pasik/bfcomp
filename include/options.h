#ifndef OPTIONS_H
#define OPTIONS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#define OPTION_WORD 0b00
#define OPTION_LONG 0b01
#define OPTION_SHORT 0b10

typedef int (*OptionFunction)(size_t argc, char **argv);

typedef struct {
    int type;
    char *key;
    char key_short;
    size_t arg_min;
    size_t arg_max;
    OptionFunction call;
} Option;

typedef struct {
    size_t opt_argc;
    char **opt_argv;
    Option *option;
} ArgInfo;

typedef struct {
    size_t count;
    size_t size;
    Option *word;
    Option *list;
} Options;

Options *init_options();
void free_options(Options *options);
void add_option(Options *options, char *key, char key_short,
                size_t arg_min, size_t arg_max, OptionFunction function);
Option *get_option(Options *options, int type, char *key);
ArgInfo *parse_argument(Options *options, size_t *argc, char ***argv);


#endif
