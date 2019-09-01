
#ifndef PROJECT_INCLUDES_COMMON_H
#define PROJECT_INCLUDES_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <setjmp.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <signal.h>
#include <time.h>
#include <locale.h>
#include <limits.h>
#include <inttypes.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/resource.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <syslog.h>
#include <termios.h>

#include <unistd.h>

#if !defined(FALSE) || !defined(TRUE)
enum { FALSE = 0, TRUE = !FALSE };
#endif // FALSE || TRUE

#ifndef BUFFER_SIZE
/** The initial buffer size of 4096 was chosen because to align with most
 *  modern hard drives, which have sectors exactly that long. That is the most
 *  efficient atomic unit, but profiling showed the sequential reads being
 *  performed here mean a much larger buffer size, 64Kb in this case, is
 *  actually beneficial.
 * 
 */
#define BUFFER_SIZE (4096)
#else
#error "BUFFER_SIZE already defined."
#endif // BUFFER_SIZE

#include "err.h"
#include "file.h"
#include "hash-table.h"
#include "mem.h"
#include "opt.h"
#include "settings.h"
#include "str.h"

int processing_first_file(void);

#endif // PROJECT_INCLUDES_COMMON_H
