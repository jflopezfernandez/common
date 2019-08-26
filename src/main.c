
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
    
#ifndef DELIMITERS
#define DELIMITERS " \n\t!\"#$%&'()*+,-./:;<=>?@[\\]^_`}|{~"
#endif

#ifndef HASH_MODULUS
#define HASH_MODULUS (52379)
#else
#error "HASH_MODULUS already defined."
#endif // HASH_MODULUS

static inline size_t hash(char* s) {
    size_t hashval = 0;

    while (*s) {
        hashval = (*s++) + 211 * hashval;
    }
    
    return hashval % HASH_MODULUS;
}

static double current_max = 0.0;

static char* most_common_word = NULL;

static int first_file = TRUE;

/** Perfect 8x4 alignment
 * 
 */
struct table_entry_t {
    char* word;
    size_t count1;
    size_t count2;
    struct table_entry_t* next;
};

static double geometric_mean(size_t a, size_t b) {
    return sqrt(a * b);
}

static double commonality(struct table_entry_t* entry) {
    return geometric_mean(entry->count1, entry->count2);
}

static struct table_entry_t* allocate_table_entry(void) {
    struct table_entry_t* entry = malloc(sizeof (struct table_entry_t));

    if (entry == NULL) {
        fprintf(stderr, "Memory allocation failure in allocate_table_entry()\n");
        exit(EXIT_FAILURE);
    }

    return entry;
}

static struct table_entry_t* create_table_entry(char* word) {
    struct table_entry_t* entry = allocate_table_entry();

    entry->count1 = 0;
    entry->count2 = 0;
    entry->word   = strdup(word);

    if (entry->word == NULL) {
        fprintf(stderr, "Memory allocation failure in create_table_entry->strdup(word)\n");
        exit(EXIT_FAILURE);
    }

    return entry;
}

static struct table_entry_t *hash_table[HASH_MODULUS];

static struct table_entry_t* lookup_word(char* word) {
    struct table_entry_t* entry = hash_table[hash(word)];

    while (entry) {
        if (strings_match(word, entry->word)) {
            return entry;
        }

        entry = entry->next;
    }

    return NULL;
}

static struct table_entry_t* add_word_to_table(char* word) {
    struct table_entry_t* entry = lookup_word(word);

    if (entry == NULL) {
        entry = create_table_entry(word);

        size_t hashval = hash(entry->word);
        entry->next = hash_table[hashval];
        hash_table[hashval] = entry;
    }

    if (first_file) {
        ++entry->count1;
    } else {
        ++entry->count2;
    }

    if (entry->count1 && entry->count2) {
        double entry_commonality_score = commonality(entry);

        if (entry_commonality_score > current_max) {
            current_max = entry_commonality_score;
            most_common_word = entry->word;
        }
    }

    return entry;
}

static char input_buffer[BUFSIZ];

int main(int argc, char *argv[])
{
    const char** filenames = parse_command_line_options(argc, argv);

    FILE* input_file = open_readonly_file(*filenames++);

    char* token_reentrancy_pointer = NULL;

    while (TRUE) {
        if (fgets(input_buffer, BUFSIZ, input_file) == NULL) {
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

    if (most_common_word) {
        printf("%s\n", most_common_word);
    }

    return EXIT_SUCCESS;
}
