#ifndef JUMP_H
#define JUMP_H

#include "i2c.h"
/*
 * Functions for a re-usable module for 7-segment display
 *
 * assignment 2 extension
 *
 * Author: Vivian Zhu
 */

//  design and implement re-usable module for 7-segment display
// encapsulates inner workings of refresh, how digits mapt to segments, etc
// make into tidy package that offers clean interface for client to config display and refresh 4-digit value
// clock client app becomes simple loop that asks 7-segment to refresh current value





i2c_device_t* accel_init(void); // initialize the display module

int jumpDetected(i2c_device_t* i2cdev); //   configure the display by setting the appropriate refresh delay and display duration for the display, and ensuring that all necessary pins are set to output mode

// void set_refresh_delay(unsigned int rdelay); // set the refresh delay for the display

// unsigned int get_refresh_delay(void); // get the refresh delay for the display

// void set_display_duration(unsigned int dur); // set the display duration for the display

// unsigned int get_display_duration(void); // get the display duration for the display

// void display(unsigned int num); //refresh 4-digit value on the display by setting the appropriate segments to high or low based on the digit values

#endif
