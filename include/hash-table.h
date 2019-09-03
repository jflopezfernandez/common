
#ifndef PROJECT_INCLUDES_HASH_TABLE_H
#define PROJECT_INCLUDES_HASH_TABLE_H

/** This is the function that our application will query last once everything
 *  has finished to determine whether a common string between the two input
 *  files does exist, and if it does, what it is. The purpose of this function
 *  is to add some modicum of encapsulation and allow the 'most_common_word'
 *  variable to have internal linkage within the hash-table implementation
 *  file.
 * 
 */
const char volatile* most_common_shared_word(void);

/** This is the struct that represents each hash table entry in memory. The
 *  reason for the separate reference counts for files 1 and 2 is because I
 *  defined the "most common shared string" as being the string with the
 *  highest geometric mean based on the two datapoints of its reference count
 *  in file one and its reference count in file two.
 * 
 */
struct table_entry_t {
    char* word;
    size_t count1;
    size_t count2;
    pthread_rwlock_t lock;
    struct table_entry_t* next;
};

/** This is where most of the magic happens. The bulk of the application is
 *  building the hash table which will result in us being able to give the user
 *  an answer when the application has finished executing. This function takes
 *  care of adding new entries to the hash table as we encounter them, as well
 *  as incrementing reference counts if the given entry already exists in the
 *  hash table. The file integer is what allows for the distinction between
 *  files 1 and 2.
 * 
 */
__attribute__((hot, nonnull(1), returns_nonnull))
struct table_entry_t* add_word_to_table(const char* word, int file);

void release_table_resources(void);

#endif // PROJECT_INCLUDES_HASH_TABLE_H
