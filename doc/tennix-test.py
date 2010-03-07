#!/usr/bin/python

import pygame
from pygame.constants import *
from math import cos
from math import pi

WIDTH = 640
HEIGHT = 480
H_FACT = 0.10

pygame.init()
pygame.display.set_mode( (WIDTH,HEIGHT))
surface = pygame.display.get_surface()
clock = pygame.time.Clock()

def convert_3d_2d( threed, h_factor):
    #x = threed.x * (0.1 * threed.y + 0.9)
    #y = (threed.y - threed.z*h_factor) * (0.15 * threed.y + 0.85)
    a = 0.2
    b = 0.6
    x = threed.x * (a * threed.y + b)
    y = (threed.y - threed.z*h_factor) * (a * threed.y + b)

    return Coord2D( x, y)

def resize_depth( size, depth):
    return size*(0.75+depth/4)

class Coord3D(object):
    def __init__( self, x, y, z = 0.0):
        self.x = x
        self.y = y
        self.z = z

    def bottom( self):
        return Coord3D( self.x, self.y, 0.0)

    def twod( self):
        return convert_3d_2d( self, H_FACT)

class Coord2D(object):
    def __init__( self, x, y):
        self.x = x
        self.y = y


tennis_area = [
        ( Coord3D( -1, -1), Coord3D( -1, 1 ) ),
        ( Coord3D( -1, -1), Coord3D( 1, -1) ),
        ( Coord3D( -1, 1), Coord3D( 1, 1) ),
        ( Coord3D( 1, -1), Coord3D( 1, 1) ),
        ( Coord3D( -1, 0, 1), Coord3D( -1, 0, 0) ),
        ( Coord3D( -1, 0, 1), Coord3D( 1, 0, 1) ),
        ( Coord3D( 1, 0, 1), Coord3D( 1, 0, 0) ),
        ( Coord3D( -1, 0, 0), Coord3D( 1, 0, 0) ),
]

def twod_to_real( twod):
    x = twod.x * WIDTH/2.1 + WIDTH/2
    y = twod.y * HEIGHT/2.1 + HEIGHT/2
    return Coord2D( x, y)


hist = []

MOVE_Y_DFL = 0.015
MOVE_X_DFL = 0.0025

move_y = MOVE_Y_DFL
move_x = MOVE_X_DFL

zrestitution = 0.7
gravity = 0.009

yrestitution = 0.7

vel_z = 0

draw_lines = True

x = 0
y = 0
z = 10
i = 1
while i > 0:
    clock.tick( 70)
    evt = pygame.event.poll()
    i += 1

    if evt.type == KEYUP and evt.key == K_ESCAPE:
        break

    if evt.type == MOUSEMOTION:
        x = (evt.pos[0]*2.0/WIDTH)-1.0
        y = (evt.pos[1]*2.0/HEIGHT)-1.0
    elif evt.type == MOUSEBUTTONDOWN:
        if evt.button == 1:
            vel_z = -.2
            mmy = MOVE_Y_DFL*1.8
        else:
            vel_z = -.4
            mmy = MOVE_Y_DFL*1.2
        if y > 0:
            move_y = -mmy
        else:
            move_y = +mmy
    else:
        x += move_x
        y += move_y
        if x > 1.0 or x < -1.0:
            move_x *= -1
        if y > 1.0 or y < -1.0:
            move_y *= -yrestitution
        if z < 0:
            vel_z *= -zrestitution
            z = 0
        vel_z += gravity
        z -= vel_z

    surface.fill( (0,0,0))
    for a in tennis_area:
        twod_a = twod_to_real( a[0].twod())
        twod_b = twod_to_real( a[1].twod())
        pygame.draw.aaline( surface, (255,255,255), (twod_a.x,twod_a.y), (twod_b.x,twod_b.y))
 
    currd = Coord3D(x, y, z)

    if i % 2 == 0:
        hist.append( currd)

    if len(hist) > 250:
        hist = hist[-250:]

    if draw_lines:
        old = None
        u = 0
        for e in hist:
            if old:
                twod_a = twod_to_real( convert_3d_2d( old, H_FACT))
                twod_b = twod_to_real( convert_3d_2d( e, H_FACT))
                pygame.draw.aaline( surface, (0,u,0), (twod_a.x,twod_a.y), (twod_b.x,twod_b.y))
                twod_a = twod_to_real( convert_3d_2d( old.bottom(), H_FACT))
                twod_b = twod_to_real( convert_3d_2d( e.bottom(), H_FACT))
                pygame.draw.aaline( surface, (max(0, min(255, 255-old.z/3*255)),0,u), (twod_a.x,twod_a.y), (twod_b.x,twod_b.y))
            old = e
            u += int(255/len(hist))
 
    ballpos = twod_to_real( convert_3d_2d( currd, H_FACT))
    twod_a = twod_to_real( convert_3d_2d( Coord3D( 0.0, 0.0), H_FACT))
    twod_b = twod_to_real( convert_3d_2d( currd, H_FACT))
    pygame.draw.aaline( surface, (255,0,0), (twod_a.x,twod_a.y), (twod_b.x,twod_b.y))
    twod_a = twod_to_real( convert_3d_2d( Coord3D( 0.0, 0.0), H_FACT))
    twod_b = twod_to_real( convert_3d_2d( Coord3D( x, y, 0), H_FACT))
    pygame.draw.aaline( surface, (255,0,0), (twod_a.x,twod_a.y), (twod_b.x,twod_b.y))
    twod_a = twod_to_real( convert_3d_2d( currd, H_FACT))
    twod_b = twod_to_real( convert_3d_2d( Coord3D( x, y, 0), H_FACT))
    pygame.draw.aaline( surface, (255,0,0), (twod_a.x,twod_a.y), (twod_b.x,twod_b.y))

    pygame.draw.circle( surface, (50,50,50), (twod_b.x,twod_b.y), int(resize_depth( 8, currd.y)/max(1,currd.z)))
    pygame.draw.circle( surface, (255,230,0), (ballpos.x,ballpos.y), int(resize_depth( 9, currd.y)))

    pygame.display.flip()

