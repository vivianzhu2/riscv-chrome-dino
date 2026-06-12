#ifndef DUCK_H
#define DUCK_H

#include "i2c.h"
/*
 * Functions for the jumping or ducking module!! 
 * * duck.c initializes the accelerometer and then reads in the data
 * It then checks if the data is a peak and within a specific threshold to return
 * whether the action is a jump or a squat or neither. 
 * 
 * Author: Vivian Zhu
 */


/*
 * `accelDuck_init: initializes the accelerometer MPU
 *
 * confirms the accelerometer is running and instantiates new device with its frequency and written registers.
 *
 * 
 * code inspired by / courtesy of  https://github.com/hasanunlu/i2c_demo_for_HiFive1/blob/master/i2c_demo/i2c_demo.c and the i2c driver
 * @return     i2c_device_t with the accelerometer properties
 */
i2c_device_t* accelDuck_init(void); // initialize the display module


/*
 * `duckDetected: checks if the data returns a jump or a squat or neither
 *
 * reads in three points: previous, current , next.
 * first updates values of prev, curr, and next by shifting curr to prev, next to curr.
 * 
 * reads the x,y,z acceleration and send to next
 * calculates the magnitude of next
 * 
 * checks if curr is a peak
 * checks if magnitude of curr is within threshold of either jump or squat
 * checks if the time between each jump is enough so it doesnt double count
 *     this is because a jump registers as two high peaks (one for pushing off the floor and one for landing)
 *
 * 
 * @return    integer that if 0: nothing detected, 1: jump detected, 2: squat detected
 */
int duckDetected(i2c_device_t* i2cdev); //   configure the display by setting the appropriate refresh delay and display duration for the display, and ensuring that all necessary pins are set to output mode


#endif
 
