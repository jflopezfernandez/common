
#ifndef PROJECT_INCLUDES_OPT_H
#define PROJECT_INCLUDES_OPT_H

typedef enum {
    OPTION_NONE,
    OPTION_HELP,
    OPTION_VERSION,
    OPTION_VERBOSE
} option_id_t;

struct option_t {
    option_id_t id;
    const char* short_option;
    const char* long_option;
    const char* description;
};

/** This is the main driver function for taking care of parsing command-line
 *  arguments. The original argument vector is clobbered by the function for
 *  simplicity. Specifically, nothing really happens to argv, but the any
 *  non-option are passed to main via the string array returned by this
 *  function. The program spec limits this array to a size of exactly two, but
 *  this limit is arbitrary.
 * 
 */
__attribute__((nonnull(2), returns_nonnull))
char** parse_command_line_options(int argc, char *argv[]);

#endif // PROJECT_INCLUDES_OPT_H
