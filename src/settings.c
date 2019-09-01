
#include "common.h"

/** There is a single settings object created during the lifetime of the
 *  application and it is here; the settings object itself has internal linkage
 *  and is accessed via the getter and setter functions.
 * 
 */
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
