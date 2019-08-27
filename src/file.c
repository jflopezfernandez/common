
#include "common.h"

/** This is a utility function to explicitly allow the caller to test whether
 *  a file exists, which it manages by leveraging the access function and errno 
 *  to accomplish this. The goal is an interface similar to PathFileExists on
 *  Windows.
 * 
 */
int file_exists(const char* filename) {
    /* Reset errno */
    errno = 0;

    if (access(filename, R_OK) == -1) {
        switch (errno) {
            case EBADF:  return FALSE;
            case ENOENT: return FALSE;

            case EACCES:        /* Fallthrough */
            case ELOOP:         /* Fallthrough */
            case EINVAL:        /* Fallthrough */
            case ENAMETOOLONG:  /* Fallthrough */

            default: {
                fprintf(stderr, "[Error] %s\n", strerror(errno));
                exit(EXIT_FAILURE);
            } break;
        }
    }

    return TRUE;
}

/** This function wraps calls to 'fopen', serving as an intermediary through
 *  which the return value of 'fopen' is checked and handled. This interface
 *  thus guarantees that the return value of this function will not be null.
 * 
 */
FILE* open_file(const char* filename, const char* mode) {
    errno = 0;

    FILE* file = fopen(filename, mode);

    if (file == NULL) {
        fprintf(stderr, "[Error] %s (%s)\n", strerror(errno), filename);
        exit(EXIT_FAILURE);
    }

    return file;
}

/** This function is a wrapper around the open_file function interface; it
 *  supplies the "r" file-open mode for the caller and provides better
 *  semantic clarity that the intent of the caller is to open a file
 *  exclusively for reading.
 * 
 */
FILE* open_readonly_file(const char* filename) {
    return open_file(filename, "r");
}

/** This function wraps calls to 'fclose', the return value of which is almost
 *  never checked by most developers, who are completely unaware 'fclose' can
 *  fail. This interface ensures execution of the program is terminated on error
 *  so any code after this call will execute if and only if the call to 'fclose'
 *  succeeds.
 * 
 */
void close_file(FILE* file) {
    errno = 0;

    if (fclose(file) == EOF) {
        fprintf(stderr, "[Error] %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}

/** Just as with the analogous FILE* functions, open_file_descriptor is an
 *  interface through which error checking is implicitly carried out so the
 *  caller does not need to do it.
 * 
 */
int open_file_descriptor(const char* filename, int flags) {
    int file_descriptor = open(filename, flags);

    if (file_descriptor == -1) {
        fprintf(stderr, "[Error] %s (%s)\n", strerror(errno), filename);
        exit(EXIT_FAILURE);
    }

    return file_descriptor;
}

/** This function is a wrapper around the 'close' function, implicitly handling
 *  error-checking and return code validation. Just as with the previous
 *  functions, the caller may be assured that execution of any code after this
 *  call will proceed if and only if the call to 'close' succeeds.
 * 
 */
void close_file_descriptor(int file_descriptor) {
    errno = 0;

    if (close(file_descriptor) == -1) {
        fprintf(stderr, "[Error] %s\n", strerror(errno));
        exit(EXIT_FAILURE);
    }
}
