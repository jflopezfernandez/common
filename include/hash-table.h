
#ifndef PROJECT_INCLUDES_HASH_TABLE_H
#define PROJECT_INCLUDES_HASH_TABLE_H

const char* most_common_shared_word(void);

/** Perfect 8x4 alignment
 * 
 */
struct table_entry_t {
    char* word;
    size_t count1;
    size_t count2;
    struct table_entry_t* next;
};

struct table_entry_t* add_word_to_table(char* word);

typedef unsigned long long int hash_t;

typedef hash_t (*hash_function)(const char*);

#endif // PROJECT_INCLUDES_HASH_TABLE_H
