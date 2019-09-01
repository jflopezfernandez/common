
#include "common.h"

static struct settings_t settings;

void settings_set_verbose(int setting) {
    settings.verbose = setting;
}

void settings_set_threads(int setting) {
    settings.threads = setting;
}

int settings_get_verbose(void) {
    return settings.verbose;
}

int settings_get_threads(void) {
    return settings.threads;
}
