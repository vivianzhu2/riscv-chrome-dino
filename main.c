#include "uart.h"
#include "mymodule.h"

void mango_run(void);

void main(void) {
    uart_init();
//    say_hello("CS107e");
    mango_run();
}
