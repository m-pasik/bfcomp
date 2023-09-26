#include <asm-generic/errno-base.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <inttypes.h>

#include "settings.h"
#include "defines.h"
#include "compiler.h"

/*
 * Struct storing compiled code.
 */ 
typedef struct {
    size_t size;
    size_t length;
    char *data;
} CompileBuffer;

/* 
 * Stores information about the instruction that should be written to the buffer
 */
typedef struct {
    char type;
    char read_needed;
    char write_needed;
    char increment_needed;
    int64_t value;
} Instruction;

#define INS_WRITE_NEEDED                            \
    if (instruction->write_needed) {                \
        buffer->length +=                           \
            sprintf(buffer->data + buffer->length,  \
                    "mov %s [r14], %s\n",           \
                    settings.data_unit,             \
                    settings.operation_register);   \
        instruction->write_needed = 0;              \
    }

#define INS_READ_NEEDED                             \
    if (instruction->read_needed) {                 \
        buffer->length +=                           \
            sprintf(buffer->data + buffer->length,  \
                    "mov %s, %s [r14]\n",           \
                    settings.operation_register,    \
                    settings.data_unit);            \
        instruction->read_needed = 0;               \
    }

#define INS_INCREMENT_NEEDED                        \
    if (instruction->increment_needed) {            \
        buffer->length +=                           \
            sprintf(buffer->data + buffer->length,  \
                    "mov r14, stack\n"              \
                    "add r14, r13\n");              \
        instruction->increment_needed = 0;          \
    }


/*
 * Writes assembly equivalent to specified brainfuck instruction.
 *
 * @param   buffer      CompileBuffer to write to.
 * @param   instruction Instruction to be written.
 */
void write_instruction(CompileBuffer *buffer, Instruction *instruction)
{
    /* Make sure there is enough space in the buffer. */
    if (buffer->length + 255 > buffer->size) {
        buffer->size *= 2;
        char *tmp = realloc(buffer->data, buffer->size);
        MEMERRVF(tmp, buffer->data);
        buffer->data = tmp;
    }

    char *ins;
    int64_t value;

    switch (instruction->type) {
        case 1:
            /* Move stack pointer by value. */
            INS_WRITE_NEEDED

            value = (instruction->value % (int64_t)settings.stack_size) + (int64_t)settings.stack_size;

            buffer->length +=
                sprintf(buffer->data + buffer->length, 
                        "mov rax, r13\n"
                        "add rax, %"PRId64"\n"
                        "mov rcx, %zu\n"
                        "xor rdx, rdx\n"
                        "div rcx\n"
                        "mov r13, rdx\n",
                        value * settings.cell_size,
                        settings.stack_size * settings.cell_size);
            break;
        case 2:
            /* Increase value in a cell pointed to by the stack pointer by value. */
            INS_INCREMENT_NEEDED
            INS_READ_NEEDED

            if (instruction->value < 0) {
                ins = "sub";
                value = -instruction->value;
            } else {
                ins = "add";
                value = instruction->value;
            }

            buffer->length +=
                sprintf(buffer->data + buffer->length,
                        "%s %s, %"PRId64"\n",
                        ins, settings.operation_register, value);
            break;
        case 3:
            /* Print character in the cell pointed to by the stack pointer. */
            INS_WRITE_NEEDED
            INS_INCREMENT_NEEDED

            buffer->length +=
                sprintf(buffer->data + buffer->length,
                        "mov rax, 1\n"
                        "mov rdi, 1\n"
                        "mov rsi, r14\n"
                        "mov rdx, 1\n"
                        "syscall\n");
            break;
        case 4:
            /* Read character from stdin to the cell pointed to by the stack pointer. */
            INS_INCREMENT_NEEDED

            buffer->length +=
                sprintf(buffer->data + buffer->length,
                        "mov rax, 0\n"
                        "mov rdi, 0\n"
                        "mov rsi, r14\n"
                        "mov rdx, 1\n"
                        "syscall\n");
                break;
        case 5:
            /* Start loop. */
            INS_WRITE_NEEDED
            INS_INCREMENT_NEEDED
            INS_READ_NEEDED

            buffer->length +=
                sprintf(buffer->data + buffer->length,
                        "cmp %s, 0\n"
                        "je endloop%"PRId64"\n"
                        "loop%"PRId64":\n",
                        settings.operation_register,
                        instruction->value, instruction->value);
            break;
        case 6:
            /* End loop. */
            INS_WRITE_NEEDED
            INS_INCREMENT_NEEDED
            INS_READ_NEEDED

            buffer->length +=
                sprintf(buffer->data + buffer->length,
                        "cmp %s, 0\n"
                        "jne loop%"PRId64"\n"
                        "endloop%"PRId64":\n", 
                        settings.operation_register,
                        instruction->value, instruction->value);
            break;                        
    }
}

char *compile(char *code)
{
    if (code == NULL) {
        errno = EINVAL;
        return NULL;
    }

    errno = 0;

    /* Initialize buffer for compiled code. */
    CompileBuffer buffer;
    buffer.size = 8192;
    buffer.length = 0;
    buffer.data = malloc(buffer.size);

    MEMERRN(buffer.data)

    /*
     * Write beginning of the code to the buffer.
     * Allocates an array of size settings.stack_size and initializes it to 0.
     * Initializes stack pointer r13 to 0.
     */
    buffer.length +=
        sprintf(buffer.data,
            "section .bss\n"
            "stack res%c %zu\n"
            "section .text\n"
            "global _start\n"
            "_start:\n"
            "mov rdi, stack\n"
            "mov rcx, %zu\n"
            "xor %s, %s\n"
            "rep stos%c\n"
            "xor r13, r13\n"
            "xor %s, %s\n"
            "mov r14, stack\n",
            *settings.data_unit, settings.stack_size,
            settings.stack_size, settings.operation_register,
            settings.operation_register, *settings.data_unit,
            settings.operation_register, settings.operation_register);

    char exit_call[] =
        "mov rax, 0x3c\n"
        "mov rdi, 0\n"
        "syscall\n";

    char last = 0;
    int64_t value;

    size_t c = 0;
    for (size_t i = 0; code[i] != '\0'; ++i) c+= code[i] == '[';

    size_t bracket_index = 0; /* Index of the last opened brackets in bracket_labels. */
    int64_t *bracket_labels = calloc(c, sizeof(int64_t)); /* Numbers assigned to each loop */
    int64_t label_index = 0; /* Total number of loops so that each label is unique. */

    Instruction instruction = {
        .type = 0,
        .read_needed = 0,
        .write_needed = 0,
        .increment_needed = 0,
        .value = 0
    };

#define INS_WRITE                                   \
    if (instruction.type) {                         \
        write_instruction(&buffer, &instruction);   \
        if (errno)                                  \
            return NULL;                            \
    }
    
    for ( ; *code != '\0'; ++code) {
        switch (*code) {
            case '>': /* Move stack pointer to the right. */
                if (instruction.type == 1) {
                    ++instruction.value;
                    break;
                }

                INS_WRITE

                instruction.type = 1;
                instruction.value = 1;
                instruction.read_needed = 1;
                instruction.increment_needed = 1;

                break;
            case '<': /* Move stack pointer to the left. */
                if (instruction.type == 1) {
                    --instruction.value;
                    break;
                }

                INS_WRITE

                instruction.type = 1;
                instruction.value = -1;
                instruction.read_needed = 1;
                instruction.increment_needed = 1;

                break;
            case '+': /* Increase value in a cell pointed to by the stack pointer by value. */
                if (instruction.type == 2) {
                    ++instruction.value;
                    break;
                }

                INS_WRITE

                instruction.type = 2;
                instruction.value = 1;
                instruction.write_needed = 1;

                break;
            case '-': /* Decrease value in a cell pointed to by the stack pointer by value. */
                if (instruction.type == 2) {
                    --instruction.value;
                    break;
                }

                INS_WRITE

                instruction.type = 2;
                instruction.value = -1;
                instruction.write_needed = 1;

                break;
            case '.': /* Print character in the cell pointed to by the stack pointer. */
                INS_WRITE

                instruction.type = 3;

                break;
            case ',': /* Read character from stdin to the cell pointed to by the stack pointer. */
                INS_WRITE
                
                instruction.type = 4;
                instruction.read_needed = 1;
                instruction.write_needed = 0;

                break;
            case '[': /* Start loop. */
                INS_WRITE

                instruction.type = 5;
                instruction.value = label_index;

                bracket_labels[bracket_index++] = label_index++;
                
                break;
            case ']': /* End loop. */
                INS_WRITE

                instruction.type = 6;
                instruction.value = bracket_labels[--bracket_index];

                break;
        }
    }

    /* Check if there was any brainfuck code and return error. */
    INS_WRITE
    else
        errno = ENOCODE;

    /* Check if brackets are closed and return error. */
    if (bracket_index)
        errno = EUNCLOSED;

    free(bracket_labels);

    /* Make sure the buffer is large enough for the exit call */
    if (buffer.length + sizeof(exit_call) > buffer.size) {
        buffer.size += sizeof(exit_call);
        char *tmp = realloc(buffer.data, buffer.size);
        MEMERRNF(tmp, buffer.data)
        buffer.data = tmp;
    }

    /* Write exit syscall to buffer. */
    buffer.length +=
        sprintf(buffer.data + buffer.length, "%s", exit_call);

    return buffer.data;
};
