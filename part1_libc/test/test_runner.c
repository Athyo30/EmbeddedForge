/* Minimal zero-dependency test harness. These tests DEFINE correct
 * behavior. Make them pass. They deliberately probe the edge cases
 * called out in the stub comments. */
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "../src/mem.h"
#include "../src/str.h"
#include "../src/alloc.h"

static int g_pass = 0, g_fail = 0;
#define CHECK(cond, msg) do {                                        \
        if (cond) { g_pass++; }                                      \
        else { g_fail++; printf("  FAIL: %s  (%s:%d)\n", msg, __FILE__, __LINE__); } \
    } while (0)

static void test_mem(void)
{
    printf("[mem]\n");
    char buf[16];

    memset(buf, 0xAA, sizeof buf);
    forge_memset(buf, 0x5C, 10);
    int ok = 1; for (int i = 0; i < 10; i++) ok &= (buf[i] == 0x5C);
    CHECK(ok, "memset fills n bytes");
    CHECK((unsigned char)buf[10] == 0xAA, "memset does not overrun");
    forge_memset(buf, 0, 0);                    /* n==0 must be a no-op */
    CHECK((unsigned char)buf[0] == 0x5C, "memset n==0 is a no-op");

    char src[] = "abcdefgh", dst[9] = {0};
    forge_memcpy(dst, src, 8);
    CHECK(memcmp(dst, src, 8) == 0, "memcpy copies n bytes");

    /* memmove overlap: shift a buffer forward by 2 */
    char ov[] = "0123456789";
    forge_memmove(ov + 2, ov, 5);               /* dst > src, overlap */
    CHECK(memcmp(ov, "0101234789", 10) == 0, "memmove handles dst>src overlap");
    char ov2[] = "0123456789";
    forge_memmove(ov2, ov2 + 2, 5);             /* dst < src, overlap */
    CHECK(memcmp(ov2, "2345656789", 10) == 0, "memmove handles dst<src overlap");

    unsigned char a[] = {1, 2, 0x80}, b[] = {1, 2, 0x10};
    CHECK(forge_memcmp(a, b, 3) > 0, "memcmp treats bytes as unsigned");
    CHECK(forge_memcmp(a, a, 3) == 0, "memcmp equal -> 0");
}

static void test_str(void)
{
    printf("[str]\n");
    CHECK(forge_strlen("") == 0, "strlen empty");
    CHECK(forge_strlen("hello") == 5, "strlen basic");

    char d[16];
    CHECK(forge_strcpy(d, "world") == d, "strcpy returns dst");
    CHECK(strcmp(d, "world") == 0, "strcpy content + terminator");

    /* strncpy: pad short strings with NULs */
    char p[6]; memset(p, 'X', 6);
    forge_strncpy(p, "ab", 6);
    CHECK(p[0]=='a' && p[1]=='b' && p[2]==0 && p[3]==0 && p[5]==0,
          "strncpy pads remainder with NUL");
    /* strncpy: no terminator when src fills n */
    char q[3] = {'Z','Z','Z'};
    forge_strncpy(q, "abc", 3);
    CHECK(q[0]=='a'&&q[1]=='b'&&q[2]=='c', "strncpy does not terminate when full");

    CHECK(forge_strcmp("abc","abc")==0, "strcmp equal");
    CHECK(forge_strcmp("abc","abd")<0,  "strcmp less");
    CHECK(forge_strcmp("abc","ab")>0,   "strcmp prefix is greater");
}

static char arena[4096];

static void test_bump(void)
{
    printf("[bump]\n");
    bump_init(arena, sizeof arena);
    void *p = bump_alloc(1);
    void *q = bump_alloc(1);
    CHECK(p != 0 && q != 0, "bump returns non-NULL");
    CHECK(((uintptr_t)p & 7u) == 0, "bump returns 8-byte-aligned");
    CHECK((char*)q - (char*)p >= 8, "bump aligns allocations apart");
    CHECK(bump_alloc(sizeof arena) == 0, "bump returns NULL when it can't fit");
    bump_reset();
    CHECK(bump_alloc(1) == p, "bump_reset rewinds");
}

static void test_freelist(void)
{
    printf("[freelist]\n");
    fl_init(arena, sizeof arena);
    void *a = fl_malloc(64);
    void *b = fl_malloc(64);
    void *c = fl_malloc(64);
    CHECK(a && b && c, "fl_malloc basic");
    CHECK(a != b && b != c, "fl_malloc hands out distinct blocks");

    if (a && b) {
        memset(a, 0x11, 64); memset(b, 0x22, 64);   /* no clobber across blocks */
        CHECK(*(unsigned char*)a == 0x11, "blocks don't overlap");
    } else {
        CHECK(0, "blocks don't overlap (skipped: malloc returned NULL)");
    }

    fl_free(b);
    void *b2 = fl_malloc(64);
    CHECK(b2 != 0, "freed block is reusable");

    /* coalescing: free everything, then a big alloc must succeed */
    fl_free(a); fl_free(b2); fl_free(c);
    void *big = fl_malloc(64 * 3);
    CHECK(big != 0, "adjacent frees coalesce (big alloc succeeds)");
}

int main(void)
{
    test_mem();
    test_str();
    test_bump();
    test_freelist();
    printf("\n%d passed, %d failed\n", g_pass, g_fail);
    return g_fail ? 1 : 0;
}
