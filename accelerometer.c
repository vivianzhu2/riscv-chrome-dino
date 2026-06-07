

// #define MPU6050_ADDR    0x68
// #define PWR_MGMT_1      0x6B
// #define ACCEL_XOUT_H    0x3B

// void main(void) {
//     // 1. Init I2C bus at 400 KHz
//     i2c_init(I2C_400KHZ);

//     // 2. Connect to MPU6050 (returns NULL if not found)
//     i2c_device_t *mpu = i2c_new(MPU6050_ADDR);
//     if (!mpu) {
//         printf("MPU6050 not found!\n");
//         return;
//     }

//     // 3. Wake up the sensor (clear sleep bit in PWR_MGMT_1)
//     i2c_write_reg(mpu, PWR_MGMT_1, 0x00);

//     // 4. Read 6 bytes of accelerometer data
//     uint8_t buf[6];
//     i2c_read_reg_n(mpu, ACCEL_XOUT_H, buf, 6);

//     // 5. Combine high/low bytes
//     int16_t ax = (buf[0] << 8) | buf[1];
//     int16_t ay = (buf[2] << 8) | buf[3];
//     int16_t az = (buf[4] << 8) | buf[5];

//     // 6. Convert to m/s² (default ±2g range = 16384 LSB/g)
//     printf("X: %.3f  Y: %.3f  Z: %.3f (m/s²)\n",
//         ax / 16384.0f * 9.81f,
//         ay / 16384.0f * 9.81f,
//         az / 16384.0f * 9.81f);


// }

// #include <project.h>
#include "i2c.h"
#include <mpu6050.h>
#include <printf.h>
#include <uart.h>
#include <timer.h>


int readSample(void)
{
    char buf[50]; //just to hold text values in for writing to UART
    
	int16_t ax, ay, az, i;
	int16_t gx, gy, gz;
    int16_t (*data)[3] = NULL;
    i2c_init(I2C_400KHZ);
	uart_init();
	
	MPU6050_init();
	MPU6050_initialize();
	uart_putstring(MPU6050_testConnection() ? "MPU6050 connection successful\n\r" : "MPU6050 connection failed\n\n\r");
    int16_t takoff_thr = -3.5-9.81;
    int16_t landing_thr = 19.0;
    uart_putstring("Raw values from gyroscope..\n\r");
    static int num_samples = 0;
    while(1)
    {
      MPU6050_getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
      data[num_samples][0] = ax;
      data[num_samples][1] = ay;
      data[num_samples][2] = az;
      printf(buf, "AX:%d, AY:%d, AZ:%d || GX:%d, GY:%d, GZ:%d,\t", ax,ay,az,gx,gy,gz);
      uart_putstring(buf);
      uart_putstring("\n\r");
    //   CyDelay(300);

      num_samples++;
      timer_delay_ms(300);
    }
    
    while(1){}
    
    //From here you will want to look at converting values to angles and then complimentary filters to combine the values
    //and to try and stop the drift which will inevitably happen on the gyroscope...
    
}

void printData(int16_t[][] data){
    for(int i =0; i < 10000000; i++){
        printf("AX:%d, AY:%d, AZ:%d\n\r", data[i][0], data[i][1], data[i][2]);
    }
}







// #include <stdio.h>
// #include <stdlib.h>
// #include "platform.h"
// #include <string.h>
// #include <unistd.h>
// #include <math.h>
// #include "stdatomic.h"

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


int main(int argc, char **argv)
{
  uint8_t data;
  uint8_t accel[6];

  printf("BIT-BANG I2C DEMO with MPU-6050 IMU\n");

  i2c_init(I2C_400KHZ);
  i2c_device_t* i2cdev = i2c_new(MPU6050_ADDR);

  printf("Power Management 1: 0x%02X\n", i2c_read_reg(i2cdev, MPU6050_RA_PWR_MGMT_1));
  i2c_write_reg(i2cdev, MPU6050_RA_PWR_MGMT_1, 0x80);

  timer_delay_ms(100);
  
  printf("Power Management 1: 0x%02X\n", i2c_read_reg(i2cdev, MPU6050_RA_PWR_MGMT_1));
  i2c_write_reg(i2cdev, MPU6050_RA_PWR_MGMT_1, 0x00);

  printf("CHIP_ID: 0x%02X\n", i2c_read_reg(i2cdev, MPU6050_RA_WHO_AM_I));
  printf("DONE\n");

  while(1)
  {
    //for(uint8_t i=0; i<6; i++){
    //  accel[i]=read_byte(MPU6050_ADDR, 0x3B+i);
    //}
    i2c_read_reg_n(i2cdev, 0x3B, accel, 6);
    int16_t x_accel = (accel[0]<<8)+accel[1];
    int16_t y_accel = (accel[2]<<8)+accel[3];
    int16_t z_accel = (accel[4]<<8)+accel[5];
    //(Math.signum(xval)*Math.sqrt(xval*xval+yval*yval+zval*zval))*-1)-9.81
    float accel = (x_accel*x_accel+y_accel*y_accel+z_accel*z_accel);
    printf("Accel: \n", accel);
   
    //printf("X: %06d, Y: %06d, Z: %06d\n", x_accel, y_accel, z_accel);
    
    timer_delay_ms(300);
  }

  return 0;
}