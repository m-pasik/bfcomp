#ifndef SETTINGS_H
#define SETTINGS_H

#include <stddef.h>

typedef struct {
    char *program_name;
    char *input_file;
    char *output_file;
    size_t stack_size;
    size_t cell_size;
    char assembly;
    char *operation_register;
    char *data_unit;
} Settings;

extern Settings settings;

#endif
