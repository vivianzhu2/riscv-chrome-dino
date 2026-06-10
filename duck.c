#include "i2c.h"
#include "printf.h"
#include "strings.h"
#include "uart.h"
#include "gpio.h"
#include "timer.h"


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

static unsigned int nexttotal = 0;
static unsigned int currtotal = 0;
static unsigned int prevtotal = 0;


static unsigned int jtakeoff_time= 0; // fix this
static unsigned int stakeoff_time = 0;
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



int duckDetected(i2c_device_t* i2cdev){
    int jump_threshold = 830723680; // Adjust this threshold based on experimentation to detect jumps accurately
    static int jump_detected = 0;
    // static int takeoff_detected = 0;


    // static int start = 0;
    // if (start == 0) {
    //     takeoff_time = timer_get_ticks();
    //     start = 1;
    // }
    static int landing_time = 0;
    static int squat_detected = 0;
    static int squat_threshold = 404970496; // Adjust this threshold based on experimentation to detect squats accurately
    static uint8_t accel[6];
    

    // static accel_t prev_acc = NULL;
    // static accel_t curr_acc = NULL;
    // static accel_t next_acc = NULL;

    
    prevtotal = currtotal;
    currtotal = nexttotal; 

 

    i2c_read_reg_n(i2cdev, 0x3B, accel, 6);
    int16_t nextx=  (accel[0]<<8)+accel[1];
    int16_t nexty = (accel[2]<<8)+accel[3];
    int16_t nextz=  (accel[4]<<8)+accel[5];
    unsigned int currtime = timer_get_ticks(); 

    nexttotal = (nextx * nextx + nexty * nexty + nextz * nextz);
//    printf("prevtotal: %d \ncurrtotal: %d\n nexttotal: %d\n", prevtotal, currtotal, nexttotal);
    if(nexttotal < currtotal && currtotal > prevtotal && nexttotal!=0 && currtotal!=0 && prevtotal !=0){
 /*    printf("PEAK: %d | jump_diff: %d | squat_diff: %d\n", 
        currtotal, 
        currtime - jtakeoff_time,
        currtime - stakeoff_time);
   */     if(currtotal > jump_threshold && currtime-jtakeoff_time > 1.5*24000000){
          jtakeoff_time = timer_get_ticks();  
          stakeoff_time = timer_get_ticks();  
          // printf("PEAK: %u | jump_thresh: %u (%s) | squat_thresh: %u (%s)\n",
          //   currtotal,
          //   jump_threshold,  currtotal > jump_threshold  ? "PASS" : "FAIL",
          //   squat_threshold, currtotal > squat_threshold ? "PASS" : "FAIL");
          // jump_detected = 1;
          printf("JUMP DETECTED\n");
          return 1;
        }
        else if(currtotal > squat_threshold && currtime-stakeoff_time > 1.5*24000000){
          stakeoff_time = timer_get_ticks();  
          // squat_detected = 1;
          printf("SQUAT DETECTED\n");
          return 2;
        }
        else{
          return 0;
        }
    }

    




    // if(totalaccel > threshold && currtime-takeoff_time > 2*24000000){ // Check if enough time has passed since last takeoff to prevent multiple detections for the same jump
    //     // takeoff_detected = 1;
    //     takeoff_time = timer_get_ticks();
    //    printf("JUMP DETECTED\n");
    //     re turn 1;
    // }
    // if(takeoff_detected == 1 && totalaccel > 20000){
    //     landing_time = timer_get_ticks();
    //     int airtime = landing_time - takeoff_time;
    //     if(airtime > 500){ // Adjust this threshold based on experimentation to filter out false positives
    //         jump_detected = 1;  
    //     }
    //     takeoff_detected = 0; // reset for next jump
    // }

   
    //printf("X: %06d, Y: %06d, Z: %06d\n", x_accel, y_accel, z_accel);
    
    // timer_delay_ms(50);
  

  return 0;

}
void duckReset(void){
  stakeoff_time = 0;
  jtakeoff_time = 0;
  prevtotal = 0;
  currtotal = 0;
  nexttotal = 0;
}

// toggle when to look at jump

//returns 1 when it is a jump, 0 when it is not
