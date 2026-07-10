#include <stdint.h>
#include "uart.h"
#include "ringbuf.h"

/* lm3s6965evb UART0 register map (offsets from base). */
#define UART0        0x4000C000u
#define UART_DR   (*(volatile uint32_t*)(UART0+0x00))
#define UART_FR   (*(volatile uint32_t*)(UART0+0x18)) //UART FLAG REGISTER
#define UART_IM   (*(volatile uint32_t*)(UART0+0x38))  /* interrupt mask   */
#define UART_MIS  (*(volatile uint32_t*)(UART0+0x40))  /* masked int status*/
#define UART_ICR  (*(volatile uint32_t*)(UART0+0x44))  /* interrupt clear  */
#define FR_TXFF   (1u<<5) //Transmit FIFO Full flag. This bit is set to 1 when the hardware transmit buffer is completely full.
#define FR_RXFE   (1u<<4)   /* receive FIFO empty */
#define INT_RX    (1u<<4)   /* RX interrupt bit in IM/MIS/ICR */
/* Cortex-M NVIC: enable IRQ n by setting bit n in ISER0. UART0 = IRQ 5. */
#define NVIC_ISER0 (*(volatile uint32_t*)0xE000E100u)
#define UART0_IRQ  5

static ringbuf rx;

/* PROVIDED: TX is polled (you already wrote this style in Part 2). */
void uart_putc(char c){ 
    while(UART_FR&FR_TXFF){
        //do nothing. Wait till buffer has space. 
    } 
    UART_DR=(uint32_t)c; //writing 4 Bytes into Data Register 
}
void uart_puts(const char*s){ 
    while(*s) uart_putc(*s++); //Here, we are passing each byte, char from a string of characters. 
}

/* EXERCISE 1: initialise RX. Enable the UART RX interrupt (UART_IM),
 * then enable UART0 in the NVIC (NVIC_ISER0). rb_init the ring. */
void uart_init(void){
    rb_init(&rx);
    UART_IM |= INT_RX;
    NVIC_ISER0 |= (1u << UART0_IRQ);
}

/* EXERCISE 2: the ISR. Runs when a byte arrives. You must:
 *   - read the byte from UART_DR
 *   - push it into the ring
 *   - clear the RX interrupt (write INT_RX to UART_ICR)
 * Keep it SHORT. No printing, no blocking inside an ISR. */
void UART0_Handler(void){
    uint8_t uart_byte = UART_DR & 0xFF;
    rb_push(&rx, uart_byte);
    UART_ICR |= INT_RX;
}

/* EXERCISE 3: consumer side, called from main. Just pops the ring. */
int uart_getc(uint8_t *out){ 
    return rb_pop(&rx, out); 
}
