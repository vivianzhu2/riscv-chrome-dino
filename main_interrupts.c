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
#include "console.h"
#include "interrupts.h"
#include "keyboard.h"
#include "shell.h"
#include "timer.h"
#include "uart.h"

void main(void) {
    interrupts_init();
    gpio_init();
    timer_init();
    uart_init();

    keyboard_init(KEYBOARD_CLOCK, KEYBOARD_DATA);
    console_init(30, 80, GL_GREEN, GL_BLACK);
    shell_init(keyboard_read_next, console_printf);

    interrupts_global_enable(); // everything fully initialized, now turn on interrupts
    shell_run();
}


int main(int argc, char **argv)
{
  interrupts_init();
  gpio_init();
  timer_init();
  uart_init();

  // keyboard_init(KEYBOARD_CLOCK, KEYBOARD_DATA);
  console_init(30, 80, GL_GREEN, GL_BLACK);
  // shell_init(keyboard_read_next, console_printf);

  interrupts_global_enable(); // everything fully initialized, now turn on interrupts
  shell_run();

  uint8_t data;
  uint8_t accel[6];
  uart_init();
  printf("BIT-BANG I2C DEMO with MPU-6050 IMU\n");

  i2c_init(I2C_400KHZ);
  i2c_device_t* i2cdev = i2c_new(MPU6050_ADDR);

  printf("I2c de %p",i2cdev);
//   printf("dev address %x\n", i2cdev->addr);
  printf("Power Management 1: 0x%02X\n", i2c_read_reg(i2cdev, MPU6050_RA_PWR_MGMT_1));
  i2c_write_reg(i2cdev, MPU6050_RA_PWR_MGMT_1, 0x80);

  timer_delay_ms(100);
  
  printf("Power Management 1: 0x%02X\n", i2c_read_reg(i2cdev, MPU6050_RA_PWR_MGMT_1));
  i2c_write_reg(i2cdev, MPU6050_RA_PWR_MGMT_1, 0x00);

  printf("CHIP_ID: 0x%02X\n", i2c_read_reg(i2cdev, MPU6050_RA_WHO_AM_I));
  printf("DONE\n");

  int threshold = 20000; // Adjust this threshold based on experimentation to detect jumps accurately
  // int jump_detected = 0;
  int takeoff_detected=0;
  int takeoff_time= 0; // fix this
  int landing_time = 0; // fix this
  while(1)
  {
    //for(uint8_t i=0; i<6; i++){
    //  accel[i]=read_byte(MPU6050_ADDR, 0x3B+i);
    //}
    i2c_read_reg_n(i2cdev, 0x3B, accel, 6);
    int16_t x_accel = (accel[0]<<8)+accel[1];
    int16_t y_accel = (accel[2]<<8)+accel[3];
    int16_t z_accel = (accel[4]<<8)+accel[5];
    printf("X: %d, Y: %d, Z: %d\n", x_accel, y_accel, z_accel);
    // (Math.signum(xval)*Math.sqrt(xval*xval+yval*yval+zval*zval))*-1)-9.81
    // int16_t totalaccel = (x_accel*x_accel+y_accel*y_accel+z_accel*z_accel);
    // printf("Accel: %d \n", totalaccel);

    // if(totalaccel > 20000 && takeoff_detected == 0){
    //     takeoff_detected = 1;
    //     takeoff_time = timer_get_ticks();
    //     printf("JUMP DETECTED\n");
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
    
    timer_delay_ms(50);
  }

  return 0;


}

// toggle when to look at jump