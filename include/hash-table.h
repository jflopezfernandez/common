
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

__attribute__((hot, nonnull(1), returns_nonnull))
struct table_entry_t* add_word_to_table(const char* word);

#endif // PROJECT_INCLUDES_HASH_TABLE_H
