/* File: duck.c
 * -----------------
 *  implement duck detection system
 * 
 * duck.c initializes the accelerometer and then reads in the data
 * It then checks if the data is a peak and within a specific threshold to return
 * whether the action is a jump or a squat or neither. 
 * 
 */

#include "i2c.h"
#include "printf.h"
#include "strings.h"
#include "uart.h"
#include "gpio.h"
#include "timer.h"

// ACCEL INITS AND ADDRESSES
#define MPU6050_ADDR 0x68

#define MPU6050_RA_ACCEL_XOUT_H     0x3B
#define MPU6050_RA_ACCEL_XOUT_L     0x3C
#define MPU6050_RA_ACCEL_YOUT_H     0x3D
#define MPU6050_RA_ACCEL_YOUT_L     0x3E
#define MPU6050_RA_ACCEL_ZOUT_H     0x3F
#define MPU6050_RA_ACCEL_ZOUT_L     0x40
#define MPU6050_RA_TEMP_OUT_H       0x41
#define MPU6050_RA_TEMP_OUT_L       0x42
#define MPU6050_RA_GYRO_XOUT_H      0x43
#define MPU6050_RA_GYRO_XOUT_L      0x44
#define MPU6050_RA_GYRO_YOUT_H      0x45
#define MPU6050_RA_GYRO_YOUT_L      0x46
#define MPU6050_RA_GYRO_ZOUT_H      0x47
#define MPU6050_RA_GYRO_ZOUT_L      0x48
#define MPU6050_RA_PWR_MGMT_1       0x6B
#define MPU6050_RA_WHO_AM_I         0x75

#define SDA PIN_8_OFFSET
#define SCL PIN_9_OFFSET

#define LONG 25 // Equals ~400KHz
#define SHORT 5

// keep track of the magnitudes of the three points to check for a peak
static unsigned int nexttotal = 0;
static unsigned int currtotal = 0;
static unsigned int prevtotal = 0;

// jump and squat initial time markings
static unsigned int jtakeoff_time= 0; 
static unsigned int stakeoff_time = 0;
/*
 * `accelDuck_init: initializes the accelerometer MPU
 *
 * confirms the accelerometer is running and instantiates new device with its frequency and written registers.
 *
 * 
 * code inspired by / courtesy of  https://github.com/hasanunlu/i2c_demo_for_HiFive1/blob/master/i2c_demo/i2c_demo.c and the i2c driver
 * @return     i2c_device_t with the accelerometer properties
 */
i2c_device_t* accelDuck_init(){

  uint8_t data;
  uint8_t accel[6];

  printf("BIT-BANG I2C DEMO with MPU-6050 IMU\n");

  i2c_init(I2C_400KHZ);
  i2c_device_t* i2cdev = i2c_new(MPU6050_ADDR);

  printf("I2c de %p",i2cdev);

  printf("Power Management 1: 0x%02X\n", i2c_read_reg(i2cdev, MPU6050_RA_PWR_MGMT_1));
  i2c_write_reg(i2cdev, MPU6050_RA_PWR_MGMT_1, 0x80);

  timer_delay_ms(100);
  
  printf("Power Management 1: 0x%02X\n", i2c_read_reg(i2cdev, MPU6050_RA_PWR_MGMT_1));
  i2c_write_reg(i2cdev, MPU6050_RA_PWR_MGMT_1, 0x00);

  printf("CHIP_ID: 0x%02X\n", i2c_read_reg(i2cdev, MPU6050_RA_WHO_AM_I));
  printf("DONE\n");
  return i2cdev;
}


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
int duckDetected(i2c_device_t* i2cdev){
    int jump_threshold = 830723680; // Adjust this threshold based on experimentation to detect jumps accurately
    static int jump_detected = 0;

    static int landing_time = 0;
    static int squat_detected = 0;
    static int squat_threshold = 404970496; // Adjust this threshold based on experimentation to detect squats accurately
    static uint8_t accel[6];

   
    prevtotal = currtotal; //update values to shift and next turns into empty
    currtotal = nexttotal; 

 

    i2c_read_reg_n(i2cdev, 0x3B, accel, 6); // read accel data
    int16_t nextx=  (accel[0]<<8)+accel[1];
    int16_t nexty = (accel[2]<<8)+accel[3];
    int16_t nextz=  (accel[4]<<8)+accel[5];
    unsigned int currtime = timer_get_ticks(); // current time to add
    nexttotal = (nextx * nextx + nexty * nexty + nextz * nextz); // calculate magnitude
//    printf("prevtotal: %d \ncurrtotal: %d\n nexttotal: %d\n", prevtotal, currtotal, nexttotal);
    if(nexttotal < currtotal && currtotal > prevtotal && nexttotal!=0 && currtotal!=0 && prevtotal !=0){ // is it a peak??
 
     if(currtotal > jump_threshold && currtime-jtakeoff_time > 1.5*24000000){ // is it a jump?? and is it the FIRST spike of the jump?
          jtakeoff_time = timer_get_ticks();  // start time of when it starts jumping
          stakeoff_time = timer_get_ticks();  

          printf("JUMP DETECTED\n"); 
          return 1; //return if it passes condition!! 
        }
        else if(currtotal > squat_threshold && currtime-stakeoff_time > 1.5*24000000){ // is it a squat?? 
          stakeoff_time = timer_get_ticks();  // start time of squat
          printf("SQUAT DETECTED\n");
          return 2; // return if squat!!
        }
        else{
          return 0; // return if neither :( 
        }
    }


  return 0;

}
