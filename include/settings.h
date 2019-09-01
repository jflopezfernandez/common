
#ifndef PROJECT_INCLUDES_SETTINGS_H
#define PROJECT_INCLUDES_SETTINGS_H

/** There are two command-line options at the moment that affect the operation
 *  of the application. The first controls the verbosity of the output during
 *  program execution. It has been implemented in a limited capacity so far,
 *  but the option itself is fully operational. The threads setting controls
 *  how many threads the program creates to operate on the input files. At the
 *  moment, the number of threads is expected to be even to be cleanly divided
 *  up into the two input files, and if it isn't, it is incremented by one.
 * 
 */
struct settings_t {
    int verbose;
    int threads;
};

void settings_set_verbose(int setting);
void settings_set_threads(int setting);

int settings_get_verbose(void);
int settings_get_threads(void);

#endif // PROJECT_INCLUDES_SETTINGS_H
