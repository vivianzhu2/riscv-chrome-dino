#include "uart.h"
#include "mymodule.h"
#include "game.h"

void main(void) {
    uart_init();
//    say_hello("CS107e");
    mango_run();
//    uart_putstring("hello\n");
}
