
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
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <sys/timeb.h>

#include "tennix.h"
#include "game.h"
#include "graphics.h"
#include "input.h"
#include "sound.h"
#include "tennix_server.h"


time_t time_now;
struct timeb tp;

int display_on = 0;

#define TIME //ftime(&tp);printf("%s:%d: Time now = %1d.%d\n", __FUNCTION__, __LINE__,tp.time,tp.millitm )


GameState *gamestate_new() {
    int x, y, z;
    GameState *s;

    GameState template = {
        { 0, 0, 0.0, 0.0, 0.0 },
        { 0, 0, 0, 0, 0 },
        { GAME_X_MIN-RACKET_X_MID*2, GAME_Y_MID, 0, 0, 0, DESIRE_NORMAL, PLAYER_TYPE_HUMAN, GAME_Y_MID, false, 0, {0}, 0, 0, PLAYER_ACCEL_DEFAULT, true, 0/* Racquet hit count*/, 0 /*point*/, 0 , 0},
        { GAME_X_MAX+RACKET_X_MID*2, GAME_Y_MID, 0, 0, 1, DESIRE_NORMAL, PLAYER_TYPE_AI, GAME_Y_MID, false, 0, {0}, 0, 0, PLAYER_ACCEL_DEFAULT, true, 0/* Racquet hit count*/, 0 /* point */, 0, 0},
        0,
        0,
        0,
        false,
        "welcome to tennix " VERSION,
        { 0 },
        { 0 },
        REFEREE_NORMAL,
        0,
        WINNER_NONE,
        false,
        GR_CTT_GRASS,
        -1,
        { 0 },
        0,
        false,
        { { { 0 } } },
        0.0,
        SOUND_MAX,
        0.0,
        0.0,
        0,
        0,
        0,
        false,
        0,
        0,
        0,
        false,
        REFEREE_COUNT
    };

    s = (GameState*)malloc(sizeof(GameState));
    if (s == NULL) abort();

    memcpy(s, &template, sizeof(GameState));

    /* Setup player1 fittness params */
    s->player1.ball_proximity= 0;
    s->player1.ball_proximity_count = 0;
    s->player1.number_of_hits = 0;
    s->player1.point_count = 0;

    game_setup_serve(s);

    /* smoothen n-gram */
    for( x = 0; x<NGRAM_STEPS; x++) {
        for( y = 0; y<NGRAM_STEPS; y++) {
            for( z = 0; z<NGRAM_STEPS; z++) {
                s->ngram[x][y][z] = 1;
            }
        }
    }

    return s;
}

#define HIT_WEIGHTAGE        1000
#define POINT_WEIGHTAGE      1500
#define PROXIMITY_WEIGHTAGE  0.3 

float calculate_fitness(int player, GameState *s)
{
    switch (player) {

        case PLAYER1:
            return ((s->player1.number_of_hits * HIT_WEIGHTAGE) + (s->player1.point_count * POINT_WEIGHTAGE) - (PROXIMITY_WEIGHTAGE * s->player1.ball_proximity));

        case PLAYER2:
            return ((s->player2.number_of_hits * HIT_WEIGHTAGE) + (s->player2.point_count * POINT_WEIGHTAGE) - (PROXIMITY_WEIGHTAGE * s->player2.ball_proximity));
    }
}

void extract_game_data(GameState *s)
{
//    printf("Ball        X = %f \t Y=%f\n", s->ball.x, s->ball.y);
      printf("Proximity Avg = %f \n", s->player1.ball_proximity/s->player1.ball_proximity_count);
//    printf("Opponent    X = %f \t Y=%f\n", s->player2.x, s->player2.y);
//    printf("Delta = %f\n", s->player2.x - s->ball.x);
      printf("Hit count = %d\n", s->player1.number_of_hits);
      printf(" Player point = %d, Opponent point = %d\n", s->player1.point_count, s->player2.point_count);
      printf("Fitness PLAYER1 = %f Fitness PLAYER2 = %f\n", calculate_fitness(PLAYER1, s), calculate_fitness(PLAYER2, s));
}


void gameloop(GameState *s) {
    strcpy(s->game_score_str, format_game(s));
    strcpy(s->sets_score_str, format_sets(s));
    s->text_changed = true;

    Uint32 ot = SDL_GetTicks();
    Uint32 nt;
    Uint32 dt = GAME_TICKS;
    Uint32 diff;
    Uint32 accumulator = 0;
    bool quit = false;

#ifdef ENABLE_FPS_LIMIT
    Uint32 ft, frames; /* frame timer and frames */
#endif

    if (s->rain > 0) {
        play_sample_background(SOUND_RAIN);
    }
    play_sample_loop(SOUND_AUDIENCE);
    /* Reset the court type, so it is redrawn on first display */
    s->old_court_type = -1;

#ifdef ENABLE_FPS_LIMIT
    frames = 0;
    ft = SDL_GetTicks();
#endif
    while( !quit ) {
#ifdef GRAPHICS
        if (display_on) {
            TIME;
            nt = SDL_GetTicks();
            TIME;
            diff = nt-ot;
            if( diff > 2000) {
                diff = 0;
            }

            accumulator += diff;
            ot = nt;
            while( accumulator >= dt) {
#endif
                quit = step(s);
                //extract_game_data(s);
#ifdef GRAPHICS
                s->time += dt;
                s->windtime += s->wind*dt;
                accumulator -= dt;
                if( s->was_stopped) {
                    ot = SDL_GetTicks();
                    s->was_stopped = false;
                }
            }
        } else {
            quit = step(s);
        }
#endif
        if (s->timelimit != 0 && s->time >= s->timelimit) {
            quit = 1;
        }

#ifdef ENABLE_FPS_LIMIT
        while (frames*1000.0/((float)(SDL_GetTicks()-ft+1))>(float)(DEFAULT_FPS)) {
            SDL_Delay(10);
        }
        frames++;
#endif
        if (display_on) {
            render(s);
        }
    }

    extract_game_data(s);

    //clear_screen();
    //store_screen();

    //stop_sample(SOUND_AUDIENCE);
    //stop_sample(SOUND_RAIN);
}
// X min is 82 and X max is 540
#define X_CLOSE_TO_PLAYER1 70
#define X_CLOSE_TO_PLAYER2 550
#define FALSE 0
#define TRUE  1

void update_players_dist_from_ball(GameState* s)
{
    /*
     * NOTE: The downside of having high weightage for ball proximity is
     * it will reward shots from center of bat, so make sure other rewards like
     * hit and point is very high
     */
    static int player1_measure_to_be_taken = TRUE; /* Take proximity measure once, for each ball coming out way */
    static int player2_measure_to_be_taken = TRUE; /* Take proximity measure once, for each ball coming out way */


    if ((player1_measure_to_be_taken == TRUE) && (s->ball.x < X_CLOSE_TO_PLAYER1)) {
        unsigned int ball_proximity_count = s->player1.ball_proximity_count;
        float ball_proximity = s->player1.ball_proximity;

        /* Update the Average for player1 */
        ball_proximity = ( float )((ball_proximity * ball_proximity_count) + ((s->ball.y - s->player1.y) * (s->ball.y - s->player1.y)))/(ball_proximity_count + 1); /* Sq root to get mean square */

        //printf("1: Ball Y diff = %f Old Average = %f   New Average = %f\n", (s->ball.y - s->player1.y), s->player1.ball_proximity, ball_proximity);

        
        s->player1.ball_proximity_count++;
        s->player1.ball_proximity = ball_proximity;
        player1_measure_to_be_taken = FALSE;

    }

    if ((player2_measure_to_be_taken == TRUE) && (s->ball.x > X_CLOSE_TO_PLAYER2)) {
        unsigned int ball_proximity_count = s->player2.ball_proximity_count;
        float ball_proximity = s->player2.ball_proximity;

        /* Update the Average for player2 */
        ball_proximity = ((ball_proximity * ball_proximity_count) + ((s->ball.y - s->player2.y) * (s->ball.y - s->player2.y)))/(ball_proximity_count + 1);

        //printf("2: Ball Y diff = %f Old Average = %f   New Average = %f\n", s->ball.y - s->player2.y,  s->player2.ball_proximity, ball_proximity);
        
        s->player2.ball_proximity_count++;
        s->player2.ball_proximity = ball_proximity;
        player2_measure_to_be_taken = FALSE;

    }

    //printf(" GAME SCORE: %s   SET SCORE: %s\n", s->game_score_str, s->sets_score_str);
    //printf("Fitness PLAYER1 = %d Fitness PLAYER2 = %d\n", calculate_fitness(PLAYER1, s), calculate_fitness(PLAYER2, s));
    //printf(" Player point = %d, Opponent point = %d\n", s->player1.point_count, s->player2.point_count);


    if ((player1_measure_to_be_taken == FALSE) && (s->ball.x > X_CLOSE_TO_PLAYER1 + 10))
    {
        // Make sure ball is hit, This is to ensure we dont measure many times while 
        // being served
        player1_measure_to_be_taken = TRUE;
    }

    if ((player2_measure_to_be_taken == FALSE) && (s->ball.x < X_CLOSE_TO_PLAYER2 - 10))
    {
        // Make sure ball is hit, This is to ensure we dont measure many times while 
        // being served
        player2_measure_to_be_taken = TRUE;
    }
}

void set_player_to_always_serve(int player, GameState* s)
{
    switch(player) {

        case PLAYER1:
            s->player1_serves = true;
            break;

        case PLAYER2:
            s->player1_serves = false;
    }
}


bool step( GameState* s) 
{
    Uint8 *keys;
    SDL_Event e;

    update_players_dist_from_ball(s); /* Keep the statistics of player closeness to ball */
   
    if( get_phase( s) < 1.0) {
        if( !s->ground.jump) {
            s->play_sound = SOUND_GROUND;

            if( IS_OUT_Y( s->ball.y)) {
                /* out - responsibilities stay the same */
                s->status = "out!";
                s->play_sound = SOUND_OUT;
                s->referee = REFEREE_OUT;
            } else {
                /* not out - responsibilities change */
                s->player1.responsible = !(s->player2.responsible = !s->player2.responsible);
                s->status = format_status( s);
                s->referee = REFEREE_NORMAL;
            }
        }
        s->ground.jump = 3;
        s->ground.x = s->ball.x;
        s->ground.y = s->ball.y;
    } else {
        if( s->ground.jump && !(s->time%5)) s->ground.jump--;
    }

    if( IS_OUT_X(s->ball.x) || IS_OFFSCREEN_Y(s->ball.y)) {
        /* Ball has gone out */
        if( IS_OFFSCREEN( s->ball.x, s->ball.y)) 
        {
            /* Ball is offscreen */
            s->player1_serves = s->player1.responsible;// If player1 hit the shot give him the serve

            /*
             * To facililtate learning, always make AI player to serve 
             * But points calculations are as before, no change there
             */
            set_player_to_always_serve(PLAYER2, s);

            score_game( s, s->player2.responsible);
            strcpy( s->game_score_str, format_game( s));
            strcpy( s->sets_score_str, format_sets( s));
            s->text_changed = true;

            if( s->player1.responsible) {
                if( (s->player1.type == PLAYER_TYPE_HUMAN && s->player2.type == PLAYER_TYPE_AI) ||
                        (s->player1.type == PLAYER_TYPE_DARWIN && s->player2.type == PLAYER_TYPE_AI)) {
                    s->status = "computer scores";
                } else {
                    s->status = "player 2 scores";
                }
                s->referee = REFEREE_PLAYER2;
            } else {
                if( (s->player1.type == PLAYER_TYPE_HUMAN && s->player2.type == PLAYER_TYPE_AI) ||
                        (s->player1.type == PLAYER_TYPE_DARWIN && s->player2.type == PLAYER_TYPE_AI)) {
                    s->status = "player scores";
                } else {
                    s->status = "player 1 scores";
                }
                s->referee = REFEREE_PLAYER1;
            }

            game_setup_serve( s);
            s->play_sound = SOUND_APPLAUSE;
#ifdef GRAPHICS
            if (display_on){
                SDL_Delay( 500);
            }
#endif
            s->was_stopped = true;
            s->history_size = 0;
            s->history_is_locked = 0;
            s->ngram_prediction = 0.0;
#ifdef DEBUG
            printf( "-- game reset --\n");
#endif
        }

        if( IS_OUT_X(s->ball.x)) {
            if( !s->history_is_locked && s->referee != REFEREE_OUT) {
                s->history[s->history_size] = (int)(NGRAM_STEPS*s->ball.y/HEIGHT);
                s->history_size++;
                if( s->history_size == 3) {
                    s->ngram[s->history[0]][s->history[1]][s->history[2]] += 10;
#ifdef DEBUG
                    printf( "history: %d, %d, %d\n", s->history[0], s->history[1], s->history[2]);
#endif
                    s->ngram_prediction = ngram_predictor( s);
                    s->history[0] = s->history[1];
                    s->history[1] = s->history[2];
                    s->history_size--;
                }
                s->history_is_locked = true;
            }
            if( s->ball.move_x <= 0 && IS_NEAR_X( s->player1.x, s->ball.x) && IS_NEAR_Y( s->player1.y, s->ball.y) && s->player1.state && s->referee != REFEREE_OUT) {
                /* Player 1 hit the ball */
                s->ball.x = GAME_X_MIN;
                if( s->player1.state == PLAYER_STATE_MAX) {
                    s->ball.move_x = PLAYER_POWERSHOT;
                } else {
                    s->ball.move_x = 2.5 + 2.0*s->player1.state/PLAYER_STATE_MAX;
                }
                s->ball.move_y = get_move_y( s, 1);
                /* Make Player 1 responsible for this shot */
                s->player2.responsible = !(s->player1.responsible = 1);
                s->ball.jump += 1.0-2.0*(s->player1.state<5);
                s->play_sound = SOUND_RACKET;
                pan_sample(SOUND_RACKET, 0.4);
                s->player1.number_of_hits++; // Every time player hits the ball , count it
                /* Dump the hit count to a separate FILE */
                char buf[50];
                snprintf(buf, 50, "echo \"Hit count = %d Points: %d\" >> hit_count", s->player1.number_of_hits, s->player1.point_count);
                system (buf);
                /* END of hit count dump */
            } else if( s->ball.move_x >= 0 && IS_NEAR_X( s->player2.x, s->ball.x) && IS_NEAR_Y( s->player2.y, s->ball.y) && s->player2.state && s->referee != REFEREE_OUT) {
                /* Player 2 hit the ball */
                s->ball.x = GAME_X_MAX;
                if( s->player2.state == PLAYER_STATE_MAX) {
                    s->ball.move_x = -PLAYER_POWERSHOT;
                } else {
                    s->ball.move_x = -(2.5 + 2.0*s->player2.state/PLAYER_STATE_MAX);
                }
                s->ball.move_y = get_move_y( s, 2);
                /* Now make player 2 responsible for this shot */
                s->player1.responsible = !(s->player2.responsible = 1);
                s->ball.jump += 1.0-2.0*(s->player2.state<5);
                s->play_sound = SOUND_RACKET;
                s->player2.number_of_hits++;
                pan_sample(SOUND_RACKET, 0.6);
            }
        }
    } else {
        s->history_is_locked = false;
    }
#ifdef JOYSTICK
    SDL_PollEvent( &e);
#endif

    keys = SDL_GetKeyState( NULL);
    /* Point to plug in neural network input */
    if (s->player1.type == PLAYER_TYPE_DARWIN)
    {
        int nn_input[3];
        server_get_input_from_NN(s->player2.x, s->player2.y, s->ball.x, s->ball.y, nn_input);
        if ( nn_input[UP] > 0 )
        {
            //printf (" NN said __UP__\n");
            keys['w'] = 1;
        } else {
            keys['w'] = 0;
        }

        if (nn_input[DOWN] > 0 )
        {
            //printf (" NN said __DOWN__\n");
            keys['s'] = 1;
        } else {
            keys['s'] =  0;
        }

        if (nn_input[HIT] > 0 )
        {
            //printf (" NN said __HIT__\n");
            keys['d'] = 1;
        } else {
            keys['d'] = 0;
        }

    /* End of NN input */
    }



#ifdef JOYSTICK
    switch(e.type) {
        case SDL_JOYAXISMOTION:
            if (e.jaxis.axis == JOYSTICK_Y_AXIS) {
                s->joystick_y = JOYSTICK_PERCENTIZE(e.jaxis.value);
            } else if (e.jaxis.axis == JOYSTICK_X_AXIS) {
                s->joystick_x = JOYSTICK_PERCENTIZE(e.jaxis.value);
            }
            break;
        case SDL_JOYBUTTONUP: case SDL_JOYBUTTONDOWN:
            if (e.jbutton.button == JOYSTICK_BUTTON_A) {
                s->joystick_a = (e.jbutton.state == SDL_PRESSED);
            }
            break;
    }
#endif /* JOYSTICK */

#ifdef GRAPHICS
if(display_on) {
    if( s->time%50==0) {
        /**
         * Maemo keys:
         * F7 = "Decrease" key
         * F8 = "Increase" key
         **/
        if (keys['c'] || keys[SDLK_F8]) {
            s->court_type++;
        } else if (keys[SDLK_F7]) {
            s->court_type--;
        }
        if (keys['r']) {
            if (s->rain == 0) {
                play_sample_background(SOUND_RAIN);
            }
            s->rain += 10;
        }
        if (keys['t']) {
            s->fog++;
        }
        if (keys['1']) {
            s->wind++;
        }
        if (keys['2']) {
            s->wind--;
        }
        if (keys['n']) {
            s->night = 1 - s->night;
        }
        if( s->court_type > GR_CTT_LAST) {
            s->court_type = GR_CTT_FIRST;
        } else if (s->court_type < GR_CTT_FIRST) {
            s->court_type = GR_CTT_LAST;
        }
    }
}
#endif

    if(/*!(SDL_GetTicks() < fading_start+FADE_DURATION) &&*/ !s->is_over) { // Part of graphics 
        if( s->player1.type == PLAYER_TYPE_HUMAN || s->player1.type == PLAYER_TYPE_DARWIN ) {
           input_human( &s->player1,
                   keys['w'] || keys[SDLK_UP] || s->joystick_y < -JOYSTICK_TRESHOLD,
                   keys['s'] || keys[SDLK_DOWN] || s->joystick_y > JOYSTICK_TRESHOLD,
                   keys['d'] || keys[SDLK_SPACE] || keys[SDLK_LCTRL] || keys[SDLK_RETURN] || s->joystick_a,
#ifdef ENABLE_MOUSE
                   true,
#else
                   false,
#endif
                   s);
        } else {
            input_ai( &s->player1, &s->ball, &s->player2, s);
        }
 
        if( s->player2.type == PLAYER_TYPE_HUMAN) {
            input_human( &s->player2, keys['o'], keys['l'], keys['k'], false, s);
        } else {
            input_ai( &s->player2, &s->ball, &s->player1, s);
        }
    }
    
    /* Maemo: The "F6" button is the "Fullscreen" button */
    if( keys['f'] || keys[SDLK_F6]) SDL_WM_ToggleFullScreen( screen);
    if( keys['y']) SDL_SaveBMP( screen, "screenshot.bmp");

    /* Maemo: The "F4" button is the "Open menu" button */
    if( keys['p'] || keys[SDLK_F4]) {
        while( keys['p'] || keys[SDLK_F4]) {
            SDL_PollEvent( &e);
            keys = SDL_GetKeyState( NULL);
            SDL_Delay( 10);
        }
        while( (keys['p'] || keys[SDLK_F4]) == 0) {
            SDL_PollEvent( &e);
            keys = SDL_GetKeyState( NULL);
            SDL_Delay( 10);
        }
        while( keys['p'] || keys[SDLK_F4]) {
            SDL_PollEvent( &e);
            keys = SDL_GetKeyState( NULL);
            SDL_Delay( 10);
        }
        s->was_stopped = true;
    }
     
    if( keys[SDLK_ESCAPE] || keys['q'] || (s->winner != WINNER_NONE) ) return true; // sachins : added to make finish criterion
    
    limit_value( &s->player1.y, PLAYER_Y_MIN, PLAYER_Y_MAX);
    limit_value( &s->player2.y, PLAYER_Y_MIN, PLAYER_Y_MAX);
    limit_value( &s->ball.jump, BALL_JUMP_MIN, BALL_JUMP_MAX);
    
    /* Update ball_dest for debugging purposes */
    get_move_y(s, 1);
    get_move_y(s, 2);

    if (s->ball.move_x > 0 && s->wind > 0) {
        s->ball.x += fabsf(s->wind)/2;
    } else if (s->ball.move_x < 0 && s->wind < 0) {
        s->ball.x -= fabsf(s->wind)/2;
    }

    s->ball.x += s->ball.move_x;
    s->ball.y += s->ball.move_y;

    if(s->player1.state) s->player1.state--;
    if(s->player2.state) s->player2.state--;

    return false;
}

void render( GameState* s) {
    int i, x, y;
#ifdef EXTENDED_REFEREE
    int t=1000;
#endif
    if (s->play_sound != SOUND_MAX) {
        play_sample(s->play_sound);
        s->play_sound = SOUND_MAX;
    }
    if( s->winner != WINNER_NONE) {
        if( !s->is_over) {
#ifdef GRAPHICS
            if (display_on) {
                start_fade();
            }
#endif
            s->is_over = true;
        }
#ifdef GRAPHICS
        if (display_on)
        {
            clear_screen();
            store_screen();
            show_sprite( GR_RACKET, 2*(s->winner-1), 4, WIDTH/2 - get_image_width( GR_RACKET)/8, HEIGHT/2 - get_image_height( GR_RACKET), 255);
        }
#endif
        sprintf( s->game_score_str, "player %d wins the match with %s", s->winner, format_sets( s));
#ifdef GRAPHICS
        if(display_on) {
            font_draw_string( GR_DKC2_FONT, s->game_score_str, (WIDTH-font_get_string_width( GR_DKC2_FONT, s->game_score_str))/2, HEIGHT/2 + 30, s->time/20, ANIMATION_WAVE | ANIMATION_BUNGEE);
            updatescr();
        }
#endif
        return;
    }
#ifdef GRAPHICS
if(display_on) {
    if (s->old_court_type != s->court_type || s->text_changed || s->old_referee != s->referee) {
        clear_screen();
        fill_image(s->court_type, 120, 120, 400, 250);
        show_image(GR_COURT, 0, 0, 255);
        font_draw_string( GR_DKC2_FONT, s->game_score_str, 14, 14, 0, ANIMATION_NONE);
        font_draw_string( GR_DKC2_FONT, s->sets_score_str, (WIDTH-font_get_string_width( GR_DKC2_FONT, s->sets_score_str))-14, 14, 0, ANIMATION_NONE);
#ifdef EXTENDED_REFEREE
        switch (s->referee) {
            case REFEREE_NORMAL:
                t = 1000;
                break;
            case REFEREE_OUT:
                t = 200;
                break;
            case REFEREE_PLAYER1:
            case REFEREE_PLAYER2:
                t = 400;
                break;
        }
        t = (s->time/t)%4;
        switch (t) {
            case 0:
                t=0;
                break;
            case 1:
                t=1;
                break;
            case 2:
                t=0;
                break;
            case 3:
                t=2;
                break;
        }
        show_sprite( GR_REFEREE, s->referee*3+t, 12, 250, 10, 255);
        if (voice_finished_flag == 0) {
            show_sprite(GR_TALK, (s->time/150)%2, 2, 280, 45, 255);
        }
#else
        show_sprite( GR_REFEREE, s->referee, 4, 250, 10, 255);
#endif
        s->old_court_type = s->court_type;
        s->text_changed = false;
        s->old_referee = s->referee;
        store_screen();
    }
    show_image( GR_SHADOW, s->ball.x-BALL_X_MID, s->ball.y + get_phase( s) - BALL_Y_MID, 255);
    
    show_sprite( GR_RACKET, (!s->player1.state), 4, s->player1.x-RACKET_X_MID, s->player1.y-RACKET_Y_MID, 255);
    show_sprite( GR_RACKET, (!s->player2.state)+2, 4, s->player2.x-RACKET_X_MID, s->player2.y-RACKET_Y_MID, 255);
    
    if( s->ball.move_x > 0) {
        show_sprite( GR_BALL, (s->time/100)%BALL_STATES, BALL_STATES, s->ball.x-BALL_X_MID, s->ball.y-BALL_Y_MID, 255);
    } else if( s->ball.move_x < 0) {
        show_sprite( GR_BALL, BALL_STATES-1-(s->time/100)%BALL_STATES, BALL_STATES, s->ball.x-BALL_X_MID, s->ball.y-BALL_Y_MID, 255);
    } else {
        show_sprite( GR_BALL, 0, BALL_STATES, s->ball.x-BALL_X_MID, s->ball.y-BALL_Y_MID, 255);
    }

    /* Player 1's mouse rectangle */
    if (!(s->player1.mouse_locked)) {
        rectangle(s->player1.x-2+5, s->player1.mouse_y-2, 4, 4, 255, 255, 255);
        rectangle(s->player1.x-1+5, s->player1.mouse_y-1, 2, 2, 0, 0, 0);
    }
    

    font_draw_string( GR_DKC2_FONT, s->status, (WIDTH-font_get_string_width( GR_DKC2_FONT, s->status))/2, HEIGHT-50, s->time/30, ANIMATION_WAVE);

    for (i=0; i<s->rain; i++) {
        x = rand()%WIDTH;
        y = rand()%HEIGHT;
        draw_line_faded(x, y, x+10+s->wind*5, y+30, 0, 0, 255, 100, 200, 255);
    }
    if (s->rain) {
        /**
         * Cheap-ish update of the whole screen. This can
         * probably be optimized.
         **/
        update_rect(0, 0, WIDTH, HEIGHT);
    }

#ifdef DEBUG
    line_horiz( s->player1.y, 255, 0, 0);
    line_horiz( s->player2.y, 0, 0, 255);
    line_horiz( s->ball.y, 0, 255, 0);

    line_vert( s->player1.x, 255, 0, 0);
    line_vert( s->player2.x, 0, 0, 255);
    line_vert( s->ball.x, 0, 255, 0);

    line_horiz( s->player1.ball_dest, 255, 0, 255);
    line_horiz( s->player2.ball_dest, 0, 255, 255);

    line_horiz( GAME_Y_MIN, 100, 100, 100);
    line_horiz( GAME_Y_MAX, 100, 100, 100);
#endif
    switch (s->fog) {
        default:
        case 4:
            fill_image_offset(GR_FOG2, 0, 0, WIDTH, HEIGHT, -s->time/150-s->windtime/200, 0);
        case 3:
            fill_image_offset(GR_FOG, 0, 0, WIDTH, HEIGHT, -s->time/100-s->windtime/150, 20);
        case 2:
            fill_image_offset(GR_FOG2, 0, 0, WIDTH, HEIGHT, -s->time/180-s->windtime/180, 80);
        case 1:
            fill_image_offset(GR_FOG, 0, 0, WIDTH, HEIGHT, s->time/200-s->windtime/100, 0);
        case 0:
            break;
    }
    if (s->night) {
        show_image(GR_NIGHT, 0, 0, 255);
    }

    updatescr();
}
#endif /* GRAPHICS */
}

void limit_value( float* value, float min, float max) {
    if( *value < min) {
        *value = min;
    } else if( *value > max) {
        *value = max;
    }
}

float get_phase( GameState* s) {
    float pos, fract;
    float x, min, max, direction;

    x = s->ball.x;
    min = GAME_X_MIN;
    max = GAME_X_MAX;
    direction = s->ball.move_x;

    pos = (direction>0)?(1-GROUND_PHASE):(GROUND_PHASE);

    fract = (x-min)/(max-min);

    if( fract < pos) {
        fract = fract/pos;
        return fabsf( cosf(PI*fract/2))*PHASE_AMP*s->ball.jump;
    } else {
        fract = (pos-fract)/(1-pos);
        return fabsf( sinf(PI*fract/2))*PHASE_AMP*s->ball.jump;
    }
}

float get_move_y( GameState* s, unsigned char player) {
    float pct, dest, x_len, y_len;
    float py, by, pa, move_x;

    py = (player==1)?(s->player1.y):(s->player2.y);
    by = s->ball.y;
    pa = RACKET_Y_MID*2;
    move_x = s->ball.move_x;

    /* -1.0 .. 1.0 for racket hit position */
    pct = (by-py)/(pa/2);
    limit_value( &pct, -1.0, 1.0);

    /* Y destination for ball */
    dest = GAME_Y_MID + pct*(GAME_Y_MAX-GAME_Y_MIN);
    if( player == 1) {
        s->player1.ball_dest = dest;
    } else {
        s->player2.ball_dest = dest;
    }

    /* lengths for the ball's journey */
    if( player == 1) {
        x_len = fabsf(GAME_X_MAX - s->ball.x);
        y_len = dest - by + MOVE_Y_SEED-rand()%MOVE_Y_SEED*2;
    } else {
        x_len = s->ball.x - GAME_X_MIN;
        y_len = by - dest + MOVE_Y_SEED-rand()%MOVE_Y_SEED*2;
    }

    /* return the should-be value for move_y */
    return (y_len*move_x)/(x_len);
}

void input_human( Player* player, bool up, bool down, bool hit, bool use_mouse, GameState* s) 
{
    int diff = PLAYER_MOVE_Y;
    int mb;

#ifdef MOUSE
    /**
     * Only use mouse control if the user isn't pressing any buttons
     * this way, keyboard control still works when mouse control is
     * enabled.
     **/
    if (use_mouse && (down || up || hit)) {
        /**
         * this is here so if the user decides to play
         * with keyboard controls, we will lock the 
         * mouse and disable displaying the mouse cursor
         **/
        player->mouse_locked = true;
    }
    if (use_mouse && !down && !up && !hit) {
        mb = SDL_GetMouseState(&(player->mouse_x), &(player->mouse_y));
        if (mb&SDL_BUTTON(SDL_BUTTON_LEFT)) {
            if (player->mouse_y < player->y) {
                down = false;
                up = true;
                diff = (player->y-player->mouse_y<diff)?(player->y-player->mouse_y):(diff);
            } else if (player->mouse_y > player->y) {
                up = false;
                down = true;
                diff = (player->mouse_y-player->y<diff)?(player->mouse_y-player->y):(diff);
            }
            player->mouse_locked = false;
        } else if (!player->mouse_locked) {
            hit = true;
        }
    }
#endif

#ifdef JOTSTICK

    if (fabsf(s->joystick_y) > JOYSTICK_TRESHOLD) {
        diff = PLAYER_MOVE_Y*fabsf(s->joystick_y)/40.0;
        if (diff > PLAYER_MOVE_Y) {
            diff = PLAYER_MOVE_Y;
        }
    }
#endif

    if (up) {
        player->y -= fminf(diff, diff*player->accelerate);
        player->accelerate *= PLAYER_ACCEL_INCREASE;
    } else if (down) {
        player->y += fminf(diff, diff*player->accelerate);
        player->accelerate *= PLAYER_ACCEL_INCREASE;
    } else {
        player->accelerate = PLAYER_ACCEL_DEFAULT;
    }

    if( hit) {
        if( !player->state && !player->state_locked) {
            player->state = PLAYER_STATE_MAX;
            player->state_locked = true;
        }
    } else {
        player->state_locked = false;
    }
}

void input_ai( Player* player, Ball* ball, Player* opponent, GameState* s) {
    float fact = 1.7;
    float target;

    if( fabsf( player->y - ball->y) > RACKET_Y_MID*5) {
        fact = 3.5;
    }

    target = GAME_Y_MID + (opponent->ball_dest - GAME_Y_MID)/5;

    if( player->responsible) {
        if( player->desire == DESIRE_NORMAL && !IS_NEAR_Y_AI( player->y, ball->y)) {
            if( player->y < ball->y) {
                player->y += fmin( 2*fact, ball->y - player->y);
            } else if( player->y > ball->y) {
                player->y -= fmin( 2*fact, player->y - ball->y);
            }
        }

        if( (ball->move_x != 0 || IS_NEAR_Y_AI( player->y, ball->y)) && IS_NEAR_X_AI( player->x, ball->x) && !player->state && rand()%4==0) {
            player->state = PLAYER_STATE_MAX;
        }
    } else if( ball->move_x == 0) {
        if( player->desire == DESIRE_NORMAL && !IS_NEAR_Y_AI( player->y, target)) {
            if( player->y < target) {
                player->y += fmin( fact, (target-player->y)/40.0);
            } else if( player->y > target) {
                player->y -= fmin( fact, (player->y-target)/40.0);
            }
        }
    } else if( s->ngram_prediction > 0.0) {
        target = s->ngram_prediction*((float)HEIGHT)/((float)(NGRAM_STEPS));
        target = GAME_Y_MID + (target-GAME_Y_MID)*1.5;

        if( player->desire == DESIRE_NORMAL && !IS_NEAR_Y_AI( player->y, target)) {
            if( player->y < target) {
                player->y += fmin( fact, (target-player->y)/40.0);
            } else if( player->y > target) {
                player->y -= fmin( fact, (player->y-target)/40.0);
            }
        }
    } else {/*
        if( player->desire == DESIRE_NORMAL) {
            if( !IS_NEAR_Y_AI( player->y, target)) {
                player->y += (target - player->y)/40.0;
            }
        }*/
    }
}

void game_setup_serve( GameState* s) {
    s->ball.jump = 7.5;
    s->ball.y = GAME_Y_MID;
    s->ball.move_x = 0.0;
    s->ball.move_y = 0.0;

    if( s->player1_serves) {
        /* Base on serve, decide who is responsible for the ball */
        s->player1.responsible = true;
        s->player1.ball_dest = 0.0;
        s->ball.x = GAME_X_MIN-RACKET_X_MID*1.5;
    } else {
        s->player1.responsible = false;
        s->player2.ball_dest = 0.0;
        s->ball.x = GAME_X_MAX+RACKET_X_MID*1.5;
    }

    s->player2.responsible = !(s->player1.responsible);
}

float ngram_predictor( GameState* s) {
    unsigned int count = 0;
    unsigned long sum = 0;
    int x, y, z;
    float result;

    if( s->history_size < 3) {
        return 0.0;
    }

    x = s->history[1];
    y = s->history[2];

    for( z = 0; z<NGRAM_STEPS; z++) {
        count += s->ngram[x][y][z];
        sum += z * s->ngram[x][y][z];
    }

    result = ((float)(sum))/((float)(count));
#ifdef DEBUG
    printf( "predicting next = %.2f\n", result);
#endif

    return result;
}

void score_game( GameState* s, bool player1_scored) {
    Player* winner = (player1_scored)?(&(s->player1)):(&(s->player2));
    Player* loser = (player1_scored)?(&(s->player2)):(&(s->player1));

    /* Increment the points per player */
    winner->point_count++;

    if (s->player1.point_count > 3)
    {
        // Turn on display for good players
        display_on = 1;
    }else {
        display_on = 0;
    }

    if( s->current_set >= SETS_TO_WIN*2-1) {
        return;
    }

    winner->game++;
    if( loser->game < winner->game-1) {
        if( winner->game >= 4) {
            winner->game = loser->game = 0;
            winner->sets[s->current_set]++;

#ifdef HAVE_VOICE_FILES
            /* speak the current score */
            voice_say_list(4, VOICE_ZERO_IN + (s->player1.sets[s->current_set])*2, VOICE_TO, VOICE_ZERO_OUT + (s->player2.sets[s->current_set])*2, VOICE_IN_THE_FIRST_SET+s->current_set);
#endif

            /* scoring the set.. */
            if( (winner->sets[s->current_set] == 6 && loser->sets[s->current_set] < 5) ||
                winner->sets[s->current_set] == 7) {
                s->current_set++;
                s->winner = game_get_winner( s);
            }
        }
    }
}

char* format_sets( GameState* s) {
    static char sets[100];
    static char tmp[100];
    int i, max = s->current_set;

    sets[0] = '\0';

    if( s->winner != WINNER_NONE) {
        max--;
    }
    for( i=0; i<=max; i++) {
        sprintf( tmp, "%d:%d, ", s->player1.sets[i], s->player2.sets[i]);
        strcat( sets, tmp);
    }

    sets[strlen(sets)-2] = '\0';

    return sets;
}

char* format_game( GameState* s) {
    static char game[100];
    static const int game_scoring[] = { 0, 15, 30, 40 };

    if( s->player1.game < 4 && s->player2.game < 4) {
#ifdef HAVE_VOICE_FILES
        if (s->player1.game > 0 || s->player2.game > 0) {
            if (s->player1.game == s->player2.game) {
                voice_say_list(2, VOICE_LOVE_IN + 2*(s->player1.game), VOICE_ALL);
            } else {
                voice_say_list(2, VOICE_LOVE_IN + 2*(s->player1.game), VOICE_LOVE_OUT + 2*(s->player2.game));
            }
        }
#endif
        sprintf( game, "%d - %d", game_scoring[s->player1.game], game_scoring[s->player2.game]);
    } else if( s->player1.game > s->player2.game) {
#ifdef HAVE_VOICE_FILES
        voice_say_list(1, VOICE_ADVANTAGE_PLAYER_ONE);
#endif
        strcpy( game, "advantage player 1");
    } else if( s->player1.game < s->player2.game) {
#ifdef HAVE_VOICE_FILES
        voice_say_list(1, VOICE_ADVANTAGE_PLAYER_TWO);
#endif
        strcpy( game, "advantage player 2");
    } else {
#ifdef HAVE_VOICE_FILES
        voice_say_list(1, VOICE_DEUCE);
#endif
        strcpy( game, "deuce");
    }

    return game;
}

char* format_status( GameState* s) {
    static char status[100];
    static const char* set_names[] = { "first", "second", "third", "fourth", "fifth" , "sixth", "seventh", "eighth"};

    /* Noticed a seg fault bug here: playing ai vs ai for 6 sets, looks like it got fixed by adding a few more set_names */

    sprintf( status, "%d:%d in %s set", s->player1.sets[s->current_set], s->player2.sets[s->current_set], set_names[s->current_set]);

    return status;
}

int game_get_winner( GameState* s) {
    int i;
    int sets[2] = {0};

    for( i=0; i<s->current_set; i++) {
        if( s->player1.sets[i] > s->player2.sets[i]) {
            sets[0]++;
        } else {
            sets[1]++;
        }
    }

    if( sets[0] == SETS_TO_WIN) return WINNER_PLAYER1;
    if( sets[1] == SETS_TO_WIN) return WINNER_PLAYER2;

    return WINNER_NONE;
}

