/* File name: game.c
 * File description: 
 *
 */

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
#include "uart.h"
#include "i2c.h"
#include "printf.h"

#define SCREEN_W 640
#define SCREEN_H 480
#define GROUND_Y 360
#define GROUND_THICK 4
#define SCALE 4

#define PLAYER_ROWS 8
#define PLAYER_COLS 8
#define CACTUS_ROWS 10
#define CACTUS_COLS 7
#define MANGO_ROWS 7
#define MANGO_COLS 7

#define PLAYER_X 80
#define CACTUS_Y (CACTUS_ROWS * SCALE)
#define CACTUS_W (CACTUS_COLS * SCALE)
#define MANGO_Y (MANGO_ROWS * SCALE)
#define MANGO_W (MANGO_COLS * SCALE)
#define MAX_CACTI 2
#define MAX_MANGOS 3

#define GRAVITY 2
#define JUMP_VY -9

static int world_speed = 3;
static int frame_counter = 0;

static color_t COLOR_GROUND;
static color_t COLOR_BACKGROUND;
static color_t player_palette[4];
static color_t cactus_palette[2];
static color_t mango_palette[3];

static void init_colors(void) {
    COLOR_GROUND = gl_color(0x3a, 0x3a, 0x3a);
    COLOR_BACKGROUND = gl_color(0x1a, 0x1a, 0x1a);
    player_palette[0] = 0;
    player_palette[1] = gl_color(0xff, 0xcc, 0xbc);    // skin
    player_palette[2] = gl_color(0xf4, 0x43, 0x36);    // shirt
    player_palette[3] = gl_color(0x37, 0x47, 0x4f);    // pants
    cactus_palette[0] = 0;
    cactus_palette[1] = gl_color(0x4c, 0xaf, 0x50);    // green
    mango_palette[0] = 0;
    mango_palette[1] = gl_color(0xff, 0x98, 0x00);    // body
    mango_palette[2] = gl_color(0x2e, 0x7d, 0x32);    // leaf
}

static const unsigned char player_map[PLAYER_ROWS][PLAYER_COLS] = {
    {0,0,1,1,1,1,0,0},
    {0,1,1,1,1,1,1,0},
    {0,0,1,0,0,1,0,0},
    {0,2,2,2,2,2,2,0},
    {2,2,2,2,2,2,2,2},
    {0,0,3,3,3,3,0,0},
    {0,3,0,0,0,0,3,0},
    {3,0,0,0,0,0,0,3},
};

static const unsigned char cactus_map[CACTUS_ROWS][CACTUS_COLS] = {
    {0,0,1,1,1,0,0},
    {0,0,1,1,1,0,0},
    {1,0,1,1,1,0,1},
    {1,1,1,1,1,1,1},
    {0,1,1,1,1,1,0},
    {0,0,1,1,1,0,0},
    {0,0,1,1,1,0,0},
    {0,0,1,1,1,0,0},
    {0,0,1,1,1,0,0},
    {0,0,1,1,1,0,0},
};
 
static const unsigned char mango_map[MANGO_ROWS][MANGO_COLS] = {
    {0,0,0,2,0,0,0},
    {0,0,2,2,2,0,0},
    {0,1,1,1,1,1,0},
    {1,1,1,1,1,1,1},
    {1,1,1,1,1,1,1},
    {0,1,1,1,1,1,0},
    {0,0,1,1,1,0,0},
};


typedef struct {
    int  py;
    int  vy;
    bool grounded;
} player_h;

typedef struct {
    int  x;
    bool on_screen;
} cactus_h;

typedef struct {
    int  x;
    int  y;
    bool active;
    bool on_screen;
} mango_h;


static player_h player;
static cactus_h cacti[MAX_CACTI];   // stores cacti on screen
static mango_h mangos[MAX_MANGOS];    // stores mangos on screen

static void draw_sprite(int px, int py, const unsigned char *sprite, int rows, int cols, const color_t *palette) {
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            unsigned char index = sprite[r * cols + c];
            if (index != 0) {
                gl_draw_rect(px + c * SCALE, py + r * SCALE, SCALE, SCALE, palette[index]);  // draws one pixel at a time
            }
        }
    }
}


static void draw_ground() {
    gl_draw_rect(0, GROUND_Y, SCREEN_W, GROUND_THICK, COLOR_GROUND);
}

static void draw_player() {
    int py = player.py;
    int px = PLAYER_X;
    draw_sprite(px, py, &player_map[0][0], PLAYER_ROWS, PLAYER_COLS, player_palette);
}

static void draw_cactus() {
    int py = GROUND_Y - CACTUS_Y;
    int px;
    for (int i = 0; i < MAX_CACTI; i++) {
	if (!cacti[i].on_screen) {
	    continue;
	}
	px = cacti[i].x;
	draw_sprite(px, py, &cactus_map[0][0], CACTUS_ROWS, CACTUS_COLS, cactus_palette);
    }
}

static void draw_mango() {    // mangos are all on the floor
    int py = GROUND_Y - MANGO_Y;
    int px;
    for (int i = 0; i < MAX_MANGOS; i++) {
	if (!mangos[i].active) {
	    continue;
        }
	px = mangos[i].x;
	draw_sprite(px, py, &mango_map[0][0], MANGO_ROWS, MANGO_COLS, mango_palette);
    }
} 

static void jump() {    // no double jumps, only jumps when player is grounded
    if (player.grounded) {
	player.vy = JUMP_VY;
	player.grounded = false;
    }
}

static void update_player(int* jump_detected) { // pass the jump+detected? 
    player.vy += GRAVITY;
    player.py += player.vy;
    if (player.py >= GROUND_Y - (PLAYER_ROWS * SCALE)) {
	player.py = GROUND_Y - (PLAYER_ROWS * SCALE);
	player.vy = 0;
	player.grounded = true;
    }
}

static void update_world() {
    for (int i = 0; i < MAX_CACTI; i++) {
	if (!cacti[i].on_screen) {
	    continue;
	}
	cacti[i].x -= world_speed;
	if (cacti[i].x < CACTUS_W * -1) {
	    cacti[i].on_screen = false;
	}
    }

    for (int i = 0; i < MAX_MANGOS; i++) {
	if (!mangos[i].on_screen || !mangos[i].active) {
	    continue;
	}
	mangos[i].x -= world_speed;
	if (mangos[i].x < MANGO_W * -1) {
	    mangos[i].on_screen = false;
	}
    }
}

static void spawn_test_cactus() {
    cacti[0].x = 400;
    cacti[0].on_screen = true;
}
 
static void spawn_test_mango(void) {
    mangos[0].x = 300;
    mangos[0].y = GROUND_Y - MANGO_Y - 2;
    mangos[0].active = true;
    mangos[0].on_screen = true;
}

static void game_reset(void) {
    frame_counter = 0;
    player.py = GROUND_Y - (PLAYER_ROWS * SCALE);
    player.vy = 0;
    player.grounded = true;
 
    for (int i = 0; i < MAX_CACTI;  i++) {
	cacti[i].on_screen  = false;
    }
    for (int i = 0; i < MAX_MANGOS; i++) {
	mangos[i].on_screen = false;
	mangos[i].active = false;
    }

    spawn_test_cactus();
    spawn_test_mango();
}

static void render() {
    gl_clear(COLOR_BACKGROUND);
    draw_ground();
    draw_player();
    draw_cactus();
    draw_mango();
    gl_swap_buffer();
}

static ps2_device_t *ps2_dev;

// void mango_run() {
//     init_colors();
//     ps2_dev = ps2_new(GPIO_PB3, GPIO_PB4); 
//     interrupts_global_enable();
//     gl_init(SCREEN_W, SCREEN_H, GL_DOUBLEBUFFER);
//     game_reset();

//     while (1) {
// 	unsigned int t0 = timer_get_ticks();
//         frame_counter++;
//         update_player();
//         update_world();
//         render();
// 	while (timer_get_ticks() - t0 < 10000);
//     }
// }


void mango_run() {
    init_colors();
    // uart_init();
    interrupts_init();
    i2c_device_t* acc = accel_init();
    // ps2_dev = ps2_new(GPIO_PB3, GPIO_PB4); 
    interrupts_global_enable();
    gl_init(SCREEN_W, SCREEN_H, GL_DOUBLEBUFFER);
    game_reset();
    int jump_detected = 0;
    int jump_done = 1;
    while (1) {
        unsigned long t0 = timer_get_ticks();
        
        if(jump_detected == 0){
          jump_detected = jumpDetected(acc);
        }
        // jump_detected = 0; // reset for next jump
        frame_counter++;
        update_player(&jump_detected); // start jump when jump detected is 1, and when jump complete on teh ground, clear the jump detector to zero
        update_world();
        render();
        printf("jump detected: %d, jump done: %d\n", jump_detected, jump_done);
        printf("time: %ld\n",(timer_get_ticks()-t0));
        while (timer_get_ticks() - t0 < 1200000);
        }
  }
