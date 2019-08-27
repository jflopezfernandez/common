
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
   
#ifndef DELIMITERS
#define DELIMITERS " \n\t!\"#$%&'()*+,-./:;<=>?@[\\]^_`}|{~"
#endif

/** This is the buffer responsible for streamlining disk-read operations as
 *  much as possible. The buffer is 64kb long to benefit as much as possible
 *  from the sequential read that's going on in this admittedly contrived
 *  example, but generally speaking, the buffer size should always be a
 *  multiple of the sector size of the hard drive, as reading in sector-aligned
 *  chunks is the most efficient way of maximizing disk throughput.
 * 
 */
static char input_buffer[BUFFER_SIZE];

int main(int argc, char *argv[])
{
    const char** filenames = parse_command_line_options(argc, argv);

PROCESS_NEXT_FILE:
    errno = 0;
    
    int input_file_descriptor = open_file_descriptor(*filenames++, O_RDONLY);

    /** Having parsed the command-line arguments and successfully opened the
     *  first of two files, we will now inform the kernel about the significant
     *  sequential disk reads we are about to partake in. The kernel is not 
     *  obligated to do anything with this information, and in fact
     *  implementations aren't even required to implement any actual
     *  functionality for it, but we'll try it on the off-chance it makes a any
     *  difference.
     * 
     */
    if (posix_fadvise(input_file_descriptor, SEEK_SET, SEEK_END, POSIX_FADV_SEQUENTIAL)) {
        /** The only way this call fails is if the file descriptor is
         *  associated with a pipe/FIFO, the file descriptor is invalid, or the
         *  length of the file we specified was negative.
         * 
         */
    }

    ssize_t bytes_read = 0;

    while ((bytes_read = read(input_file_descriptor, input_buffer, BUFFER_SIZE))) {
        char* token_reentrancy_pointer = NULL;

        char* word = strtok_r(input_buffer, DELIMITERS, &token_reentrancy_pointer);

        if (word == NULL) {
            continue;
        }

        add_word_to_table(word);

        while ((word = strtok_r(NULL, DELIMITERS, &token_reentrancy_pointer)) != NULL) {
            add_word_to_table(word);
        }
    }

    close_file_descriptor(input_file_descriptor);

    if (first_file) {
        first_file = FALSE;

        goto PROCESS_NEXT_FILE;
    }

    /** This is the grand-finale; should there exist a string commonly found
     *  in both input files, the most_common_shared_word function will evaluate
     *  to true (as it is a pointer to said word), and the printf function will
     *  subsequently print it for the user.
     * 
     *  (Spoiler alert: it's probably "the")
     * 
     */
    if (most_common_shared_word()) {
        printf("%s\n", most_common_shared_word());
    }

    return EXIT_SUCCESS;
}
