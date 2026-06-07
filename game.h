#ifndef GAME_H
#define GAME_H

#include "fb.h"
#include "gl.h"
#include "keyboard.h"
#include "ps2.h"
#include "ringbuffer.h"
#include "timer.h"
#include "interrupts.h"
#include "malloc.h"
#include "strings.h"
#include <stddef.h>
#include <stdbool.h>
#include "jump.h"

/*
 * Author: Vivian Zhu Nancy Zou
 */


void mango_run(void);

// void jumpDetected(i2c_device_t i2cdev, unsigned int dur); //   configure the display by setting the appropriate refresh delay and display duration for the display, and ensuring that all necessary pins are set to output mode

// void set_refresh_delay(unsigned int rdelay); // set the refresh delay for the display

// unsigned int get_refresh_delay(void); // get the refresh delay for the display

// void set_display_duration(unsigned int dur); // set the display duration for the display

// unsigned int get_display_duration(void); // get the display duration for the display

// void display(unsigned int num); //refresh 4-digit value on the display by setting the appropriate segments to high or low based on the digit values

#endif
