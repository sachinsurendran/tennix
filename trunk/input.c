
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

#include "tennix.h"
#include "input.h"

static char joystick_help[100];

void wait_keypress() {
    SDL_Event e;
    unsigned char done = 0;

    SDL_Delay( 100);

    /* clean up all events */
    while( SDL_PollEvent( &e));

    while( !done) {
        if( SDL_PollEvent( &e)) {
            done = (e.type == SDL_KEYUP || e.type == SDL_MOUSEBUTTONUP);
        } else {
            SDL_Delay( 50);
        }
    }
}

void init_joystick()
{
    SDL_JoystickEventState(SDL_ENABLE);
}

void uninit_joystick()
{
    SDL_JoystickEventState(SDL_IGNORE);
}

void joystick_list()
{
    int i, n;

    n = SDL_NumJoysticks();

    for (i=0; i<n; i++) {
        printf("Joystick %d: %s (use with: --joystick \"%s\")\n", i+1, SDL_JoystickName(i), SDL_JoystickName(i));
    }
}

int joystick_open(const char* name)
{
    int i, n;

    n = SDL_NumJoysticks();

    for (i=0; i<n; i++) {
        if (strcmp(SDL_JoystickName(i), name) == 0) {
            SDL_JoystickOpen(i);
            sprintf(joystick_help, "joystick enabled: %s", name);
            return 1;
        }
    }
    return 0;
}

char* get_joystick_help()
{
    if (strcmp(joystick_help, "") == 0) {
        return "no joystick (enable with --joystick)";
    }
    else {
        return joystick_help;
    }
}

