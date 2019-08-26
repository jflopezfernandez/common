
#include "common.h"

FILE* open_file(const char* filename, const char* mode) {
    FILE* file = fopen(filename, mode);

    if (file == NULL) {
        fprintf(stderr, "Could not open file: %s\n", filename);
        exit(EXIT_FAILURE);
    }

    return file;
}

FILE* open_readonly_file(const char* filename) {
    return open_file(filename, "r");
}

void close_file(FILE* file) {
    if (fclose(file) == EOF) {
        fprintf(stderr, "Call to fclose() failed\n");
        exit(EXIT_FAILURE);
    }
}
