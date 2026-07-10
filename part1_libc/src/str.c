#include "str.h"

/* Contract: return number of bytes before the NUL. NUL not counted.
 * Approach: Whats Nul? same as NULL?? 
 * Time / space:  O(n)                                             */
size_t forge_strlen(const char *s)
{
    size_t count = 0;
    const char* s_ptr = s;
    while(*s_ptr!= '\0')
    {
        count++;
        s_ptr++;
    }
    return count;
}

/* Contract: copy src (incl. NUL) to dst; return dst.
 * Trap: this is the "trust the simple solution" one. It is ~3 lines.
 *       Resist adding bounds you weren't asked for.
 * Approach:  O(n), just moving bytes and overwriting.                                               */
char *forge_strcpy(char *dst, const char *src)
{
    const char* src_ptr = src;
    char* dst_ptr = dst;
    while(*src_ptr!= '\0')
    {
        *dst_ptr = *src_ptr;
        src_ptr++;
        dst_ptr++;
    }
    //To copy the Nul char.
    *dst_ptr = *src_ptr;
    return dst;
}

/* Contract (read carefully — strncpy is weird by design):
 *   - copy at most n bytes from src.
 *   - if src is shorter than n, PAD the remainder of dst with '\0'.
 *   - if src is >= n bytes, dst is NOT nul-terminated.
 * The padding + no-terminator rule is what the test checks. Look it
 * up if unsure, then state the rule in your own words here:
 * Rule:
 * Approach: Was earlier thinking within the loop, will check if nul hit, then its shorter. But causing issues with filling the rest with \0
 * I think better to check length prior to looping.                                      */
char *forge_strncpy(char *dst, const char *src, size_t n)
{
    char* dst_ptr = dst;

    size_t src_len = forge_strlen(src);
    if(src_len >= n)
    {
        //src bigger
        for(size_t i = 0; i<n; i++){
            dst_ptr[i] = src[i];
        }
    }
    else
    {
        //src smaller
        for(size_t i = 0; i<src_len; i++){
            dst_ptr[i] = src[i];
        }
        for(size_t i = src_len; i < n; i++)
        {
            //padding
            dst_ptr[i] = '\0';
        }
    }
    return dst;
}

/* Contract: return <0/0/>0. Again: UNSIGNED char comparison.
 * Approach:                                                     */
int forge_strcmp(const char *a, const char *b)
{
    const unsigned char* a_ptr = (unsigned char*) a;
    const unsigned char* b_ptr = (unsigned char*) b;

    while (*a_ptr != '\0' && *a_ptr == *b_ptr) {
        //keeping looping till a hasnt reached end, and both are equal. 
        a_ptr++;
        b_ptr++;
    }
    return *a_ptr - *b_ptr;
}
