
#include "common.h"

#ifndef HASH_MODULUS
#define HASH_MODULUS (52379)
#else
#error "HASH_MODULUS already defined."
#endif // HASH_MODULUS

/** The most basic hash function known to man. Used literally just for getting
 *  the prototype going.
 * 
 */
static size_t __attribute__((unused)) hash(const char* str) {
    size_t h = 0;

    while (*str) {
        h = (*str++) + 211 * h;
    }

    return h % HASH_MODULUS;
}

/** Professor Robert Sedgewick's universal hash function for string keys, from
 *  Algorithms in C page 579. I elided the length argument, as it isn't
 *  necessary to iterate over the string to hash.
 * 
 */
static hash_t __attribute__((unused)) basic_hash(const char* str) {
    int h = 0x0000;
    int a = 0x7ab7;
    int b = 0x6a2f;

    while (*str) {
        h = (a * h + *str++);
        a = (a * b);
    }

    return h % HASH_MODULUS;
}

/** Hashing algorithm developed by Dr. Peter Weinberger and discussed at length
 *  in the dragon book.
 * 
 */
static hash_t __attribute__((unused)) weinberger_hash(const char* str) {
    hash_t hash = 0x00;
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

const char* most_common_shared_word(void) {
    return most_common_word;
}

static struct table_entry_t *hash_table[HASH_MODULUS];

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

static struct table_entry_t* lookup_word(char* word) {
    struct table_entry_t* entry = hash_table[calculate_hash(word)];

    while (entry) {
        if (strings_match(word, entry->word)) {
            return entry;
        }

        entry = entry->next;
    }

    return NULL;
}

struct table_entry_t* add_word_to_table(char* word) {
    struct table_entry_t* entry = lookup_word(word);

    if (entry == NULL) {
        entry = create_table_entry(word);

        size_t hashval = calculate_hash(entry->word);
        entry->next = hash_table[hashval];
        hash_table[hashval] = entry;
    }

    if (processing_first_file()) {
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

#if defined(HASH_MODULUS)
#undef HASH_MODULUS
#endif
