
#ifndef PROJECT_INCLUDES_SETTINGS_H
#define PROJECT_INCLUDES_SETTINGS_H

struct settings_t {
    int verbose;
    int threads;
};

void settings_set_verbose(int setting);
void settings_set_threads(int setting);

int settings_get_verbose(void);
int settings_get_threads(void);

#endif // PROJECT_INCLUDES_SETTINGS_H
