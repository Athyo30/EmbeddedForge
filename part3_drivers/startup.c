#include <stdint.h>
/* Provided complete for Part 3 (you built this in Part 2). Note the
 * vector table now routes IRQ 5 (UART0) to UART0_Handler. */
extern uint32_t _sidata,_sdata,_edata,_sbss,_ebss,_estack;
int main(void);
void Reset_Handler(void);
void UART0_Handler(void);              /* you implement in uart.c */
void Default_Handler(void){for(;;){}}
__attribute__((section(".isr_vector")))
void (* const vtab[])(void)={
  (void(*)(void))(&_estack),/*0 MSP*/ Reset_Handler,/*1*/
  Default_Handler,/*2 NMI*/ Default_Handler,/*3 HardFault*/
  Default_Handler,Default_Handler,Default_Handler,Default_Handler,   /*4-7*/
  Default_Handler,Default_Handler,Default_Handler,Default_Handler,   /*8-11*/
  Default_Handler,Default_Handler,Default_Handler,Default_Handler,   /*12-15*/
  Default_Handler,                 /* IRQ0  (vec 16) */
  Default_Handler,                 /* IRQ1 */
  Default_Handler,                 /* IRQ2 */
  Default_Handler,                 /* IRQ3 */
  Default_Handler,                 /* IRQ4 */
  UART0_Handler                    /* IRQ5  (vec 21) = UART0 */
};
void Reset_Handler(void){
  uint32_t *s=&_sidata,*d=&_sdata; while(d<&_edata)*d++=*s++;
  for(d=&_sbss; d<&_ebss;) *d++=0;
  main();
  for(;;){}
}
