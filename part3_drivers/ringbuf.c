#include "ringbuf.h"
/* EXERCISE. Keep it lock-free for the SP/SC case — you do NOT need a
 * mutex here, and reaching for one is the over-engineering trap. The
 * mask trick (idx & (RB_CAP-1)) works because RB_CAP is a power of 2;
 * derive why. State your approach before coding:
 *   push approach: Check full using mask trick, cause when say 64-1 => 63, then index also 63 means last one? Na Idts this is ther eason or use.
 *   pop approach:
 *   why is this safe with one producer + one consumer and no lock? */
void rb_init(ringbuf *rb){ 
    rb->head = 0;
    rb->tail = 0;
    // rb->size = 0;
    return;
}
int rb_push(ringbuf *rb, uint8_t b){ 
    //Check if full
    // if(rb->size == RB_CAP) return -1;
    //THIS WILL SACRIFICE ONE SLOT AT THE END. Currently, tail is at last, so its empty at last. But we are checking what will happen if we add to the last free slot -> Tail will go back to head and buffer will be full.
    if(((rb->tail + 1) & (RB_CAP - 1)) == rb->head)
    {
        //If its going to circle back to the head, its full
        return -1;
    }

    //If not:
    rb->buf[rb->tail] = b;
    // rb->tail = (rb->tail + 1) % RB_CAP; //SLOWER VERSION
    rb->tail = (rb->tail + 1) & (RB_CAP - 1); //Cause RB_CAP - 1 is basically all bits 1's. So when head goes to up until 63 -> it remains that. When it goes to 64-> then all 1s anded => zeroes it.
    // rb->size++;
    return 0;
}
int rb_pop(ringbuf *rb, uint8_t *out){ 
    //Check if empty
    // if(rb->size == 0) return -1;
    if(rb->head == rb->tail) return -1;
    //If not:
    *out = rb->buf[rb->head];
    // rb->head = (rb->head + 1) % RB_CAP; //SLOWER VERSION
    rb->head = (rb->head + 1) & (RB_CAP - 1); //Cause RB_CAP - 1 is basically all bits 1's. So when head goes to up until 63 -> it remains that. When it goes to 64-> then all 1s anded => zeroes it.
    // rb->size--;
    return 0;
}
