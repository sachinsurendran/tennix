
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

#ifndef __GRAPHICS_H
#define __GRAPHICS_H

#include "tennix.h"

#include "SDL_image.h"

#define RECT_UPDATE_CACHE 150

#define FADE_DURATION 500
#define BUTTON_HIGHLIGHT 0.4
#define BUTTON_BORDER 2

#define GET_PIXEL_DATA(surface,x,y) (*((Uint32*)(surface->pixels + x * surface->format->BytesPerPixel + y * surface->pitch)))

#define GET_PIXEL_RGB(surface,x,y,r,g,b) (SDL_GetRGB( GET_PIXEL_DATA(surface,x,y), surface->format, r, g, b))
#define SET_PIXEL_RGB(surface,x,y,r,g,b) (GET_PIXEL_DATA(surface,x,y)=SDL_MapRGB(surface->format, r, g, b))

typedef struct {
    SDL_Surface* data;
} Image;

typedef unsigned int image_id;
enum {
    GR_COURT = 0,
    GR_SHADOW,
    GR_RACKET,
    GR_BALL,
    GR_SMALLISH_FONT,
    GR_DKC2_FONT,
    GR_REFEREE,
    GR_CTT_HARD,
    GR_CTT_CLAY,
    GR_CTT_GRASS,
    GR_SIDEBAR,
    GR_TENNIXLOGO,
    GR_BTN_PLAY,
    GR_BTN_RESUME,
    GR_BTN_QUIT,
    GR_STADIUM,
    GR_FOG,
    GR_FOG2,
    GR_NIGHT,
    GR_TALK,
    GR_COUNT
};

#define GRAPHICS_FONT_FIRST GR_SMALLISH_FONT
#define GRAPHICS_FONT_LAST GR_DKC2_FONT

#define GRAPHICS_IS_FONT(id) ((id>=GRAPHICS_FONT_FIRST) && (id<=GRAPHICS_FONT_LAST))

#define GRAPHICS_FONT_COUNT (GRAPHICS_FONT_LAST-GRAPHICS_FONT_FIRST+1)

#define GR_CTT_FIRST GR_CTT_HARD
#define GR_CTT_LAST GR_CTT_GRASS

enum {
    ANIMATION_NONE = 0,
    ANIMATION_WAVE = 1,
    ANIMATION_BUNGEE = 2,
    ANIMATION_PENDULUM = 4,
    ANIMATION_COUNT = 8
};

void init_graphics();
void uninit_graphics();
int get_image_width(image_id);
int get_image_height(image_id);
int get_sprite_width(image_id, int);
void show_sprite(image_id, int, int, int, int, int);
#define show_image(id, x_offset, y_offset, opacity) show_sprite(id, 0, 1, x_offset, y_offset, opacity)
void line_horiz( int y, Uint8 r, Uint8 g, Uint8 b);
void line_vert( int x, Uint8 r, Uint8 g, Uint8 b);
void rectangle( int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b);
void fill_image_offset(image_id id, int x, int y, int w, int h, int offset_x, int offset_y);
#define fill_image(id, x, y, w, h) fill_image_offset(id, x, y, w, h, 0, 0)
void draw_button( int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b, char pressed);
void draw_button_text( char* s, int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b, char pressed);
#define draw_button_object(button,mx,my) (draw_button_text(button.text, button.x, button.y, button.w, button.h, button.r, button.g, button.b, M_POS_BUTTON(button, mx, my)))

void clear_screen();
void store_screen();
void reset_screen();

void clearscr();
void update_rect(Sint32 x, Sint32 y, Sint32 w, Sint32 h);
#define update_rect2(r) (update_rect(r.x, r.y, r.w, r.h))
void updatescr();
void start_fade();

int font_get_metrics(image_id id, unsigned char ch, int* xp, int* wp);
int font_draw_char( image_id id, char ch, int x_offset, int y_offset);
void font_draw_string_alpha( image_id id, const char* s, int x_offset, int y_offset, int start, int animation, int opacity);
#define font_draw_string(id,s,x_offset,y_offset,start,animation) font_draw_string_alpha(id,s,x_offset,y_offset,start,animation,255)
int font_get_string_width( image_id id, const char* s);

void draw_line_faded( int x1, int y1, int x2, int y2, int r, int g, int b, int r2, int g2, int b2);
#define draw_line(x1,y1,x2,y2,r,g,b) draw_line_faded(x1,y1,x2,y2,r,g,b,r,g,b)

extern Uint32 fading_start;

#endif

