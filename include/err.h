
#ifndef PROJECT_INCLUDES_ERR_H
#define PROJECT_INCLUDES_ERR_H

/** Ideally, I would have error messages for every error level (info, warning,
 *  fatal error). The only level in existence right now is this one, which is
 *  actually pretty helpful for debugging, as things crash at the first sign
 *  of an error, rather than much further down the line. The joys of developing
 *  software no one actually uses, I suppose.
 * 
 */
__attribute__((nonnull(1),noreturn))
void fatal_error(const char* error_message);

#endif // PROJECT_INCLUDES_ERR_H
