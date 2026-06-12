
/* File: main.c
 * -----------------
 *  container of adding game.c and duck.c to run!!
 * 
 * 
 */

#include "uart.h"
#include "mymodule.h"
// #include "jump.h"
#include "game.h"
#include "duck.h"
#include "printf.h"

void mango_run(void);

void main(void) {
    uart_init();
    i2c_device_t* ducktest = accel_init();
    mango_run(); 
}
