
#include "common.h"

typedef unsigned long long int hash_t;

typedef hash_t (*hash_function)(const char*);

#ifndef HASH_MODULUS
#define HASH_MODULUS (52379)
#else
#error "HASH_MODULUS already defined."
#endif // HASH_MODULUS

/** The most basic hash function known to man. Used literally just for getting
 *  the prototype going. It is marked with the 'unused' attribute because I
 *  haven't implemented the ability to switch hash functions after compilation,
 *  and the weinberg hash function has proven much more robust, with a
 *  negligible amount of hash collisions during testing.
 * 
 */
__attribute__((hot, nonnull(1), unused))
static hash_t trivial_hash(const char* str) {
    hash_t hash = 0;

    while (*str) {
        hash = (*str++) + 211 * hash;
    }

    return hash % HASH_MODULUS;
}

/** Professor Robert Sedgewick's universal hash function for string keys, from
 *  Algorithms in C page 579. I elided the length argument, as it isn't
 *  necessary to iterate over the string to hash.
 * 
 *  This function is also marked 'unused' to indicate that I have not yet
 *  implemented the functionality to switch hash functions after compilation.
 * 
 */
__attribute__((nonnull(1), unused))
static hash_t basic_hash(const char* str) {
    hash_t hash = 0;
    hash_t    a = 63689;
    hash_t    b = 378551;

    while (*str) {
        hash = (*str++) + a * hash;
        a = a * b;
    }

    return hash % HASH_MODULUS;
}

/** Hashing algorithm developed by Dr. Peter Weinberger and discussed at length
 *  in the dragon book.
 * 
 */
__attribute__((hot, nonnull(1)))
static hash_t weinberger_hash(const char* str) {
    hash_t hash = 0;
    hash_t bits = 8 * sizeof (hash_t);
    hash_t three_fourths = (bits * 3) / 4;
    hash_t one_eighth = bits / 8;
    hash_t high_bits = 0xffffffff << (bits - one_eighth);

    while (*str) {
        hash_t test = 0;
        hash = (hash << one_eighth) + (*str++);

        if ((test = hash & high_bits) != 0) {
            hash = ((hash ^ (test >> three_fourths)) & (~high_bits));
        }
    }

    return hash % HASH_MODULUS;
}

/** This function pointer determines the hashing algorithm to use for the
 *  table. Crucially, there is no mechanism in place right now to modify the
 *  algorithm at runtime, nor is there a way to rebuild the table as is
 *  necessary when changing hash functions once the table already exists. This
 *  will have to be implemented before dynamic algorithm selection can occur.
 * 
 */
static hash_function calculate_hash = weinberger_hash;

static double current_max = 0.0;

static char* most_common_word = NULL;

/** This function returns the most common word shared by the two input files.
 *  The most_common_word variable is initially zero, so if there is no common
 *  word, maybe because the two input files are empty, the return value will be
 *  a NULL pointer. 
 * 
 */
const char* most_common_shared_word(void) {
    return most_common_word;
}

/** This is the hash table for the strings in the input files. The hash table
 *  employs lock-based synchronization in the form of reader-writer locks to
 *  ensure data coherence in spite of being manipulated by multiple threads
 *  concurrently.
 * 
 *  Each entry in the hash table has its own reader-writer lock to allow for
 *  more granular control over memory resources.
 * 
 */
static struct table_entry_t *hash_table[HASH_MODULUS];

/** This is the lock-based synchronization tool to ensure data coherence within
 *  the hash table. The benefit of employing a reader-writer lock instead of a
 *  mutex is that multiple threads may hold a lock in read mode, while a thread
 *  requiring write-access enjoys the same semantics as a mutex. This reduces
 *  the computational cost of two threads accessing the same hash table entry,
 *  which is great because reading the hash table to see if a string exists is
 *  one of the most common actions in the application.
 * 
 */
static pthread_rwlock_t hash_table_lock = PTHREAD_RWLOCK_INITIALIZER;

__attribute__((hot, returns_nonnull))
static struct table_entry_t* allocate_table_entry(void) {
    struct table_entry_t* entry = malloc(sizeof (struct table_entry_t));

    if (entry == NULL) {
        fatal_error("Memory allocation failure in allocate_table_entry()");
    }

    return entry;
}

__attribute__((nonnull(1), returns_nonnull))
static struct table_entry_t* create_table_entry(const char* word) {
    struct table_entry_t* entry = allocate_table_entry();

    entry->count1 = 0;
    entry->count2 = 0;

    entry->word   = strdup(word);

    if (entry->word == NULL) {
        fatal_error("Memory allocation failure in create_table_entry->strdup(word)");
    }

    if (pthread_rwlock_init(&entry->lock, NULL)) {
        fatal_error("Failed to dynamically initialize entry mutex");
    }

    entry->next = NULL;

    return entry;
}

/** This function calculates the geometric mean of two real numbers in the
 *  expected manner: it multiplies the two numbers together and takes the
 *  square root.
 * 
 *  This function is used to calculate what I called the 'commonality' of the
 *  strings in the input files. To differentiate between many repeated
 *  appearances of a string in one file while barely any in the other, the
 *  geometric mean will score more favorably by a string that has about equal
 *  representation in both files, rather than one over the other.
 * 
 */
__attribute__((hot, pure, unused))
static double geometric_mean(double a, double b) {
    return sqrt(a * b);
}

/** This function calculates the harmonic mean of two real numbers in the
 *  expected manner: it multiplies the two numbers together and takes the
 *  square root.
 * 
 *  This function is also used to calculate the commonality of the strings in
 *  the input files, with the difference being that a higher priority is given
 *  to strings that appeared in high amounts in both files, rather than
 *  a lot in one file and maybe once in the other, which is a weakness with the
 *  geometric mean metric.
 * 
 */
__attribute((hot, pure))
static double harmonic_mean(double a, double b) {
    return 2.0 / ((1.0 / a) + (1.0 / b));
}

/** This function implicitly casts the entry parameters to real numbers and
 *  calls the geometric_mean function to calculate the commonality score for
 *  the given string.
 * 
 */
__attribute__((hot, nonnull(1)))
static inline double commonality(struct table_entry_t* entry, double (*metric_function)(double,double)) {
    return metric_function((double) entry->count1, (double) entry->count2);
}

/** This function takes care of hashing the current string and returning the
 *  address the entry is supposed to be in. This function takes care of
 *  resolving hash collisions by iterating through the linked list of entries
 *  at the hash value in question until either a free spot is found or the
 *  string matches the string in one of the linked list entries. This latter
 *  case means we should not allocate a new entry; we simply need to increment
 *  the that entry's reference count.
 * 
 */
__attribute__((nonnull(1)))
static struct table_entry_t* lookup_word(const char* word) {
    struct table_entry_t* entry = hash_table[calculate_hash(word)];

    pthread_rwlock_rdlock(&hash_table_lock);

    while (entry) {
        if (strings_match(word, entry->word)) {
            break;
        }

        entry = entry->next;
    }

    pthread_rwlock_unlock(&hash_table_lock);

    return entry;
}

/** Each entry in the hash table has a reader-writer lock for data coherence.
 *  The benefit of the reader-writer lock over a simple mutex is that multiple
 *  threads can hold a read lock, minimizing the need for write locks, as they
 *  are the real bottleneck of the operation.
 * 
 */
static inline void increment_reference_count(struct table_entry_t* entry, int file) {
    pthread_rwlock_wrlock(&entry->lock);

    if (file == 1) {
        ++entry->count1;
    } else if (file == 2) {
        ++entry->count2;
    } else {
        fatal_error("Invalid file number");
    }

    pthread_rwlock_unlock(&entry->lock);
}

/** The commonality score calculation has been modified to accept a function
 *  pointer, allowing for the customization of program operation down the line.
 *  I'm not actively working on this at the moment, but it is a possibility.
 * 
 *  The formula has been changed to the harmonic mean of the string counts in
 *  the input files rather than the geometric mean. I initially thought
 *  filtering zero values from the calculation would be much more involved, but
 *  I simply set the calculation to be contingent on nonzero counts in both
 *  input files, and the division by zero problem has been completely avoided.
 * 
 */
static inline void calculate_commonality_score(struct table_entry_t* entry) {
    double entry_commonality_score = commonality(entry, harmonic_mean);

    if (entry_commonality_score > current_max) {
        current_max = entry_commonality_score;
        most_common_word = entry->word;
    }
}

/** This is where most of the magic happens. The bulk of the application is
 *  building the hash table which will result in us being able to give the user
 *  an answer when the application has finished executing. This function takes
 *  care of adding new entries to the hash table as we encounter them, as well
 *  as incrementing reference counts if the given entry already exists in the
 *  hash table. The file integer is what allows for the distinction between
 *  files 1 and 2.
 * 
 */
__attribute__((nonnull(1), returns_nonnull))
struct table_entry_t* add_word_to_table(const char* word, int file) {
    /** Look up the word in the table by passing in the 'word' string to the
     *  lookup_word function. 'lookup_word' handles hashing the word and
     *  determining whether the entry is in the hash table. If it isn't, the
     *  return value will be a NULL pointer.
     * 
     */
    struct table_entry_t* entry = lookup_word(word);

    /** Having determined that the entry is not already in the hash table, we
     *  must add it now. The create_table_entry takes care of allocating an
     *  entry struct, deep-copying in the word string, and return the pointer
     *  to the entry.
     * 
     *  Prior to modifying the hash table pointers, we lock the hash_table_lock
     *  mutex to prevent modifications to the same memory by two different
     *  threads, corrupting the data.
     * 
     */
    if (entry == NULL) {
        entry = create_table_entry(word);

        pthread_rwlock_wrlock(&hash_table_lock);
        size_t hashval = calculate_hash(entry->word);
        entry->next = hash_table[hashval];
        hash_table[hashval] = entry;
        pthread_rwlock_unlock(&hash_table_lock);
    }

    increment_reference_count(entry, file);

    calculate_commonality_score(entry);

    return entry;
}

void release_table_resources(void) {
    for (size_t i = 0; i < HASH_MODULUS; ++i) {
        struct table_entry_t* entry = hash_table[i];

        while (entry) {
            struct table_entry_t* next = NULL;

            if (entry->next) {
                next = entry->next;
            }

            pthread_rwlock_destroy(&entry->lock);

            FREE(entry->word);
            FREE(entry);

            entry = next;
        }
    }
}

#if defined(HASH_MODULUS)
#undef HASH_MODULUS
#endif
