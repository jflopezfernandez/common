
#include "common.h"

void safe_free(void** ptr) {
    if (*ptr == NULL) {
        return;
    }

    free(*ptr);

    *ptr = NULL;
}
