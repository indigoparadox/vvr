
import pygame

from . import vvr

def render_vvr( **kwargs ):

    vvr_parms = None
    with open( kwargs['vvr_path'], 'rb' ) as vvr_file:
        vvr_parms = vvr.parse_file( vvr_file )

    running = True

    screen = pygame.display.set_mode( (800, 600) )
    
    offset = 300

    while running:
        
        for prsm in vvr_parms[1]['ROOT'][0]['PRSM']:
            r = 255
            g = 10
            b = 10
            for coord in prsm['POLY'][0]['coords']:
                screen.set_at(
                    (coord[0] + offset, coord[2] + offset),
                    (r, g, b) )
                g += 10
                b += 10

        for event in pygame.event.get():
            if pygame.QUIT == event.type:
                running = False

        pygame.display.flip()

