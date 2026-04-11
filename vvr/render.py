
import pygame

from . import vvr

def render_vvr( **kwargs ):

    vvr_parms = None
    with open( kwargs['vvr_path'], 'rb' ) as vvr_file:
        vvr_parms = vvr.parse_file( vvr_file )

    running = True

    screen = pygame.display.set_mode( (800, 600) )
    
    offset = 100

    while running:
        
        for prsm in vvr_parms[1]['ROOT'][0]['PRSM']:
            color = (prsm['COLR'][0][1],
                prsm['COLR'][0][2],
                prsm['COLR'][0][3])

            posn_x = int( prsm['POSN'][0][0] )
            posn_y = int( prsm['POSN'][0][1] )

            last_coord = None
            for coord in prsm['POLY'][0]['coords']:
                #screen.set_at(
                #    (coord[0] + offset + posn_x,
                #     coord[2] + offset + posn_y),
                #    color )
                if last_coord:
                    pygame.draw.line( screen, color,
                        (last_coord[0] + offset + posn_x,
                        last_coord[2] + offset + posn_y),
                        (coord[0] + offset + posn_x,
                        coord[2] + offset + posn_y) )
                last_coord = coord

            # Link the end back up with the beginning.
            pygame.draw.line( screen, color,
                (last_coord[0] + offset + posn_x,
                last_coord[2] + offset + posn_y),
                (prsm['POLY'][0]['coords'][0][0] + offset + posn_x,
                 prsm['POLY'][0]['coords'][0][2] + offset + posn_y) )

        for event in pygame.event.get():
            if pygame.QUIT == event.type:
                running = False

        pygame.display.flip()

