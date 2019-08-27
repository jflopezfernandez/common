
/** Write a C99+ command line program (GNU and Linux-specific extensions are fine)
 *  that takes two files as arguments and attempts to find the word that is most
 *  commonly shared between the files. The files consist only of ASCII characters
 *  and a word is delimited by any non-alphanumeric character. Both files will be
 *  warmed up and are in the operating system buffer cache. The test files will
 *  span 2GB each. Optimize for speed.
 *  
 *  You are welcome to use non-LSB third party source-code but it must either come
 *  from Concurrency Kit, a package that is available on Debian or packaged with
 *  your distribution. C++ is not permitted.
 *  
 *  Please document any cool optimizations you make as well as basic usage.
 *  
 *  Target Environment:
 *  Processor: Intel(R) Core(TM) i7-2600K CPU @ 3.40GHz
 *  Cores: 4 
 *  Kernel: Linux 3.2.0
 *  Compiler: clang 3.6.0 or GCC 4.7.2+
 *  Storage medium: SSD
 *  
 *  Expected Usage:
 *  ./program <file 1> <file 2>
 *  word
 *  
 *  In the above example, "word" is the most commonly used word between <file 1>
 *  and <file 2>.
 *  
 */

#include "common.h"

static int first_file = TRUE;

int processing_first_file(void) {
    return first_file;
}

/** This is the buffer responsible for streamlining disk-read operations as
 *  much as possible. The buffer is 4096 bytes to match the sector size of most
 *  modern hard drives. Reading in sector-aligned chunks is the most efficient
 *  way of interacting with disk memory.
 * 
 */
static char input_buffer[BUFFER_SIZE];
    
#ifndef DELIMITERS
#define DELIMITERS " \n\t!\"#$%&'()*+,-./:;<=>?@[\\]^_`}|{~"
#endif

int main(int argc, char *argv[])
{
    const char** filenames = parse_command_line_options(argc, argv);

    FILE* input_file = open_readonly_file(*filenames++);

    char* token_reentrancy_pointer = NULL;

    while (TRUE) {
        if (fgets(input_buffer, BUFFER_SIZE, input_file) == NULL) {
            close_file(input_file);

            if (!first_file) {
                break;
            }

            first_file = FALSE;

            input_file = open_readonly_file(*filenames);

            continue;
        }
        
        char* word = strtok_r(input_buffer, DELIMITERS, &token_reentrancy_pointer);

        if (word == NULL) {
            continue;
        }

        add_word_to_table(word);

        while ((word = strtok_r(NULL, DELIMITERS, &token_reentrancy_pointer)) != NULL) {
            add_word_to_table(word);
        }
    }

    if (most_common_shared_word()) {
        printf("%s\n", most_common_shared_word());
    }

    return EXIT_SUCCESS;
}
