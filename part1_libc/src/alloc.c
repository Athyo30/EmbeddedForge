#include "alloc.h"
#include <stdint.h>

/* ==================================================================
 * STAGE A — BUMP ALLOCATOR
 * ------------------------------------------------------------------
 * State to keep: base pointer, current offset, total size. Static
 * file-scope variables are fine here.
 * Alignment: round every allocation size UP to a multiple of 8 so
 * returned pointers stay 8-byte aligned. The bit-twiddle for that is
 *     (n + 7) & ~(size_t)7
 * — but derive WHY that works before you paste it.
 * ================================================================== */

/* TODO: your static state here */

static void* base_ptr = (void*) 0x10000000; //Some base address from which the block starts
size_t curr_offset = 0; //Offset from base address pointing to next free. Why need both base ptr and curr offset? If only curr offset enough to alloc/dealloc? Is it cause base used to clear the whole block?

static size_t total_cap = 0; //Total capacity/size that can be allocated. Set in bump_init

void bump_init(void *arena, size_t size)
{
    /* TODO */
    base_ptr = arena; //arena is the init address
    total_cap = size; 
}

void *bump_alloc(size_t size)
{

    //This fn will return a void pointer. Cause any kinda data type can be pointing to this memory.
    /* TODO: align size up; if it fits, return current base+offset and
     *       advance offset; else return NULL. */

    // A 32-bit address like 0x1000 0018 is still just a number, and "8-byte aligned" just means that number is divisible by 8. It has nothing to do with how wide the pointer is.

    //To return the pointer to the next free after allocating size amount of bytes from
    
    //8 byte aligning:  Adding n-1 (i.e. +7) before masking pushes any value that isn't already a multiple of 8 up into the next block, so the subsequent round-down lands exactly on the ceiling
    // void* new_sizing = (size + 7) & ~(size_t)7; //this is zeroing out the bottom 3 bits. Why the +7 first tho?

    size_t new_size = (size + 7) & ~(size_t)7;

    if((new_size + curr_offset > total_cap))
    {
        return NULL;
    }
    else{
        void* new_addr = (void*) (base_ptr + curr_offset);
        curr_offset += new_size; //moving ahead by new_size
        return new_addr;
    }
}

void bump_reset(void)
{
    curr_offset = 0;
}

/* ==================================================================
 * STAGE B — FREE-LIST ALLOCATOR
 * ------------------------------------------------------------------
 * Suggested design (you may improve it — but get the SIMPLE version
 * working and tested first; do not over-build):
 *
 *   Each block has a header stored just before the user pointer:
 *       struct header { size_t size; int free; struct header *next; };
 *   Free list threads through the 'next' fields.
 *
 *   fl_malloc: first-fit scan; if the found block is much bigger than
 *              needed, SPLIT it. Return header+1 to the user.
 *   fl_free:   mark free; coalesce with the physically-next block if
 *              it is also free. (Coalescing with the previous block is
 *              a stretch goal — skip it in v1.)
 *
 * Pitfalls the tests will probe: returning a pointer that isn't past
 * the header; forgetting to split (you'll run out of memory early);
 * forgetting to coalesce (fragmentation — a big alloc after many
 * free/alloc cycles will fail).
 * ================================================================== */



typedef struct header { 
    size_t size; //this size refers to how many bytes does this block hold currently.
    int free; //Its a 1 or a 0. 1: free, 0: allocated
    struct header *next; //Points to the next block's header
}header;

#define MIN_SPACE 50

static header* head; //static so that malloc and free can access it too? but thats global. Why static?

void fl_init(void *arena, size_t size)
{
    /* TODO: place one big free block spanning the arena */
    
    head = (header*) arena;
    head->size = size - sizeof(struct header); //cause currently, everything is its payload
    head->free = 1; //the payload is free rn, nothing been allocated
    head->next = NULL;
}

void *fl_malloc(size_t size)
{
    /* TODO */
    //need to go through the list starting from the global head, and then check if free space is there. Return the pointer to the next space.
    //Should return the address of the new PAYLOAD. So past the header.
    header* ptr = head;

    while(ptr != NULL)
    {
        if(ptr->free)
        {
            if(ptr->size >= size)
            {
                //Available size is more than whats requested
                if(ptr->size >= (size + sizeof(struct header) + MIN_SPACE))
                {
                    //Splitting case cause lots of space left
                    ptr->free = 0;
                    header* post_header = (ptr+1);
                    header* new_head = (header*)((char*) post_header + size); //moving past the payload of allocated block
                    new_head->free = 1;
                    new_head->size = ptr->size - (size + sizeof(struct header));
                    header* old_next = ptr->next;
                    new_head->next = old_next; //NEED to point to wtv ptr was pointing to earlier. 
                    ptr->next = new_head;
                    ptr->size = size; //updated size, cause now its just the payload. NOT THE ENTIRE BLOCK REMAINING***
                    return ptr+1; //returning addr of the payload, one past the header.
                }
                else
                {
                    ptr->free = 0;
                    return ptr + 1; //moving one header ahead
                }
            }
            else
            {
                //No Space available here, move fwd!
                ptr = ptr->next;
            }
        }
        else{
            //Not free, move fwd!
            ptr = ptr->next;
        }


    }
    return NULL;
}

void fl_free(void *ptr)
{
    /* TODO */
}
