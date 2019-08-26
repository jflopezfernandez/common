
#ifndef PROJECT_INCLUDES_MEM_H
#define PROJECT_INCLUDES_MEM_H

void safe_free(void** ptr);

#ifndef FREE
#define FREE(ptr) ((void **) &(ptr))
#endif

#endif // PROJECT_INCLUDES_MEM_H
