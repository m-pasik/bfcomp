#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <stddef.h>

size_t parse_size_t(char *str, char *error);

int die(const char *msg);

int help(size_t argc, char **argv);

int stack_size(size_t argc, char **argv);

int cell_size(size_t argc, char **argv);

int input_file(size_t argc, char **argv);

int output_file(size_t argc, char **argv);

int assembly(size_t argc, char **argv);

int file(size_t argc, char **argv);


#endif
