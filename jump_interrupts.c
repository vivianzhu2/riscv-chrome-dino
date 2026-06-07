/* File: ps2_assign7.c
 * -------------------
 // This file implements the ps2 module for assign7. Instead of assign5 algorithm of polling the clock and data lines, this version uses interrupts to read scancodes from the PS/2 device.
 * The ps2 module encapsulates functionality for interacting with a PS/2 device.
 * The ps2.c module has only two functions: ps2_new and ps2_read.
 * All of the nitty-gritty low-level protocol details are encapsulated within the ps2 module. 
 */
#include "gpio.h"
#include "gpio_extra.h"
#include "malloc.h"
#include "ps2.h"
#include "printf.h"
#include "timer.h"
#include "ringbuffer.h"
#include "gpio_interrupt.h"
#include "uart.h"
// A ps2_device is a structure that stores all of the state and information
// needed for a PS2 device. The clock field stores the gpio id for the
// clock pin, and the data field stores the gpio id for the data pin.
// Read ps2_new for example code that sets and uses these fields.
//
// You may extend the ps2_device structure with additional fields as needed.
// A pointer to the current ps2_device is passed into all ps2_ calls.
// Storing state in this structure is preferable to using global variables:
// it allows your driver to support multiple PS2 devices accessed concurrently
// (e.g., a keyboard and a mouse).
//

// The ps2_state field is an enum that defines to track the current state of the scancode being read.
typedef enum ps2_state_ids ps2_state_t;
enum ps2_state_ids {
    PS2_IDLE = 0x000, // start of B group, enum vals assigned in sequence from here
    PS2_START,
    PS2_BIT0,
    PS2_BIT1,
    PS2_BIT2,
    PS2_BIT3,
    PS2_BIT4,
    PS2_BIT5,
    PS2_BIT6,
    PS2_BIT7,
    PS2_GPARITY
};

// This definition fills out the structure declared in ps2.h.
// added two fields: ps2_state to track the current state of the scancode being read, and rb to store the scancodes read from the PS2 device in an interrupt handler and later read by ps2_read.
struct ps2_device {
    gpio_id_t clock;
    gpio_id_t data;
    ps2_state_t ps2_state;
    rb_t *rb;
};


// This function is the interrupt handler for the PS2 clock gpio. It is called on every falling edge of the clock signal.
// It reads the current value of the data line and updates the ps2_state and code fields of the ps2_device accordingly. When a full scancode has been read, 
//it enqueues the scancode into the ring buffer for later retrieval by ps2_read.

    
void handle_clock(void* aux_data){
    gpio_interrupt_clear(((ps2_device_t*)aux_data)->clock); // clear the interrupt to allow future interrupts to be generated

    static int ones = 0;
    static uint8_t code = 0; 
    // static int resync=0; ///
    static unsigned long time=0;
    unsigned long lasttime = time;
    time = timer_get_ticks(); 
    //unsigned long time = timer_get_ticks();

    // gpio_id_t clock = ((ps2_device_t*)aux_data)->clock;
    uint8_t rdata = gpio_read(((ps2_device_t*)aux_data)->data);
// state machine to read bits of scancode, check for valid start bit, parity bit, and stop bit. If a full valid scancode is read, enqueue it into the ring buffer.
// If an invalid bit is read, reset state and start over.
    switch (((ps2_device_t*)aux_data)->ps2_state) { 
        case PS2_IDLE: // wait for start bit, which is 0
            ones = 0;
            code = 0;

            if(rdata == 0){ // start bit detected, move to next state to read data bits
                ((ps2_device_t*)aux_data)->ps2_state = PS2_START; 
            }
            break;

        case PS2_START: //start state


            if(time - lasttime > 500 * 24){
                // printf("restart: %ld, time: %ld", restart, time);
                if(rdata == 0){ // start bit detected, move to next state to read data bits
                    ((ps2_device_t*)aux_data)->ps2_state = PS2_START; 
                    ones = 0;
                    code = 0;
                }
                else
                     ((ps2_device_t*)aux_data)->ps2_state = PS2_IDLE; 
                break;
            }
            if(rdata == 1){
                ones++; // count number of ones for parity check
            }

            ((ps2_device_t*)aux_data)->ps2_state = PS2_BIT0; // move to next state to read first data bit
            code |= (rdata << 0);  // store first data bit in code variable, LSB first
            break;
        case PS2_BIT0: // read first bit
            if(time - lasttime > 500 * 24){
                // printf("restart: %ld, time: %ld", restart, time);
                if(rdata == 0){ // start bit detected, move to next state to read data bits
                    ((ps2_device_t*)aux_data)->ps2_state = PS2_START; 
                    ones = 0;
                    code = 0;
                }
                else
                     ((ps2_device_t*)aux_data)->ps2_state = PS2_IDLE; 
                break;
            }
            if(rdata == 1){
                ones++; // count number of ones for parity check
            }
            
            ((ps2_device_t*)aux_data)->ps2_state = PS2_BIT1; // move to next state to read second data bit
            code |= (rdata << 1);  // store second data bit in code variable, LSB first
            break;
        case PS2_BIT1: // read second bit
            if(time - lasttime > 500 * 24){
                // printf("restart: %ld, time: %ld", restart, time);
                if(rdata == 0){ // start bit detected, move to next state to read data bits
                    ((ps2_device_t*)aux_data)->ps2_state = PS2_START; 
                    ones = 0;
                    code = 0;
                }
                else
                     ((ps2_device_t*)aux_data)->ps2_state = PS2_IDLE; 
                break;
            }
            if(rdata == 1){
                ones++;  // count number of ones for parity check
            }
            ((ps2_device_t*)aux_data)->ps2_state = PS2_BIT2; // move to next state to read third data bit
            code |= (rdata<< 2); // store third data bit in code variable, LSB first

            break;
        case PS2_BIT2: // read third bit
            if(time - lasttime > 500 * 24){
                // printf("restart: %ld, time: %ld", restart, time);
                if(rdata == 0){ // start bit detected, move to next state to read data bits
                    ((ps2_device_t*)aux_data)->ps2_state = PS2_START; 
                    ones = 0;
                    code = 0;
                }
                else
                     ((ps2_device_t*)aux_data)->ps2_state = PS2_IDLE; 
                break;
            }            if(rdata == 1){
                ones++;  // count number of ones for parity check
            }
            ((ps2_device_t*)aux_data)->ps2_state = PS2_BIT3; // move to next state to read fourth data bit
            code |= (rdata << 3);   
            break;
        case PS2_BIT3: // read fourth bit
            if(time - lasttime > 500 * 24){
                // printf("restart: %ld, time: %ld", restart, time);
                if(rdata == 0){ // start bit detected, move to next state to read data bits
                    ((ps2_device_t*)aux_data)->ps2_state = PS2_START; 
                    ones = 0;
                    code = 0;
                }
                else
                     ((ps2_device_t*)aux_data)->ps2_state = PS2_IDLE; 
                break;
            }            if(rdata == 1){
                ones++;  // count number of ones for parity check
            }
            ((ps2_device_t*)aux_data)->ps2_state = PS2_BIT4; // move to next state to read fifth data bit
            code |= (rdata << 4);  // store fifth data bit in code variable, LSB first
            // printf("Case 3 is Matched.");
            break;
        case PS2_BIT4: // read fifth bit
            if(time - lasttime > 500 * 24){
                // printf("restart: %ld, time: %ld", restart, time);
                if(rdata == 0){ // start bit detected, move to next state to read data bits
                    ((ps2_device_t*)aux_data)->ps2_state = PS2_START; 
                    ones = 0;
                    code = 0;
                }
                else
                     ((ps2_device_t*)aux_data)->ps2_state = PS2_IDLE; 
                break;
            }            if(rdata == 1){ 
                ones++; 
            }
            ((ps2_device_t*)aux_data)->ps2_state = PS2_BIT5; // move to next state to read sixth data bit
            code |= (rdata << 5); //    store sixth data bit in code variable, LSB first
            // printf("Case 3 is Matched.");
            break;
        case PS2_BIT5:  // read sixth bit
            if(time - lasttime > 500 * 24){
                // printf("restart: %ld, time: %ld", restart, time);
                if(rdata == 0){ // start bit detected, move to next state to read data bits
                    ((ps2_device_t*)aux_data)->ps2_state = PS2_START; 
                    ones = 0;
                    code = 0;
                }
                else
                     ((ps2_device_t*)aux_data)->ps2_state = PS2_IDLE; 
                break;
            }
            if(rdata== 1){
                ones++; // count number of ones for parity check
            }
            ((ps2_device_t*)aux_data)->ps2_state = PS2_BIT6; // move to next state to read seventh data bit
            code |= (rdata<< 6); 
            // printf("Case 3 is Matched.");
            break;
        case PS2_BIT6: // read seventh bit
            if(time - lasttime > 500 * 24){
                // printf("restart: %ld, time: %ld", restart, time);
                if(rdata == 0){ // start bit detected, move to next state to read data bits
                    ((ps2_device_t*)aux_data)->ps2_state = PS2_START; 
                    ones = 0;
                    code = 0;
                }
                else
                     ((ps2_device_t*)aux_data)->ps2_state = PS2_IDLE; 
                break;
            }
            if(rdata == 1){
                ones++;  // count number of ones for parity check
            }
            ((ps2_device_t*)aux_data)->ps2_state = PS2_BIT7; // move to next state to read parity bit
            code |= (rdata << 7);  // store seventh data bit in code variable, LSB first
            // printf("Case 3 is Matched.");
            break;
        case PS2_BIT7: //read parity bit
            if(time - lasttime > 500 * 24){
                // printf("restart: %ld, time: %ld", restart, time);
                if(rdata == 0){ // start bit detected, move to next state to read data bits
                    ((ps2_device_t*)aux_data)->ps2_state = PS2_START; 
                    ones = 0;
                    code = 0;
                }
                else
                     ((ps2_device_t*)aux_data)->ps2_state = PS2_IDLE; 
                break;
            }
            if(rdata == 1){
                ones++; // count number of ones for parity check
            }
            if(ones % 2 == 1){ // check for valid odd parity, if valid move to next state to read stop bit
                ((ps2_device_t*)aux_data)->ps2_state = PS2_GPARITY; // move to next state to read stop bit
            }
            else{
                if(rdata == 0){ // start bit detected, move to next state to read data bits
                    ((ps2_device_t*)aux_data)->ps2_state = PS2_START; 
                    ones = 0;
                    code = 0;
                }
                else
                     ((ps2_device_t*)aux_data)->ps2_state = PS2_IDLE; 
            
            }
            
            // printf("Case 3 is Matched.");
            break;
        case PS2_GPARITY: // read stop bit, which should be 1
            if(time - lasttime > 500 * 24){
                // printf("restart: %ld, time: %ld", restart, time);
                if(rdata == 0){ // start bit detected, move to next state to read data bits
                    ((ps2_device_t*)aux_data)->ps2_state = PS2_START; 
                    ones = 0;
                    code = 0;
                }
                else
                     ((ps2_device_t*)aux_data)->ps2_state = PS2_IDLE; 
                break;
            }
            if(rdata == 1){
                if(!rb_full(((ps2_device_t*)aux_data)->rb)){ // if ring buffer is not full, enqueue the scancode for later retrieval by ps2_read
                    rb_enqueue(((ps2_device_t*)aux_data)->rb, code);
                }
            }
                if(rdata == 0){ // start bit detected, move to next state to read data bits
                    ((ps2_device_t*)aux_data)->ps2_state = PS2_START; 
                    ones = 0;
                    code = 0;
                }
                else
                     ((ps2_device_t*)aux_data)->ps2_state = PS2_IDLE; 
                break;
        default: 
            break;
        }
    
}

// Creates a new PS2 device connected to given clock and data pins,
// The gpios are configured as input and set to use internal pull-up
// (PS/2 protocol requires clock/data to be high default)
ps2_device_t *ps2_new(gpio_id_t clock_gpio, gpio_id_t data_gpio) {
    
    ps2_device_t *dev = malloc(sizeof(*dev));
    dev->rb = rb_new(); // initialize ring buffer to store scancodes 
    dev->ps2_state= PS2_IDLE; // initialize state to idle, waiting for start bit


   
    dev->clock = clock_gpio;
    gpio_set_input(dev->clock);
    gpio_set_pullup(dev->clock);

    dev->data = data_gpio;
    gpio_set_input(dev->data);
    gpio_set_pullup(dev->data);

    gpio_interrupt_init(); 
    gpio_interrupt_config(clock_gpio, GPIO_INTERRUPT_NEGATIVE_EDGE, false); // configure interrupt on falling edge of clock signal
    gpio_interrupt_set_handler(clock_gpio, handle_clock, dev); // set interrupt handler for clock signal 

    return dev;
}

// OLD function. No longer used in the interrupt version. 
// Helper function to read one bit from the PS2 device, checking for valid timing
uint8_t read_bit(ps2_device_t *dev){
    // check if the input is high enough
    gpio_id_t clock = dev->clock;
    gpio_id_t data = dev->data;

    while(gpio_read(clock) == 0){ // wait for rising edge
    }
    while(gpio_read(clock) == 1){ // wait for falling edge
    }
    return  gpio_read(data);
}


// old function. No longer used in the interrupt version. ps2read2 is the polling version!! 

// The ps2_read function reads one well-formed scancode.
// "Well-formed" means that the start, parity, and stop bit each have valid values.
// Read a single PS2 scancode. Always returns a correctly received scancode:
// if an error occurs (e.g., start bit not detected, parity is wrong), the
// function should read another scancode.
uint8_t ps2_readOLD(ps2_device_t *dev) {
   
    int ones = 0;
    uint8_t code = 0; 
    unsigned long time = timer_get_ticks();
    unsigned long lasttime = timer_get_ticks();
    while(1){
        int start = read_bit(dev);
        if(start != 0){
            continue ;
        }
        lasttime = time;
        time =  timer_get_ticks();

        for(int i = 0; i < 8; i++){
            int bit = read_bit(dev);
            lasttime = time;
            time = timer_get_ticks(); 
            if(time - lasttime > 500 * 24){
                continue;
            }
            code |= (bit << i); 
            if(bit == 1){
                ones += 1;
            }
        }
        int parity = read_bit(dev);
        lasttime = time;
        time = timer_get_ticks(); 
        if(time - lasttime > 500 * 24){
            continue;
        }
        if((ones + parity) % 2 == 0){
            ones = 0;
            continue;  // bad parity, try again
        }
        else{
            ones = 0;
        }

        // // 4. stop bit must be 1
        int stop = read_bit(dev);
        lasttime = time;
        time = timer_get_ticks(); 
        if(time - lasttime > 500 * 24){
            // printf("restart: %ld, time: %ld", restart, time);
            continue;
        }
        if(stop != 1){
            continue;
        }   
        return code;
    }

}
// The new ps2_read function for the interrupt version. It waits for a scancode to be available in the ring buffer, which is filled by the interrupt handler, and returns it.
//It always returns a scancode, either one already in the queue or waits for a new one to be enqueued.
uint8_t ps2_read(ps2_device_t *dev) {

    // static int ones = 0;
    // static uint8_t code = 0; 
    

    while(1){ // wait for a scancode to be available in the ring buffer, which is filled by the interrupt handler
        if(!rb_empty(dev->rb)){
            uint8_t newscan = rb_dequeue(dev->rb); // dequeue the next scancode from the ring buffer
            return newscan; // return the scancode
        }
    }
    return 0;

}


