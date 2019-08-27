
#include "common.h"

#ifndef DISPLAY_COLUMNS
#define DISPLAY_COLUMNS (80)
#endif 

/** Ideally, I would have error messages for every error level (info, warning,
 *  fatal error). The only level in existence right now is this one, which is
 *  actually pretty helpful for debugging, as things crash at the first sign
 *  of an error, rather than much further down the line. The joys of developing
 *  software no one actually uses, I suppose.
 * 
 */
void fatal_error(const char* error_message) {
    fprintf(stderr, "[Error] %s\n", error_message);
    exit(EXIT_FAILURE);
}

#if defined(DISPLAY_COLUMNS)
#undef DISPLAY_COLUMNS
#endif
