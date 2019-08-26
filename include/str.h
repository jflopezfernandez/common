
#ifndef PROJECT_INCLUDES_STR_H
#define PROJECT_INCLUDES_STR_H

/** strings_match
 * 
 *  This is a more intuitive looking version of the standard library's 'strcmp'
 *  function. 'strcmp' returns the lexicographic sort difference between its two
 *  input strings, which means a match is indicated via a return value of zero.
 *  This has caused a lot of confusion in the past because both the compiler and
 *  programmer using the function interpret a zero-valued expression as 'false'.
 * 
 *  This function simply returns the negation of that call, and it probably
 *  helps that the name of the function is also more immediately intuitive than
 *  'strcmp'.
 * 
 */
int strings_match(const char* a, const char* b);

#ifndef NUL
/** This is a more semantically intuitive synonym for the null character, 
 *  represented by a decimal value of 0 or an ASCII value of '\0'.
 * 
 */
#define NUL '\0'
#else
#error "NUL is already defined."
#endif

#endif // PROJECT_INCLUDES_STR_H
