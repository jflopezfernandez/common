
#ifndef PROJECT_INCLUDES_OPT_H
#define PROJECT_INCLUDES_OPT_H

typedef enum {
    OPTION_NONE,
    OPTION_HELP,
    OPTION_VERSION,
    OPTION_VERBOSE,
    OPTION_TOP_N
} option_id_t;

struct option_t {
    option_id_t id;
    const char* short_option;
    const char* long_option;
    const char* description;
};

const char** parse_command_line_options(int argc, char *argv[]);

#endif // PROJECT_INCLUDES_OPT_H
