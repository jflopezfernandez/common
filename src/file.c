
#include "common.h"

/** This function wraps calls to 'fopen', serving as an intermediary through
 *  which the return value of 'fopen' is checked and handled. This interface
 *  thus guarantees that the return value of this function will not be null.
 * 
 */
__attribute__((nonnull(1,2), returns_nonnull))
FILE* open_file(const char* filename, const char* mode) {
    FILE* file = fopen(filename, mode);

    if (file == NULL) {
        fprintf(stderr, "Could not open file: %s\n", filename);
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
__attribute__((nonnull(1), returns_nonnull))
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
__attribute__((nonnull(1)))
void close_file(FILE* file) {
    if (fclose(file) == EOF) {
        fprintf(stderr, "Call to fclose() failed\n");
        exit(EXIT_FAILURE);
    }
}
