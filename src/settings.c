#include "settings.h"

Settings settings = {
    .program_name = NULL,
    .input_file = NULL,
    .output_file = NULL,
    .stack_size = 30000,
    .cell_size = 1,
    .assembly = 0,
    .operation_register = "r12b",
    .data_unit = "byte"
};
