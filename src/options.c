#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "defines.h"
#include "options.h"

Options *init_options()
{
    errno = 0;
    Options *options = (Options*)malloc(sizeof(Options));
    MEMERRN(options);
    options->count = 0;
    options->size = 100;
    options->word = NULL;
    options->list = (Option*)malloc(options->size * sizeof(Option));
    MEMERRNF(options->list, options);
    return options;
}

void free_options(Options *options)
{
    for (int i = 0; i < options->count; i++)
        free(options->list[i].key);
    free(options->word);
    free(options->list);
    free(options);
}

void add_option(Options *options, char *key, char key_short,
                size_t arg_min, size_t arg_max, OptionFunction function)
{
    errno = 0;

    /*
     * Assign new option to options->word if keys aren't set.
     * If options->word already exists, overwrite it.
     */
    if (!key && !key_short) {
        if (!options->word) {
            options->word = (Option*)malloc(sizeof(Option));
            MEMERRV(options->word);
        }

        options->word->key = NULL;
        options->word->key_short = 0;
        options->word->arg_min = 1;
        options->word->arg_max = 1;
        options->word->call = function;

        return;
    }

    /* 
     * Allocate more memory if number of defined options
     * exceeds the size of the array. (pretty obvious) 
     */
    if (options->count == options->size) {
        options->size += 100;
        Option *tmp = (Option*)realloc(options->list, options->size * sizeof(Option));
        MEMERRV(options->list)
        options->list = tmp;
    }

    /* This part sets up the new option in options->list. */ 

    if (key) {
        options->list[options->count].key = strdup(key);
        MEMERRV(options->list[options->count].key)
    } else {
        options->list[options->count].key = NULL;
    }

    options->list[options->count].key_short = key_short;
    options->list[options->count].arg_min = arg_min;
    options->list[options->count].arg_max = arg_max;
    options->list[options->count].call = function;
    ++options->count;
}

Option *get_option(Options *options, int type, char *key)
{
    switch (type) {
        /* If type == OPTION_LONG searches long keys. */
        case OPTION_LONG:
            for (int i = 0; i < options->count; i++)
                if (strcmp(options->list[i].key, key) == 0)
                    return &options->list[i];
            return NULL;

        /* If type == OPTION_SHORT searches long keys. */
        case OPTION_SHORT:
            for (int i = 0; i < options->count; i++)
                if (options->list[i].key_short == *key &&
                    *(key + 1) == '\0')
                    return &options->list[i];
            return NULL; 

        /* If type == OPTION_WORD returns options->word. */
        case OPTION_WORD:
            return options->word;
    }

    return NULL;
}

ArgInfo *parse_argument(Options *options, size_t *argc, char ***argv)
{
    if (*argc < 1)
        return NULL;
    
    ArgInfo *info = malloc(sizeof(ArgInfo));
    MEMERRN(info);

    /* If argument doesn't start with '-' assume OPTION_WORD */
    if ((*argv)[0][0] != '-') {
        info->option = options->word;
        info->opt_argc = 1;
        info->opt_argv = *argv;
        ++*argv;
        --*argc;
    } else {
        /*
         * If argument tarts with '--' it's OPTION_LONG with corresponding key,
         * and if it starts with '-' get OPTION_SHORT with corresponding key.
         */
        if ((*argv)[0][1] == '-')
            info->option = get_option(options, OPTION_LONG, (*argv)[0] + 2);
        else
            info->option = get_option(options, OPTION_SHORT, (*argv)[0] + 1);

        /*
         * If option was returned update argc and argv to point to the next option,
         * and set info->opt_argv to point to the first argument provided to the option.
         * Else also point to the next argument but return pointer to the invalid argument
         * in opt_argv and number of arguments to the next option in opt_argc.
         */ 
        if (info->option) {
            for (info->opt_argc = 1;
                 info->opt_argc < *argc && info->opt_argc < info->option->arg_max + 1
                 && (*argv)[info->opt_argc][0] != '-';
                 ++info->opt_argc);

            *argc -= info->opt_argc;
            info->opt_argv = *argv + 1;
            *argv += info->opt_argc;
            --info->opt_argc;

        } else {
            for (info->opt_argc = 1;
                 info->opt_argc < *argc && (*argv)[info->opt_argc][0] != '-';
                 ++info->opt_argc);

            *argc -= info->opt_argc;
            info->opt_argv = *argv;
            *argv += info->opt_argc;
        }
    }

    return info;
}

