#include <stdint.h>
#include "uart.h"
/* Echo server: whatever you type in the QEMU console comes back
 * uppercased — proving bytes flow ISR -> ring -> main. Type, and if
 * your ring/ISR are correct you'll see echoes. Ctrl-A then x to quit. */
int main(void){
    uart_init();
    uart_puts("\r\npart3: type; I echo (uppercased). Ctrl-A x quits.\r\n");
    for(;;){
        uint8_t c;
        if (uart_getc(&c) == 0){
            if (c>='a' && c<='z') c -= 32;
            uart_putc((char)c);
        }
    }
}
