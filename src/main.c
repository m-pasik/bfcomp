#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "options.h"
#include "settings.h"
#include "compiler.h"
#include "functions.h"

int main(int argc, char **argv)
{
    settings.program_name = argv[0];

    /* Check if nasm and ld are installed */
    int nasm_status = system("nasm --version > /dev/null 2>&1");
    int ld_status = system("ld --version > /dev/null 2>&1");

    if (nasm_status)
        die("`nasm` not found.");

    if (ld_status)
        die("`ld` not found.");

    if (argc < 2)
        help(0, NULL);

    /*
     * Initialize `Options` struct
     * and declare command line arguments.
     */
    Options *options = init_options();

    /* Prints help */
    add_option(options, "help", 'h', 0, 0, help);

    /* Sets input and output files */
    add_option(options, "input", 'i', 1, 1, input_file);
    add_option(options, "output", 'o', 1, 1, output_file);
    add_option(options, NULL, 0, 1, 1, file);

    /* Sets stack and cell size */
    add_option(options, "stack_size", 's', 1, 1, stack_size);
    add_option(options, "cell_size", 'c', 1, 1, cell_size);

    /* Tells if output should be assembly. */
    add_option(options, "output_assembly", 'S', 0, 0, assembly);

    char **args = argv + 1;
    size_t arg_count = argc - 1;

    /*
     * Parse command line arguments
     */
    for (ArgInfo *arg = parse_argument(options, &arg_count, &args);
         arg;
         arg = parse_argument(options, &arg_count, &args)) {
        
        /* Exif if invalid argument was provided. */
        if (!arg->option) {
            fprintf(stderr, "ERROR: Invalid argument %s\n", arg->opt_argv[0]);
            exit(1);
        }

        /* Call function associated with the provided argument. */
        arg->option->call(arg->opt_argc, arg->opt_argv);

        free(arg);
    }

    free_options(options);

    /*
     * Check if input and output files were provided.
     */
    if (!settings.input_file)
        die("Input file not provided.");
    if (!settings.output_file)
        die("Output file not provided.");
    
    /*
     * Read input file.
     */
    FILE *input_file = fopen(settings.input_file, "r");
    if (input_file == NULL) {
        die("Failed to open input file.");
    }

    fseek(input_file, 0, SEEK_END);
    long file_size = ftell(input_file);
    fseek(input_file, 0, SEEK_SET);

    char *buffer = (char *)malloc(file_size + 1);

    size_t read_size = fread(buffer, 1, file_size, input_file);

    if (read_size != file_size)
        die("Error reading file.");

    buffer[file_size] = '\0';

    fclose(input_file);

    /*
     * Compile brainfuck code and check for errors.
     */

    char error;

    char *compiled = compile(buffer, &error);

    if (error == 2)
        die("File contains no brainfuck code.");

    if (error == 3)
        die("Unterminated brackets.");

    free(buffer);

    /*
     * Depending on the settings, output either assembly or an executable.
     */

    if (settings.assembly) {
        /* Open output file */
        FILE *output_file = fopen(settings.output_file, "w");    
        if (output_file == NULL)
            die("Failed to open output file.");

        /* Write compiled assembly into the output file. */
        fprintf(output_file, "%s", compiled);

        free(compiled);

        fclose(output_file);
    } else {
        /* Create temporary file. */
        char temp_name[] = "/tmp/bfcomp_XXXXXX";
        int temp_descriptor = mkstemp(temp_name);

        char temp_name_o[sizeof(temp_name) + 2];
        sprintf(temp_name_o, "%s.o", temp_name);

        /* Write compiled assembly into the temporary file. */
        FILE *temp_file = fdopen(temp_descriptor, "w");

        if (temp_file == NULL)
            die("Failed to create temporary file.");
        
        fprintf(temp_file, "%s", compiled);

        free(compiled);

        fclose(temp_file); 

        /* Assemble and link executable. */
        char nasm[] = "nasm -f elf64 -w-all %s";
        char ld[] = "ld %s.o -o %s";

        char nasm_command[sizeof(nasm) + L_tmpnam];
        sprintf(nasm_command, nasm, temp_name);
        system(nasm_command);

        char ld_command[sizeof(ld) + L_tmpnam + strlen(settings.output_file)];
        sprintf(ld_command, ld, temp_name, settings.output_file);
        system(ld_command);

        /* Remove temporary files. */
        char error = 0;
        if (remove(temp_name)) error = 1;
        if (remove(temp_name_o)) error = 1;

        if (error)
            die("Failed to remove temporary files");
    }

    return 0;
}

