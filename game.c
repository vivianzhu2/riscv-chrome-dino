/* File name: game.c
 * File description: This file includes functions to implement a dinosaur-game inspired game where the user needs to jump to avoid obsticles. 
 * There are functions to draw the player, cacti, mangos, and the ground and it manages collision detection between them. It progresses the
 * game every frame and keeps track of score. When a collision is detected it renders the end screen and restarts after 5 seconds. It reads
 * input from an accelerometer to detect the jumps.
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
#include "uart.h"
#include "i2c.h"
#include "printf.h"
#include "duck.h"


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
#define JUMP_VY -25

static int  CACTI_RATE = 100;
static int  MANGO_RATE = 150;

static int world_speed = 5;
static int frame_counter = 0;	

static color_t COLOR_GROUND;
static color_t COLOR_BACKGROUND;
static color_t player_palette[4];
static color_t cactus_palette[2];
static color_t mango_palette[3];

static uint32_t score = 0; 
static uint32_t highScore = 0; 
static int  squatting = 0;

static void init_colors(void) {
    COLOR_GROUND = gl_color(0x3a, 0x3a, 0x3a);    // dark grey
    COLOR_BACKGROUND = gl_color(0x1a, 0x1a, 0x1a) ;    // grey
    player_palette[0] = COLOR_BACKGROUND;
    player_palette[1] = gl_color(0xff, 0xcc, 0xbc);    // skin
    player_palette[2] = gl_color(0xf4, 0x43, 0x36);    // shirt
    player_palette[3] = gl_color(0x37, 0x47, 0x4f);    // pants
    cactus_palette[0] = COLOR_BACKGROUND;
    cactus_palette[1] = gl_color(0x4c, 0xaf, 0x50);    // green
    mango_palette[0] = COLOR_BACKGROUND;
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

static const unsigned char player_squat_map[PLAYER_ROWS][PLAYER_COLS] = {
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0},
    {0,0,1,1,1,1,0,0},
    {0,0,1,0,0,1,0,0},
    {2,2,2,2,2,2,2,2},
    {0,3,3,3,3,3,3,0},
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
    int height;
} player_h;

typedef struct {
    int  x;
    bool on_screen;
} cactus_h;

typedef struct {
    int  x;
    int  y;
    bool on_screen;
} mango_h;


static player_h player;
static cactus_h cacti[MAX_CACTI];   // stores cacti on screen
static mango_h mangos[MAX_MANGOS];    // stores mangos on screen
const char *decimal_string(long val);

// draws a sprite with a scale at px, py by iterating over a sprite map and drawing the colors in the corresponding palette
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


// draws a horizontal line that acts as the ground
static void draw_ground() {
    gl_draw_rect(0, GROUND_Y, SCREEN_W, GROUND_THICK, COLOR_GROUND);
}


// draws player sprite in its current position
static void draw_player(){
    int py = player.py;
    int px = PLAYER_X;
    draw_sprite(px, py, &player_map[0][0], player.height, PLAYER_COLS, player_palette);
}


// draws squated version of player in its current position
static void draw_squat_player(){
    int py = player.py;
    int px = PLAYER_X;
    draw_sprite(px, py, &player_squat_map[0][0], player.height, PLAYER_COLS, player_palette);
}


// draws current score in upper right corner of the screen
static void draw_score() {
    const char* scorestr = decimal_string(score);
    char scoreScreen[64] = "SCORE: "; 
    int length = strlcat(scoreScreen, scorestr, sizeof(scoreScreen));
    gl_draw_string(gl_get_width() - (length * gl_get_char_width() + 15), 20, scoreScreen, GL_WHITE);

}


// draws all time high score under current score
static void draw_highScore(){
    const char* hscorestr = decimal_string(highScore);
    char hscoreScreen[64] = "HIGH SCORE: "; 
    int hlength = strlcat(hscoreScreen, hscorestr, sizeof(hscoreScreen));
    gl_draw_string(gl_get_width() - (hlength * gl_get_char_width() + 15), 20 + gl_get_char_height() + 10, hscoreScreen, GL_WHITE);
}


// draws all on screen cacti in their curent positions
static void draw_cactus() {
    int py = GROUND_Y - CACTUS_Y;
    for (int i = 0; i < MAX_CACTI; i++) {
	if (!cacti[i].on_screen) {
	    continue;
	}
	int px = cacti[i].x;
	draw_sprite(px, py, &cactus_map[0][0], CACTUS_ROWS, CACTUS_COLS, cactus_palette);
    }
}


// draws all on screen mangos in their current positions
static void draw_mango() {   
    int py =  GROUND_Y - MANGO_Y - PLAYER_ROWS - 20;// make the mangoes float above the ground so the player can squat underneath it
    for (int i = 0; i < MAX_MANGOS; i++) {
	if (!mangos[i].on_screen) {
	    continue;
        }
	int px = mangos[i].x;
	draw_sprite(px, py, &mango_map[0][0], MANGO_ROWS, MANGO_COLS, mango_palette);
    }
} 


// sets player's verticle velocity as the jump velocity if they are on the ground
static void jump() {    // no double jumps, only jumps when player is grounded
    if (player.grounded) {
        player.vy = JUMP_VY;
        player.py += 5; 
        player.grounded = false;
    }
}


// advances all obsticles, removes obsticles that go off screen, and spawns new cacti and mangos at respective rates
static void update_world() {
    for (int i = 0; i < MAX_CACTI; i++) {   // move cacti
	if (!cacti[i].on_screen) {
	    continue;
	}
	cacti[i].x -= world_speed;
	if (cacti[i].x < CACTUS_W * -1) {   // off screen
	    cacti[i].on_screen = false;
	}
    }

    for (int i = 0; i < MAX_MANGOS; i++) {   // move mangos
	if (!mangos[i].on_screen) {
	    continue;
	}
	mangos[i].x -= world_speed;
	if (mangos[i].x < MANGO_W * -1) {
	    mangos[i].on_screen = false;
	}
    }

    if (frame_counter % CACTI_RATE == 0) {    // spawn more cacti
        for (int i = 0; i < MAX_CACTI; i++) {
            if (!cacti[i].on_screen) {
                cacti[i].x = SCREEN_W;
                cacti[i].on_screen = true;
                break;
            }
        }
    }

    if (frame_counter % MANGO_RATE == 0) {    // spawn more mango
        for (int i = 0; i < MAX_MANGOS; i++) {
  	    if (!mangos[i].on_screen) {
                mangos[i].x = SCREEN_W;
                mangos[i].y = GROUND_Y - MANGO_Y;
                mangos[i].on_screen = true;
                break;
            }
        }
    }

}


// returns true if an nonzero/nonbackground color player pixel will advance into a cacti or mango pixel
static bool check_collision(int squat) {
    for (int x = 0; x < PLAYER_ROWS; x++) {
	for (int y = 0; y < PLAYER_COLS; y++) {
            if (squatting == 1) {
                if (player_squat_map[x][y] == 0){
                    continue;
                }
            } else {
                if (player_map[x][y] == 0){
                    continue;
                } 
            }
	    
	    int px = PLAYER_X + x * SCALE;
	    int py = player.py + y * SCALE;

	    color_t next_pixel = gl_read_pixel(px + world_speed, py);
            color_t green = gl_color(0x4c, 0xaf, 0x50);    // green
            color_t yellow = gl_color(0xff, 0x98, 0x00);    // mango
	    if (next_pixel == green) {
                return true;
	    }
        
	    if(next_pixel == yellow && !squatting){
                return true;
            }
	}
    }
    return false;
}


// resets all game states to their initial values for a new game
static void game_reset(void) {
    frame_counter = 0;
    player.py = GROUND_Y - (PLAYER_ROWS * SCALE);
    player.vy = 0;
    squatting = 0;
    player.grounded = true;
    player.height = PLAYER_COLS;
    draw_score();

    for (int i = 0; i < MAX_CACTI;  i++) {
	cacti[i].on_screen  = false;
    }
    for (int i = 0; i < MAX_MANGOS; i++) {
	mangos[i].on_screen = false;
    }
}


// clears the gramebuffer, draws all elements, and checks and returns if there was a collision
static bool render(int squat) {
    bool hit = check_collision(squat);
    gl_clear(COLOR_BACKGROUND);
    draw_ground();
    draw_cactus();
    draw_mango();
    draw_score();
    draw_highScore();
    if(squatting == 1){
        draw_squat_player();
    }
    else{
        draw_player();
    }

    gl_swap_buffer();
    return hit;
}


// displays game over screen with final score and reset message
static void render_end() {
    gl_clear(COLOR_GROUND);
    const char* endstr = "GAME OVER";
    const char* restartGame = "STARTING AGAIN IN 5 SECONDS";

    draw_score();
    draw_highScore();

    gl_draw_string((gl_get_width() - (strlen(endstr) * gl_get_char_width())) / 2, gl_get_height() / 2 - 10, endstr, GL_WHITE);
    gl_draw_string((gl_get_width() - (strlen(restartGame) * gl_get_char_width())) / 2, gl_get_height() / 2 + gl_get_char_height(), restartGame, GL_WHITE);
    gl_swap_buffer();
}    


// keeps the player in a squat for 1.5 seconds and returns true if there was a collision
static bool squat(){
    int duration = 1.5 * 24000000; 
    int start_time = timer_get_ticks();
    squatting = 1;
    bool hit = false;
    while(timer_get_ticks() - start_time < duration){
        frame_counter++;
        update_world();
        if(render(1)){
            hit = true;
            return true;
        }   // record hit but keep going
    }
    squatting = 0;
    return hit;  // report hit after squat fully completes
}


// if there was a jump, then draw jump action. applies gravity and updates player position
static int update_player(int* jump_detected) { // pass the jump+detected? 
    if (*jump_detected == 1 && player.grounded) {
        jump();
        player.grounded = false;
        *jump_detected = 0;
    }

    if(*jump_detected == 2 && player.grounded){
        bool squat_hit = squat();
        *jump_detected = 0;
        if(squat_hit){
            return -1;
        }   
    }

    player.vy += GRAVITY;
    player.py += player.vy;
    
    if (player.py >= GROUND_Y - (PLAYER_ROWS * SCALE)) {  // move player back to ground level if they went past
        player.py = GROUND_Y - (PLAYER_ROWS * SCALE);
        player.vy = 0;
        player.grounded = true;
    }
    return *jump_detected;
}


// initializes everything then runs the game loop which reads inputs, updates objects, renders, and handles game restarts
void mango_run() {
    gl_init(SCREEN_W, SCREEN_H, GL_DOUBLEBUFFER);
    init_colors();
    game_reset();
    i2c_device_t* acc = accelDuck_init();

    static int jump_detected = 0;
    while (1) {
        printf("LOOP TOP\n");
        unsigned long t0 = timer_get_ticks();
        
        int detected = duckDetected(acc);  // always read every frame
        if (detected != 0 && jump_detected == 0) {
            jump_detected = detected;      // only latch if nothing pending
        }
        
	frame_counter++;
	int player_result = update_player(&jump_detected);
        score = frame_counter * world_speed * 0.025;
	update_world();
        bool hit;
        if (player_result == -1) {
            hit = true;
        } else {
            hit = render(0);
        }
	
        if (hit) {
            int start = timer_get_ticks();
            if (score > highScore) {
               highScore = score;
            }
            render_end();
            unsigned int beg = timer_get_ticks();
            while (timer_get_ticks() - beg < 5 * 24000000);
            game_reset();
            jump_detected = 0;
            render(0);  

        }
        while (timer_get_ticks() - t0 < 10000);
    }
}
