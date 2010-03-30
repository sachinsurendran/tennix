
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

#ifndef __TENNIX_H
#define __TENNIX_H

#include <sys/param.h>

#include "SDL.h"

#ifdef DELUXE_EDITION
#  define HAVE_VOICE_FILES
#  define EXTENDED_REFEREE
#endif

#define ARCHIVE_FILE "tennix.tnx"
#define ARCHIVE_FILE_INSTALLED "/" PREFIX "share/tennix/" ARCHIVE_FILE

#define COPYRIGHT "Copyright 2003, 2007, 2008, 2009 Thomas Perl"
#define URL "http://icculus.org/tennix/"

#define WIDTH 640
#define HEIGHT 480

#define BENCHMARK_TIMELIMIT 60 /* in seconds */
#define MENU_BUTTON_HEIGHT 62
#define MENU_BUTTON_WIDTH 185

#define MENU_START_YPOS 150
#define MENU_RESUME_YPOS 230
#define MENU_QUIT_YPOS 350

#define M_POS_XPOS(x)    (x>WIDTH-MENU_BUTTON_WIDTH)
#define M_POS_START(y)   (y>MENU_START_YPOS && y<MENU_START_YPOS+MENU_BUTTON_HEIGHT)
#define M_POS_RESUME(y) (y>MENU_RESUME_YPOS && y<MENU_RESUME_YPOS+MENU_BUTTON_HEIGHT)
#define M_POS_QUIT(y)    (y>MENU_QUIT_YPOS && y<MENU_QUIT_YPOS+MENU_BUTTON_HEIGHT)

#define MENU_OPTIONS_BORDER 20
#define MENU_OPTIONS_BUTTON_HEIGHT 50
#define MENU_OPTIONS_BUTTON_WIDTH 150

/* Menu items */
enum {
    MENU_NONE = 0,
    MENU_START = 1,
    MENU_RESUME = 2,
    MENU_COURT_CHANGE = 4,
    MENU_PLAYER1 = 8,
    MENU_PLAYER2 = 16,
    MENU_QUIT = 32
};

#define M_POS_DECODE(x,y) (M_POS_XPOS(x)?((M_POS_START(y))?(MENU_START):((M_POS_QUIT(y))?(MENU_QUIT):((M_POS_RESUME(y))?(MENU_RESUME):(MENU_NONE)))):(MENU_NONE))

/* Menu states */
enum {
    MENU_STATE_STARTED = 0,
    MENU_STATE_SLIDE_TO_MAINMENU,
    MENU_STATE_SLIDE_TO_MAINMENU_IN_PROGRESS,
    MENU_STATE_MAINMENU,
    MENU_STATE_SLIDE_TO_OPTIONS,
    MENU_STATE_SLIDE_TO_OPTIONS_IN_PROGRESS,
    MENU_STATE_OPTIONS,
    MENU_STATE_SLIDE_TO_GAME,
    MENU_STATE_SLIDE_TO_GAME_IN_PROGRESS,
    MENU_STATE_GAME,
    MENU_STATE_SLIDE_TO_RESUME,
    MENU_STATE_SLIDE_TO_RESUME_IN_PROGRESS,
    MENU_STATE_RESUME,
    MENU_STATE_SLIDE_TO_QUIT,
    MENU_STATE_SLIDE_TO_QUIT_IN_PROGRESS,
    MENU_STATE_QUIT
};

/* Comment the following #define to disable FPS limiting */
/*#define ENABLE_FPS_LIMIT*/
#define DEFAULT_FPS 33


extern SDL_Surface *screen;
extern int display_on;

typedef struct {
    char *text;
    int x;
    int y;
    int w;
    int h;
    Uint8 r;
    Uint8 g;
    Uint8 b;
} MenuButton;

#define M_POS_BUTTON(button,mx,my) (mx>=button.x && mx<=button.x+button.w && my>=button.y && my<=button.y+button.h)
/* Added by sachins */
#define PLAYER1 1
#define PLAYER2 2

/* For debugging */
// Comment GRAPHICS if we dont need any display
#define GRAPHICS

#endif
