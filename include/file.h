
#ifndef PROJECT_INCLUDES_FILE_H
#define PROJECT_INCLUDES_FILE_H

FILE* open_file(const char* filename, const char* mode);

FILE* open_readonly_file(const char* filename);

void close_file(FILE* file);

#endif // PROJECT_INCLUDES_FILE_H
