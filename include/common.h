
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

#include <sys/types.h>

#include <unistd.h>

#if !defined(FALSE) || !defined(TRUE)
enum { FALSE = 0, TRUE = !FALSE };
#endif // FALSE || TRUE

#ifndef BUFFER_SIZE
/** This buffer size was chosen because modern hard drives have sectors 4096
 *  bytes long, making this the most efficient size for buffered IO.
 * 
 */
#define BUFFER_SIZE (4096)
#else
#error "BUFFER_SIZE already defined."
#endif // BUFFER_SIZE

#include "file.h"
#include "hash-table.h"
#include "mem.h"
#include "opt.h"
#include "str.h"

int processing_first_file(void);

#endif // PROJECT_INCLUDES_COMMON_H
