#include "mem.h"

/* ============================================================
 * DISCIPLINE (do this for EVERY function, in the comment block):
 *   1. State the contract in one line: what does it return, what
 *      does it mutate?
 *   2. List the edge cases BEFORE you write code (n==0, overlap,
 *      NULL, alignment).
 *   3. Write the simple version first. Do not reach for word-sized
 *      copies or duff's device until the byte version passes.
 *   4. After it compiles, predict which test will fail and why.
 * ============================================================ */

/* Contract: set n bytes at dst to (unsigned char)c; return dst.
 * Edge cases you must handle: n==0.
 * Approach:
 * So supposed to use only the first byte of int and copy it over n times at dst. Need to cast things to char, char* before doing stuff. 
 * Time / space:   O(n)                              */
void *forge_memset(void *dst, int c, size_t n)
{
    unsigned char byte_c = (unsigned char)c; 
    unsigned char* dst_ptr = (unsigned char*) dst;
    for(size_t i = 0; i < n; i++)
    {
        dst_ptr[i] =  byte_c;       
    }
    return dst;
}

/* Contract: copy n bytes src->dst (NO overlap allowed); return dst.
 * Edge cases: n==0.
 * Approach: Similar to memset approach, looping and copying over. No overlap allowed, is interesting.  
 * Time / space:    O(n)                                            */
void *forge_memcpy(void *dst, const void *src, size_t n)
{
    unsigned char* dst_ptr = (unsigned char*) dst;
    const unsigned char* src_ptr = (const unsigned char*) src;
    for(size_t i = 0; i < n; i++)
    {
        dst_ptr[i] =  *src_ptr;
        src_ptr++;      
    }
    return dst;
}

/* Contract: copy n bytes src->dst CORRECTLY EVEN IF THEY OVERLAP;
 *           return dst. This is the whole point of memmove.
 * Edge cases: dst<src, dst>src, dst==src, n==0.
 * Hint: which direction do you copy when dst > src? Reason it out
 *       on paper with a 4-byte buffer before coding.
 * Approach: If dst>src, write backward. If dst < src then write fwd. 
 * Time / space: Still O(n)                                             */
void *forge_memmove(void *dst, const void *src, size_t n)
{
    unsigned char* dst_ptr = (unsigned char*) dst;
    const unsigned char* src_ptr = (const unsigned char*) src;

    if(dst_ptr >= src_ptr)
    {
        //cause even if its equal then either doesnt matter
        for(int i = n-1; i >=0; i--)
        {
            dst_ptr[i] = src_ptr[i];
        }
    }
    else
    {
        for(size_t i = 0; i < n; i++)
        {
            dst_ptr[i] =  src_ptr[i];
        }
    }

    return dst;

}

/* Contract: compare n bytes. Return <0, 0, >0 like standard memcmp.
 * Gotcha: comparison is on UNSIGNED chars. A naive signed-char
 *         version is the classic bug here — the test will catch it.
 * Approach: Why is signed char a bug? Is it somn to do with how the MSB is treated? Yes, 255 will be considered as negative in signed char.
 * Will compare through a looping type byte comparision. If same: 0, if first a smaller -> return neg. a bigger -> return pos. 
 * Time / space:                                                 */
int forge_memcmp(const void *a, const void *b, size_t n)
{
    unsigned char* a_ptr = (unsigned char*) a;
    unsigned char* b_ptr = (unsigned char*) b;

    for(size_t i = 0; i < n ; i++)
    {
        if(a_ptr[i] != b_ptr[i])
        {
            return a_ptr[i] - b_ptr[i];
        }
    }
    return 0;
}
