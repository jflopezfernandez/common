
#include "common.h"

/** This function is a wrapper around the C standard library's 'free' function.
 *  This interface is designed to help prevent accidentally double-freeing
 *  heap-allocated memory, so the function takes in the pointer to be freed by
 *  reference, forwarding it to the 'free' standard library function, then
 *  setting its value equal to NULL. This simply bookkeeping makes preventing
 *  double-free's much easier.
 * 
 */
void safe_free(void** ptr) {
    /** The pointer is first dereferenced and its value is checked against
     *  the NULL pointer. Any previously freed pointers will now point to the
     *  0x00 address, and any pointers that were not previously freed but
     *  currently point to NULL clearly do not need to be freed anyway, so the
     *  subroutine short-circuits here.
     * 
     */
    if (*ptr == NULL) {
        return;
    }

    /** Having verified the memory location is valid, we now call free as we
     *  normally would, with the sole exception being the added level of
     *  indirection required having passed in the pointer by reference rather
     *  than by value.
     * 
     */
    free(*ptr);

    /** Once the memory has been freed successfully, we set the pointer to
     *  point to NULL to make sure a second call to safe_free will immediately
     *  short-circuit rather than causing the program to abort.
     * 
     */
    *ptr = NULL;
}
