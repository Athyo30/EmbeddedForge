#ifndef RINGBUF_H
#define RINGBUF_H
#include <stdint.h>
#include <stddef.h>
/* Single-producer (ISR) / single-consumer (main) byte ring buffer.
 * THIS is where your volatile / atomicity / memory-ordering model
 * gets tested for real: head is written by the ISR, tail by main
 * (or vice versa). Think about which fields cross the ISR/main
 * boundary and how they must be qualified. */
#define RB_CAP 64                      /* power of two on purpose */
typedef struct {
    uint8_t buf[RB_CAP]; //64 Bytes capacity in Ring Buffer

    /* TODO: declare head and tail. What qualifier do they need, and
     *       WHY? Write your one-line justification here:  Them being 4 byte ints is fine. Cause 32 bits is more than enough for 64 things.
    But do they have to be volatile? Volatile ensures that compiler doesnt optimize them away, ensures r/w from mem, not caching them. 
    So it would be necessary if like interrupt -> directly touches one of them in the memory directly. But if not volatile, wont fetch the updated version.
    */
    volatile uint32_t head; //Points to where can pop         
    volatile uint32_t tail; //Points to where can push
} ringbuf;
void    rb_init(ringbuf *rb);
int     rb_push(ringbuf *rb, uint8_t b);   /* returns 0 ok, -1 full  */
int     rb_pop(ringbuf *rb, uint8_t *out); /* returns 0 ok, -1 empty */
#endif
