
/**
 *
 * Tennix! SDL Port
 * Copyright (C) 2003, 2007, 2008, 2009 Thomas Perl <thp@thpinfo.com>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, 
 * MA  02110-1301, USA.
 *
 **/

#ifndef __GAME_H
#define __GAME_H

#include <math.h>
#include "tennix.h"
#include "sound.h"

#define SETS_TO_WIN 1

#define NGRAM_STEPS 6

#ifdef DELUXE_EDITION
#  define BALL_STATES 16
#else
#  define BALL_STATES 4
#endif

typedef unsigned char bool;
enum {
    false,
    true
};

#ifdef EXTENDED_REFEREE
enum {
    REFEREE_NORMAL,
    REFEREE_OUT,
    REFEREE_PLAYER1,
    REFEREE_PLAYER2,
    REFEREE_COUNT
};
#else
enum {
    REFEREE_NORMAL,
    REFEREE_PLAYER1,
    REFEREE_OUT,
    REFEREE_PLAYER2,
    REFEREE_COUNT
};
#endif

enum {
    WINNER_NONE,
    WINNER_PLAYER1,
    WINNER_PLAYER2
};

typedef struct {
    float x;
    float y;
    float move_x;
    float move_y;
    float jump;
} Ball;

typedef struct {
    float x;
    float y;
    unsigned char state;
    unsigned int score;
    bool responsible; /* responsible for the next fault (if any), When player hit the ball it becomes his responsiblity */
    unsigned char desire; /* what the player aims to do (0=normal, 1=upper edge, 2=lower edge)*/
    bool type; /* is this player ai-controlled or human? */
    float ball_dest; /* prospective y-position of ball */
    bool state_locked; /* enabled when user keeps pressing the "hit" key */
    int game; /* score for the current game */
    int sets[SETS_TO_WIN*2]; /* score for each set */
    int mouse_x; /* x position of mouse */
    int mouse_y; /* y position of mouse */
    float accelerate; /* a value [0..1] how fast the user accelerates */
    bool mouse_locked; /* on start, ignore unpressed mouse state */
    int number_of_hits; /* Count the number of times the plaer hits the ball */
    int point_count; /* Count of every win, this increments every time player scores */
    float ball_proximity; /* figure  of proximity to ball */
    unsigned int ball_proximity_count; /* count of times the ball proximity was measured, for averages */
} Player;

enum {
    PLAYER_TYPE_HUMAN,
    PLAYER_TYPE_AI,
    PLAYER_TYPE_DARWIN
};

enum {
    DESIRE_NORMAL,
    DESIRE_MAX
};

typedef struct {
    Ball ball;
    Ball ground;
    Player player1;
    Player player2;
    float phase;
    unsigned int time;
    bool was_stopped;
    bool player1_serves;
    char* status;
    char game_score_str[50];
    char sets_score_str[50];
    unsigned char referee;
    unsigned int current_set;
    int winner;
    bool is_over;
    unsigned int court_type;
    unsigned int old_court_type;
    unsigned int history[3];
    unsigned int history_size;
    bool history_is_locked;
    unsigned char ngram[NGRAM_STEPS][NGRAM_STEPS][NGRAM_STEPS];
    float ngram_prediction;
    sound_id play_sound;
    float joystick_y;
    float joystick_x;
    unsigned char joystick_a;
    unsigned int rain;
    unsigned int fog;
    bool night;
    int wind;
    unsigned int windtime;
    int timelimit;
    bool text_changed;
    unsigned char old_referee;
} GameState;

#define PI 3.1415

#define GAME_TICKS 15

#define GROUND_PHASE 0.4
#define PHASE_AMP 3.2

#define BALL_JUMP_MIN 4
#define BALL_JUMP_MAX 10

#define RACKET_X_MID 15
#define RACKET_Y_MID 24

#define BALL_X_MID 9
#define BALL_Y_MID 9

#define GAME_X_MIN 41.0*2
#define GAME_X_MAX 270.0*2
#define GAME_X_MID ((GAME_X_MIN+GAME_X_MAX)/2)

#define GAME_Y_MIN 155.0
#define GAME_Y_MAX 330.0
#define GAME_Y_MID ((GAME_Y_MIN+GAME_Y_MAX)/2)

#define GAME_EDGE_AREA 20.0
#define GAME_EDGE_UPPER GAME_Y_MIN+GAME_EDGE_AREA
#define GAME_EDGE_LOWER GAME_Y_MAX-GAME_EDGE_AREA

#define PLAYER_Y_MIN 0.0
#define PLAYER_Y_MAX 1.0*HEIGHT

#define IS_OFFSCREEN_Y(y) (((y)<0-BALL_Y_MID*2) || ((y)>HEIGHT+BALL_Y_MID*2))
#define IS_OFFSCREEN_X(x) (((x)<0-BALL_X_MID*2) || ((x)>WIDTH+BALL_X_MID*2))
#define IS_OFFSCREEN(x,y) ((IS_OFFSCREEN_X(x)) || (IS_OFFSCREEN_Y(y)))
#define IS_OUT_Y(y) (y<GAME_Y_MIN || y>GAME_Y_MAX)
#define IS_OUT_X(x) (x<GAME_X_MIN || x>GAME_X_MAX)

#define PLAYER_AREA_Y RACKET_Y_MID
#define PLAYER_AREA_X RACKET_X_MID
#define IS_NEAR_Y(py,by) (fabsf(py-by)<PLAYER_AREA_Y)
#define IS_NEAR_Y_AI(py,by) (fabsf(py-by)<PLAYER_AREA_Y/3.5)
#define IS_NEAR_X(px,bx) (fabsf(px-bx)<PLAYER_AREA_X)
#define IS_NEAR_X_AI(px,bx) (fabsf(px-bx)<PLAYER_AREA_X*2)

#define PLAYER_MOVE_Y 5.0
#define PLAYER_ACCEL_DEFAULT 0.2
#define PLAYER_ACCEL_INCREASE 1.2
#define PLAYER_STATE_MAX 7
#define PLAYER_POWERSHOT 6.2

#define MOVE_Y_SEED 3

/* Comment out the following #define to enable mouse control */
#define ENABLE_MOUSE

/* GameState handling*/
GameState *gamestate_new();


/* Game module functions */
void gameloop(GameState*);
void render( GameState*);
bool step( GameState*);
void limit_value( float*, float, float);
float get_phase( GameState*);
float get_move_y( GameState*, unsigned char);
void input_human( Player*, bool, bool, bool, bool, GameState*);
void input_ai( Player*, Ball*, Player*, GameState*);
void game_setup_serve( GameState*);
float ngram_predictor( GameState*);

void score_game( GameState*, bool);
char* format_sets( GameState*);
char* format_game( GameState*);
char* format_status( GameState*);
int game_get_winner( GameState*);
void extract_game_data(GameState *s);
float calculate_fitness(int player, GameState *s);

#endif

