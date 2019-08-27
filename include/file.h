
#ifndef PROJECT_INCLUDES_FILE_H
#define PROJECT_INCLUDES_FILE_H

/** This is a utility function to explicitly allow the caller to test whether
 *  a file exists, which it manages by leveraging the access function and errno 
 *  to accomplish this. The goal is an interface similar to PathFileExists on
 *  Windows.
 * 
 */
__attribute__((nonnull(1)))
int file_exists(const char* filename);

/** This function wraps calls to 'fopen', serving as an intermediary through
 *  which the return value of 'fopen' is checked and handled. This interface
 *  thus guarantees that the return value of this function will not be null.
 * 
 */
__attribute__((nonnull(1,2), returns_nonnull))
FILE* open_file(const char* filename, const char* mode);

/** This function is a wrapper around the open_file function interface; it
 *  supplies the "r" file-open mode for the caller and provides better
 *  semantic clarity that the intent of the caller is to open a file
 *  exclusively for reading.
 * 
 */
__attribute__((nonnull(1), returns_nonnull))
FILE* open_readonly_file(const char* filename);

/** This function wraps calls to 'fclose', the return value of which is almost
 *  never checked by most developers, who are completely unaware 'fclose' can
 *  fail. This interface ensures execution of the program is terminated on error
 *  so any code after this call will execute if and only if the call to 'fclose'
 *  succeeds.
 * 
 */
__attribute__((nonnull(1)))
void close_file(FILE* file);

/** Just as with the analogous FILE* functions, open_file_descriptor is an
 *  interface through which error checking is implicitly carried out so the
 *  caller does not need to do it.
 * 
 */
__attribute__((nonnull(1)))
int open_file_descriptor(const char* filename, int flags);

/** This function is a wrapper around the 'close' function, implicitly handling
 *  error-checking and return code validation. Just as with the previous
 *  functions, the caller may be assured that execution of any code after this
 *  call will proceed if and only if the call to 'close' succeeds.
 * 
 */
__attribute__((flatten))
void close_file_descriptor(int file_descriptor);

#endif // PROJECT_INCLUDES_FILE_H
