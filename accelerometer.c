#include "i2c.h"

#define MPU6050_ADDR    0x68
#define PWR_MGMT_1      0x6B
#define ACCEL_XOUT_H    0x3B

void main(void) {
    // 1. Init I2C bus at 400 KHz
    i2c_init(I2C_400KHZ);

    // 2. Connect to MPU6050 (returns NULL if not found)
    i2c_device_t *mpu = i2c_new(MPU6050_ADDR);
    if (!mpu) {
        printf("MPU6050 not found!\n");
        return;
    }

    // 3. Wake up the sensor (clear sleep bit in PWR_MGMT_1)
    i2c_write_reg(mpu, PWR_MGMT_1, 0x00);

    // 4. Read 6 bytes of accelerometer data
    uint8_t buf[6];
    i2c_read_reg_n(mpu, ACCEL_XOUT_H, buf, 6);

    // 5. Combine high/low bytes
    int16_t ax = (buf[0] << 8) | buf[1];
    int16_t ay = (buf[2] << 8) | buf[3];
    int16_t az = (buf[4] << 8) | buf[5];

    // 6. Convert to m/s² (default ±2g range = 16384 LSB/g)
    printf("X: %.3f  Y: %.3f  Z: %.3f (m/s²)\n",
        ax / 16384.0f * 9.81f,
        ay / 16384.0f * 9.81f,
        az / 16384.0f * 9.81f);


}

// #include <project.h>
#include <mpu6050.h>
#include <printf.h>
#include <uart.h>
#include <timer.h>
int main()
{
    char buf[50]; //just to hold text values in for writing to UART
    
	int16_t ax, ay, az, i;
	int16_t gx, gy, gz;
    
    i2c_init(I2C_400KHZ);
	uart_init();
	
	MPU6050_init();
	MPU6050_initialize();
	uart_putstring(MPU6050_testConnection() ? "MPU6050 connection successful\n\r" : "MPU6050 connection failed\n\n\r");
    takoff_thr=-3.5-9.81;
    landing_thr=19.0;
    uart_putstring("Raw values from gyroscope..\n\r");
    static int num_samples = 0;
    while(1)
    {
      MPU6050_getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
      
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