
#ifndef PROJECT_INCLUDES_MEM_H
#define PROJECT_INCLUDES_MEM_H

/** This function is a wrapper around the C standard library's 'free' function.
 *  This interface is designed to help prevent accidentally double-freeing
 *  heap-allocated memory, so the function takes in the pointer to be freed by
 *  reference, forwarding it to the 'free' standard library function, then
 *  setting its value equal to NULL. This simply bookkeeping makes preventing
 *  double-free's much easier.
 * 
 */
void safe_free(void** ptr);

#ifndef FREE
/** This preprocessor definition is a convenience macro, making it much simpler
 *  for the user to call the 'safe_free' function. The macro includes the
 *  address-of operator so the caller does not have to worry about remembering
 *  to pass in the pointer by reference, and the macro also includes the cast
 *  to mitigate spurious type errors from the compiler.
 * 
 */
#define FREE(ptr) ((void **) &(ptr))
#endif

#endif // PROJECT_INCLUDES_MEM_H
