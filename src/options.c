#include "options.h"

/*
 * Initializes `Options` struct. (it's pretty obvious)
 */
Options *init_options()
{
    Options *options = (Options*)malloc(sizeof(Options));
    options->count = 0;
    options->size = 100;
    options->word = NULL;
    options->list = (Option*)malloc(options->size * sizeof(Option));
    return options;
}

/*
 * Frees `Options` struct. (another useless comment)
 */
void free_options(Options *options)
{
    for (int i = 0; i < options->count; i++)
        free(options->list[i].key);
    free(options->word);
    free(options->list);
    free(options);
}

/*
 * Adds a new `Option` to `options->list`.
 */
void add_option(Options *options, char *key, char key_short,
                size_t arg_min, size_t arg_max, OptionFunction function)
{
    /*
     * Determines option type based on `key` and `key_short`.
     * 
     * OPTION_LONG will be chosen if only `key` is provided.
     * Long options are ones starting with `--`.
     *
     * OPTION_SHORT will be chosen if only `key_short` is provided.
     * Short options are ones starting with `-`.
     *
     * If both `key` and `key_short` are provided,
     * `type` will be OPTION_LONG | OPTION_SHORT (0b11). 
     *
     * OPTION_WORD is the default type if no keys were provided.
     * There can only be one option of that type
     * and arguments not starting with '-' will default to it,
     * which will result in calling the function with one argument
     * corresponding to the command line option provided.
     */

    int type = OPTION_WORD | (key ? OPTION_LONG : 0) | (key_short ? OPTION_SHORT : 0);

    /*
     * If the function type is OPTION_WORD and a options->word
     * is NULL assign new option to options->word.
     * If options->word already exists, overwrite it
     * with the new function.
     */
    if (type == OPTION_WORD) {
        if (!options->word)
            options->word = (Option*)malloc(sizeof(Option));

        options->word->type = type;
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
        options->list = (Option*)realloc(options->list, options->size * sizeof(Option));
    }

    /* This part sets up the new option in options->list. */ 

    options->list[options->count].type = type; /* Do I even have to explain it? */
    
    if (key)
        options->list[options->count].key = strdup(key);
    else
        options->list[options->count].key = NULL;

    options->list[options->count].key_short = key_short;
    options->list[options->count].arg_min = arg_min;
    options->list[options->count].arg_max = arg_max;
    options->list[options->count].call = function;
    ++options->count;
}

/*
 * Returns option by `key`.
 */
Option *get_option(Options *options, int type, char *key)
{
    switch (type) {
        /* If `type` == OPTION_LONG searches long keys. */
        case OPTION_LONG:
            for (int i = 0; i < options->count; i++)
                if (strcmp(options->list[i].key, key) == 0)
                    return &options->list[i];
            return NULL;

        /* If `type` == OPTION_SHORT searches long keys. */
        case OPTION_SHORT:
            for (int i = 0; i < options->count; i++)
                if (options->list[i].key_short == *key &&
                    *(key + 1) == '\0')
                    return &options->list[i];
            return NULL; 

        /* If `type` == OPTION_WORD returns options->word. */
        case OPTION_WORD:
            return options->word;
    }

    return NULL;
}

/*
 * Gets first argument in `argv`
 * updates `argc` and `argv`, and returns
 * associated function (`ArgInfo.call`),
 * passed arguments (`ArgInfo.opt_argv`)
 * and numbeber of arguments (`ArgInfo.opt_argc`).
 */
ArgInfo *parse_argument(Options *options, size_t *argc, char ***argv)
{
    if (*argc < 1)
        return NULL;
    
    ArgInfo *info = malloc(sizeof(ArgInfo));

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
         * If option was returned update `argc` and `argv` to point to the next option,
         * and set `info->opt_argv` to point to the first argument provided to the option.
         * Else also point to the next argument but return pointer to the invalid argument
         * in `opt_argv` and number of arguments to the next option in `opt_argc`.
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

