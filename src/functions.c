#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "settings.h"
#include "functions.h"

/* 
 * Parses string to size_t.
 * Supports decimal, hex and binary.
 */
size_t parse_size_t(char *str, char *error)
{
    char *endptr;
    size_t num;
    
    *error = 0;

    if (strncmp(str, "0x", 2) == 0)
        num = strtoul(str + 2, &endptr, 16);
    else if (strncmp(str, "0b", 2) == 0)
        num = strtoul(str + 2, &endptr, 2);
    else
        num = strtoul(str, &endptr, 10);

    if (*endptr != '\0' && *endptr != '\n' &&
        *endptr != '\r' && *endptr != '\t' &&
        *endptr != ' ') 
        *error = 1;

    return num;
}

/*
 * Exits process with error.
 */
int die(const char *msg)
{
    fprintf(stderr, "ERROR: %s\n", msg);
    exit(1);
}

/*
 * Displays help
 */
int help(size_t argc, char **argv)
{
    printf("Usage: %s [options]\n\n", settings.program_name);
    printf("Options:\n"
           "  --help                -h  -- Displays help.\n"
           "  --input_file <file>   -i  -- Sets input file.\n"
           "  --output_file <file>  -o  -- Sets output file.\n"
           "  --stack_size <value>  -s  -- Sets length of the stack.\n"
           "  --cell_size <value>   -c  -- Sets cell size. (Accepts 1, 2, 4 or 8 bytes)\n"
           "  --assembly            -S  -- Outputs assembly instead of an executable.\n");
    exit(0);
    return 0;
}

/*
 * Sets stack size.
 */
int stack_size(size_t argc, char **argv)
{
    if (!argc)
        die("Stack size not provided.");

    char err;
    size_t stack_size = parse_size_t(argv[0], &err);
    
    if (err)
        die("Stack size must be a number.");

    if (stack_size <= 0)
        die("Stack size must be greater than 0.");

    settings.stack_size = stack_size;
    return 0;
}

/*
 * Sets cell size, data unit and register for operations.
 */
int cell_size(size_t argc, char **argv)
{
    if (!argc)
        die("Cell size not provided.");

    char err;
    size_t cell_size = parse_size_t(argv[0], &err);

    if (err)
        die("Cell size must be a number.");

    switch (cell_size) {
        case 1:
            settings.operation_register = "r12b";
            settings.data_unit = "byte"; 
            break;
        case 2:
            settings.operation_register = "r12w";
            settings.data_unit = "word";
            break;
        case 4:
            settings.operation_register = "r12d";
            settings.data_unit = "dword";
            break;
        case 8:
            settings.operation_register = "r12";
            settings.data_unit = "qword";
            break;
        default:
            die("Cell size must be 1, 2, 4 or 8 bytes.");
    }

    settings.cell_size = cell_size;

    return 0;
}

/*
 * Sets input file.
 */
int input_file(size_t argc, char **argv)
{
    if (!argc)
        die("Input file not provided.");
    if (!settings.input_file)
        settings.input_file = argv[0];
    else
        die("Provide only one input file.");
    return 0;
}

/*
 * Sets output file.
 */
int output_file(size_t argc, char **argv)
{
    if (!argc)
        die("Output file not provided.");
    if (!settings.output_file)
        settings.output_file = argv[0];
    else
        die("Provide only one output file.");
    return 0;
}

/*
 * Sets if the output should be in assembly.
 */
int assembly(size_t argc, char **argv)
{
    settings.assembly = 1;
    return 0;
}

/*
 * Sets either input or output file if it was provided
 * without the use of 'input_file' or 'output_file' option.
 */
int file(size_t argc, char **argv)
{
    if (!settings.input_file)
        settings.input_file = argv[0];
    else if (!settings.output_file)
        settings.output_file = argv[0];
    else
        die("Too many files provided.");
    return 0;
}

