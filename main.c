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
    while(1){
        printf("%d", duckDetected(ducktest));
        // jumpDetected(ducktest);
    }
    
    
    //    say_hello("CS107e");
    // mango_run();

//    uart_putstring("hello\n");
}
// #include "i2c.h"
// #include "printf.h"
// #include "strings.h"
// #include "uart.h"
// #include "gpio.h"
// #include "timer.h"
// #include "jump.h"

// #define MPU6050_ADDR 0x68

// #define MPU6050_RA_ACCEL_XOUT_H     0x3B
// #define MPU6050_RA_ACCEL_XOUT_L     0x3C
// #define MPU6050_RA_ACCEL_YOUT_H     0x3D
// #define MPU6050_RA_ACCEL_YOUT_L     0x3E
// #define MPU6050_RA_ACCEL_ZOUT_H     0x3F
// #define MPU6050_RA_ACCEL_ZOUT_L     0x40
// #define MPU6050_RA_TEMP_OUT_H       0x41
// #define MPU6050_RA_TEMP_OUT_L       0x42
// #define MPU6050_RA_GYRO_XOUT_H      0x43
// #define MPU6050_RA_GYRO_XOUT_L      0x44
// #define MPU6050_RA_GYRO_YOUT_H      0x45
// #define MPU6050_RA_GYRO_YOUT_L      0x46
// #define MPU6050_RA_GYRO_ZOUT_H      0x47
// #define MPU6050_RA_GYRO_ZOUT_L      0x48
// #define MPU6050_RA_PWR_MGMT_1       0x6B
// #define MPU6050_RA_WHO_AM_I         0x75

// #define SDA PIN_8_OFFSET
// #define SCL PIN_9_OFFSET

// #define LONG 25 // Equals ~400KHz
// #define SHORT 5
// typedef struct {
//   uint16_t x_a;
//   uint16_t y_a;
//   uint16_t z_a;
//   uint16_t totalaccel;
// } accel_t;



// int main(int argc, char **argv)
// {
//   uint8_t data;
//   uint8_t accel[6];
//   uart_init();
//   timer_init();
//   // console_init(800,600, GL_GREEN, GL_BLACK);
//   printf("BIT-BANG I2C DEMO with MPU-6050 IMU\n");

//   i2c_init(I2C_400KHZ);
//   i2c_device_t* i2cdev = i2c_new(MPU6050_ADDR);

//   printf("I2c de %p",i2cdev);
// //   printf("dev address %x\n", i2cdev->addr);
//   printf("Power Management 1: 0x%02X\n", i2c_read_reg(i2cdev, MPU6050_RA_PWR_MGMT_1));
//   i2c_write_reg(i2cdev, MPU6050_RA_PWR_MGMT_1, 0x80);

//   timer_delay_ms(100);
  
//   printf("Power Management 1: 0x%02X\n", i2c_read_reg(i2cdev, MPU6050_RA_PWR_MGMT_1));
//   i2c_write_reg(i2cdev, MPU6050_RA_PWR_MGMT_1, 0x00);

//   printf("CHIP_ID: 0x%02X\n", i2c_read_reg(i2cdev, MPU6050_RA_WHO_AM_I));
//   printf("DONE\n");

//   int jump_threshold = 830723680; // Adjust this threshold based on experimentation to detect jumps accurately
//   static int jump_detected = 0;
//   static int takeoff_detected=0;
//   static int takeoff_time= 0; // fix this
//   static int landing_time = 0;
//   static int squat_detected = 0;
//   static int squat_threshold = 404970496; // Adjust this threshold based on experimentation to detect squats accurately
//   static uint8_t accel[6];
//   static accel_t prev_acc = NULL;
//   static accel_t curr_acc = NULL;
//   static accel_t next_acc = NULL;
//   while(1){
// //     //1. Prev/curr/next_acc, just use unsigned int to store the total, not need to use struct to store x/y/z
// // 2. Prev=curr;curr=next. Need to move before reading the sensor; 
// // 3. Continue should be return 0; at the end should return 0 too 
// // 4. line jump_detected=0; should be jump_detected=1
// // 5. This function should replace
// // the old jumpdetected function. With the same jumpdetected function name as used in the main function.

//     prev_acc = curr_acc;
//     curr_acc = next_acc;


//     i2c_read_reg_n(i2cdev, 0x3B, accel, 6);
//     next_acc.x_a=  (accel[0]<<8)+accel[1];
//     next_acc.y_a = (accel[2]<<8)+accel[3];
//     next_acc.z_a =  (accel[4]<<8)+accel[5];
//     int currtime = timer.get_ticks(); 

//     next_acc.totalaccel = (next_acc.x_a * next_acc.x_a + next_acc.y_a * next_acc.y_a + next_acc.z_a * next_acc.z_a);

//     if(next_acc.totalaccel < curr_acc.totalaccel && curr_acc.totalaccel > prev_accel.totalaccel){

//         if(curr_acc.totalaccel > jump_threshold && currtime-takeoff_time > 2*24000000){
//           takeoff_time = timer_get_ticks();  
//           jump_detected = 0;
//           printf("JUMP DETECTED");
//           return 1;
//         }
//         else if(curr_acc.totalaccel > squat_threshold && currtime-takeoff_time > 2*24000000){
//           takeoff_time = timer_get_ticks();  
//           squat_detected = 1;
//           printf("SQUAT DETECTED")
//           return 2;
//         }
//         else{
//             return 0;
//         }
//     }

//   }

// }

// // toggle when to look at jump
// // void mango_run() {
// //   init_colors();
// //   uart_init();
// //   i2c_device_t acc = accel_init();
// //   ps2_dev = ps2_new(GPIO_PB3, GPIO_PB4); 
// //   interrupts_global_enable();
// //   gl_init(SCREEN_W, SCREEN_H, GL_DOUBLEBUFFER);
// //   game_reset();
// //   int jump_detected = 0;
// //   int jump_done = 1;
// //   while (1) {
// //       unsigned int t0 = timer_get_ticks();
// //       if(jump_done == 1 && jump_detected == 0){
// //         jump_detected = jumpDetected(&acc);
        
// //       }
// //       frame_counter++;
// //       update_player();
// //       update_world();
// //       render();

      
// //   while (timer_get_ticks() - t0 < 10000);
// //   }
// // }