
#include "common.h"

#ifndef NONE
#define NONE ""
#endif

/** This is the structure that holds option information. It has been declared
 *  static so as to imbue internal linkage upon it, thereby preventing the
 *  pollution of the global namespace.
 * 
 *  Unimplemented:
 *      -N
 *      -v, --verbose
 * 
 */
static struct option_t options[] = {
    { OPTION_HELP   , "-h", "--help"   , "Display this help menu and exit"                      },
    { OPTION_VERSION, NONE, "--version", "Display program version info and exit"                },
    { OPTION_VERBOSE, "-v", "--verbose", "Display detailed info during program execution"       }
};

static size_t number_of_program_options = sizeof (options) / sizeof (options[0]);

/** This function takes care of automatically printing and correctly formatting
 *  the available program command-line options using only the options struct.
 * 
 *  Currently the help menu looks as follows:
 * 
 *    Usage: common [OPTIONS...] FILE1 FILE2
 *    Find the most common string shared between two files.
 *
 *        -h, --help                   Display this help menu and exit
 *            --version                Display program version info and exit
 *        -v, --verbose                Display detailed info during program execution
 * 
 */
static void print_options_help(void) {
    for (size_t i = 0; i < number_of_program_options; ++i) {
        const char* short_option_delimiter = ",";

        if (strings_match(options[i].short_option, NONE) || strings_match(options[i].long_option, NONE)) {
            short_option_delimiter = " ";
        }

        printf("%6s%s %-24s %s\n", options[i].short_option, short_option_delimiter, options[i].long_option, options[i].description);
    }
}

__attribute__((nonnull(1)))
static option_id_t string_matches_program_option(const char* str) {
    for (size_t i = 0; i < number_of_program_options; ++i) {
        if (strings_match(options[i].short_option, str) || strings_match(options[i].long_option, str)) {
            return options[i].id;
        }
    }

    return OPTION_NONE;
}

typedef enum {
    NORMAL,
    ERROR
} message_type_t;

static const char* usage_str = "Usage: common [OPTIONS...] FILE1 FILE2";

static void print_usage(message_type_t message_type) {
    fprintf((message_type) ? stderr : stdout, "%s\n", usage_str);
}

static const char* help_str = "Find the most common string shared between two files.";

static void print_help(void) {
    print_usage(NORMAL);
    printf("%s\n\n", help_str);
    print_options_help();
    printf("\n");
}

static const char* version_str = "common 1.0.0";

static void print_version_info(void) {
    printf("%s\n", version_str);
}

static const char** arguments = NULL;

static size_t number_of_non_option_arguments = 0;

__attribute__((hot, nonnull(1)))
static void add_argument(const char* argument) {
    /** The program specification calls for accepting two and only two
     *  filename arguments. While this seems a little arbitrary, The Spec's
     *  Will Be Done.
     * 
     *  If we already have two filename arguments in the argument vector, we
     *  will unapologetically exit with an error.
     * 
     */
    if (number_of_non_option_arguments == 2) {
        fprintf(stderr, "Too many filename arguments passed in.\n");
        exit(EXIT_FAILURE);
    }

    arguments = reallocarray(arguments, sizeof (const char *), ++number_of_non_option_arguments);

    if (arguments == NULL) {
        fprintf(stderr, "Memory allocation failure in add_argument()\n");
        exit(EXIT_FAILURE);
    }

    arguments[number_of_non_option_arguments - 1] = strdup(argument);

    if (arguments[number_of_non_option_arguments - 1] == NULL) {
        fprintf(stderr, "Memory allocation failure in add_argument()->strdup()\n");
        exit(EXIT_FAILURE);
    }
}

/** This is the main driver function for taking care of parsing command-line
 *  arguments. The original argument vector is clobbered by the function for
 *  simplicity. Specifically, nothing really happens to argv, but the any
 *  non-option are passed to main via the string array returned by this
 *  function. The program spec limits this array to a size of exactly two, but
 *  this limit is arbitrary.
 * 
 */
const char** parse_command_line_options(int argc, char *argv[]) {
    for (int i = 1; i < argc; ++i) {
        option_id_t option_id = string_matches_program_option(argv[i]);

        if (option_id) {
            switch (option_id) {
                case OPTION_HELP: {
                    print_help();
                    exit(EXIT_SUCCESS);
                } break;

                case OPTION_VERSION: {
                    print_version_info();
                    exit(EXIT_SUCCESS);
                } break;

                default: {
                    fprintf(stderr, "Invalid option id: %d\n", option_id);
                    exit(EXIT_FAILURE);
                } break;
            }
        } else {
            add_argument(argv[i]);
        }
    }

    /** Having finished iterating through all the command-line arguments, if
     *  the user did not supply the requisite amount of filenames for the 
     *  program to process, we must exit with an error message now.
     * 
     *  The user could have invoked the program with a '--help' or '--version'
     *  flag, which would have also amounted to an incorrect number of command-
     *  line parameters, so this is really the best time for us to exit, having
     *  thus verified we really are missing arguments.
     * 
     *  Had the user requested help or version information, we would never have
     *  made it to this point in the subroutine.
     * 
     */
    if (number_of_non_option_arguments != 2) {
        print_usage(ERROR);
        exit(EXIT_FAILURE);
    }

    return arguments;
}

#undef NONE
