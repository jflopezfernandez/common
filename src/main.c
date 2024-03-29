
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

/** This object holds the parameters needed by each thread to execute the
 *  'thread_process_file' function, which each thread's main method. The object
 *  contains two fields:
 * 
 *      1. filename     The name of the input file to process
 *      2. file         The number (1 or 2) of the input file it is handling
 * 
 *  The thread's start function takes a single void pointer argument, meaning
 *  that we have to aggregate the arguments into a single object to then pass
 *  in.
 * 
 */
struct thread_arguments_t {
    const char* filename;
    int file;
};

/** This function's only job is to allocate the memory required by the thread
 *  arguments object. The function also ensures the pointer returned by the
 *  call to malloc is valid, and if it isn't, the function prints an error
 *  message to standard error and exits with a status code of EXIT_FAILURE. The
 *  caller may be sure that execution beyond the call to this function will
 *  take place if and only if the call is successful.
 * 
 */
static inline struct thread_arguments_t* allocate_thread_arguments(void) {
    struct thread_arguments_t* thread_arguments = malloc(sizeof (struct thread_arguments_t));

    if (thread_arguments == NULL) {
        fatal_error("Memory allocation failure in allocate_thread_arguments");
    }

    return thread_arguments;
}

/** This is the constructor for the thread arguments object. To invoke, the
 *  caller must pass in both the name of the input file and the file number
 *  designation. This is an arbitrary number (1 or 2) to distinguish the word
 *  counts for the file. This allows for a simple calculation of the harmonic
 *  mean using each count from both input files.
 * 
 *  A deep copy of the filename must be made via a call to strdup because the
 *  filename pointer's data is sitting in the thread_process_file function's
 *  input_buffer, which will be overwritten once the entire buffer has been
 *  processed.
 * 
 */
static inline struct thread_arguments_t* create_thread_arguments(const char* filename, int file) {
    struct thread_arguments_t* thread_arguments = allocate_thread_arguments();

    thread_arguments->filename = strdup(filename);

    if (thread_arguments->filename == NULL) {
        fatal_error("strdup() failed in create_thread_arguments");
    }

    thread_arguments->file = file;

    return thread_arguments;
}

/** This function takes care of safely freeing the heap-allocated memory
 *  resources used by the object pointed to by thread_arguments. The function
 *  uses the 'FREE' macro defined in mem.h to call the safe_free function,
 *  preventing the double-freeing of memory.
 * 
 */
__attribute__((nonnull(1)))
static inline void free_thread_arguments(struct thread_arguments_t* thread_arguments) {
    FREE(thread_arguments->filename);
    FREE(thread_arguments);
}
   
#ifndef DELIMITERS
/** These are the delimiters the strtok_r function will use to tokenize the
 *  input buffer. The program spec indicated valid strings would be composed of
 *  alphanumeric characters exclusively. These are all graphic non-alphanumeric
 *  ASCII characters, plus space.
 * 
 */
#define DELIMITERS " \n\t\"!#$%%&'()*+,-./:;<=>?@[\\]^_`}|{~"
#endif

static off_t f1_offset = 0;

static pthread_mutex_t f1_lock = PTHREAD_MUTEX_INITIALIZER;

static off_t f2_offset = 0;

static pthread_mutex_t f2_lock = PTHREAD_MUTEX_INITIALIZER;

void* thread_process_file(void* arg) {
    struct thread_arguments_t* thread_arguments = (struct thread_arguments_t *) arg;

    /** This is the buffer responsible for streamlining disk-read operations as
     *  much as possible. The buffer is 4kb long to benefit as much as possible
     *  from the sequential read that's going on in this admittedly contrived
     *  example, but generally speaking, the buffer size should always be a
     *  multiple of the sector size of the hard drive, as reading in
     *  sector-aligned chunks is the most efficient way of maximizing disk
     *  throughput.
     * 
     */
    char input_buffer[BUFFER_SIZE] = { 0 };

    int input_file_descriptor = open_file_descriptor(thread_arguments->filename, O_RDONLY);

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

    off_t current_offset = 0;

    while (TRUE) {
        /** Before performing any kind of read operations, we must first get
         *  the file offset at which this thread is to begin reading from,
         *  while at the same time incrementing the input file offset by the
         *  buffer size so the next thread begins reading from the next sector.
         * 
         *  While I usually prefer reader-writer locks to mutexes, a
         *  reader-writer lock gives us no additional functionality here. We
         *  can't simply lock the global offset in read mode, get our offset
         *  value, lock the the global offset in write mode, increment it, and
         *  unlock it. Other threads waiting to read and increment the global
         *  file offset may get spurious values if we treat read and write
         *  operations distinctly. Reading and writing operations on the global
         *  file offset must be treated atomically, so this is the perfect use
         *  case for a mutex.
         * 
         */
        if (thread_arguments->file == 1) {
            pthread_mutex_lock(&f1_lock);
            current_offset = f1_offset;
            f1_offset += BUFFER_SIZE;
            pthread_mutex_unlock(&f1_lock);
        } else if (thread_arguments->file == 2) {
            pthread_mutex_lock(&f2_lock);
            current_offset = f2_offset;
            f2_offset += BUFFER_SIZE;
            pthread_mutex_unlock(&f2_lock);
        } else {
            fatal_error("Invalid file id");
        }
        
        /** The benefit of using 'pread' over 'read' is that not only is pread
         *  equivalent to using 'lseek' then 'read', which is perfect here
         *  because each thread is keeping track of its own offset in the
         *  input file, but pread is an atomic IO operation.
         * 
         */
        bytes_read = pread(input_file_descriptor, input_buffer, BUFFER_SIZE, current_offset);

        if ((bytes_read == 0) || ((bytes_read == -1))) {
            break;
        }

        char* token_reentrancy_pointer = NULL;

        char* word = strtok_r(input_buffer, DELIMITERS, &token_reentrancy_pointer);

        if (word == NULL) {
            continue;
        }

        add_word_to_table(word, ((struct thread_arguments_t *) arg)->file);

        while ((word = strtok_r(NULL, DELIMITERS, &token_reentrancy_pointer)) != NULL) {
            add_word_to_table(word, ((struct thread_arguments_t *) arg)->file);
        }
    }

    close_file_descriptor(input_file_descriptor);

    return NULL;
}

/** This is the entry point of the program, which begins by calling the
 *  parse_command_line_options function. This function handles any options and
 *  validates the number of command line parameters. This allows the rest of
 *  the main function to deal only with the actual mechanics of processing each
 *  input file.
 * 
 */
int main(int argc, char *argv[])
{
    /** This argument vector returned by parse_command_line_options contains
     *  only the names of the filenames to process. The current program is
     *  structured to handle only two filenames, but this limit is arbitrary
     *  and could be changed with only minor modifications.
     * 
     */
    char** filenames = parse_command_line_options(argc, argv);

    const int total_threads    = settings_get_threads();
    const int threads_per_file = total_threads / 2;

    pthread_t* threads = malloc(total_threads * sizeof (pthread_t));

    if (threads == NULL) {
        fatal_error("Memory allocation failure in main()");
    }

    struct thread_arguments_t* t1_args = create_thread_arguments(filenames[0], 1);

    /** This pthread_attributes_t variable is used for configuring the
     *  attributes on newly created threads, which is especially useful given
     *  that we are automating the thread creation process below.
     * 
     */
    pthread_attr_t thread_attributes;

    /** Of the available configurable thread attributes, the only two I chose
     *  to modify are the minimum stack size and the guard buffer size. I'm not
     *  particularly keen on configuring the minimum stack address or the
     *  detached state of created threads.
     * 
     */
    pthread_attr_init(&thread_attributes);

    /** It's possible for there to be enough threads with big enough stacks to
     *  require more memory than is available in the process' virtual memory
     *  space.
     * 
     */
    pthread_attr_setstacksize(&thread_attributes, PTHREAD_STACK_MIN);

    /** The default guard size is usually the system page size. Depending on
     *  how many threads the user requests, this could add up to a
     *  computationally overhead. Program execution has been validated through
     *  testing using both gcov and valgrind, so I'm disabling the thread
     *  guard size feature.
     * 
     */
    pthread_attr_setguardsize(&thread_attributes, 0);

    for (int i = 0; i < threads_per_file; ++i) {
        if (pthread_create(&threads[i], &thread_attributes, thread_process_file, t1_args)) {
            fatal_error("Could not create new thread");
        }
    }

    struct thread_arguments_t* t2_args = create_thread_arguments(filenames[1], 2);

    for (int i = threads_per_file; i < total_threads; ++i) {
        if (pthread_create(&threads[i], &thread_attributes, thread_process_file, t2_args)) {
            fatal_error("Could not create thread");
        }
    }

    for (int i = 0; i < total_threads; ++i) {
        if (pthread_join(threads[i], NULL)) {
            fatal_error("Could not rejoin sub-threads");
        }
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

    /** The operating system will reclaim all process resources on termination,
     *  but for housekeeping purposes -specifically for valgrind to give us the
     *  okay on 0 bytes of heap memory in use at exit- I'm including these
     *  cleanup functions.
     * 
     *  The FREE macro calls the safe_free function defined in mem.h and
     *  prevents double-freeing heap-allocated memory.
     * 
     */
    free_thread_arguments(t1_args);
    free_thread_arguments(t2_args);

    FREE(filenames[0]);
    FREE(filenames[1]);
    FREE(filenames);

    pthread_attr_destroy(&thread_attributes);
    
    release_table_resources();

    return EXIT_SUCCESS;
}
