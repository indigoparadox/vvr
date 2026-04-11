
import pygame
import OpenGL.GL as GL

from . import vvr

def render_face_3d( coord_a, coord_b, height=4.0 ):

    # Lower Triangle
    GL.glVertex3f( # Left Low
        coord_a[0], 0, coord_a[1] )
    GL.glVertex3f( # Right Low
        coord_b[0], 0, coord_b[1] )
    GL.glVertex3f( # Right High
        coord_b[0], height, coord_b[1] )

    # Upper Triangle
    GL.glVertex3f( # Right High
        coord_b[0], height, coord_b[1] )
    GL.glVertex3f( # Left High
        coord_a[0], height, coord_a[1] )
    GL.glVertex3f( # Left Low
        coord_a[0], 0, coord_a[1] )

def render_vvr( **kwargs ):

    vvr_parms = None
    with open( kwargs['vvr_path'], 'rb' ) as vvr_file:
        vvr_parms = vvr.parse_file( vvr_file )

    running = True

    screen = None
    if kwargs['threed']:
        screen = pygame.display.set_mode(
            (800, 600), pygame.OPENGL | pygame.DOUBLEBUF )
        GL.glViewport( 0, 0, 800, 600 );
        aspect_ratio = 600 / 800;
        GL.glMatrixMode( GL.GL_PROJECTION );
        GL.glLoadIdentity();
        GL.glFrustum(
            -1.0 * aspect_ratio, aspect_ratio, -1.0, 1.0, 0.5, 300.0 );
        GL.glMatrixMode( GL.GL_MODELVIEW );
        GL.glClearColor( 0, 0, 0, 0 );
        GL.glEnable( GL.GL_CULL_FACE );
        GL.glEnable( GL.GL_NORMALIZE );
        GL.glEnable( GL.GL_DEPTH_TEST );

    else:
        screen = pygame.display.set_mode( (800, 600) )
    
    offset = 100
    cam_z = 0
    cam_rot = 180

    while running:

        if kwargs['threed']:
            GL.glClear( GL.GL_COLOR_BUFFER_BIT | GL.GL_DEPTH_BUFFER_BIT );
            GL.glPushMatrix();
            GL.glTranslatef( 0, -20.0, cam_z );
            GL.glRotatef( cam_rot, 0, 1, 0 );
        
        for prsm in vvr_parms[1]['ROOT'][0]['PRSM']:
            color = (prsm['COLR'][0][1],
                prsm['COLR'][0][2],
                prsm['COLR'][0][3])

            if kwargs['threed']:
                GL.glPushMatrix();
                GL.glTranslatef(
                    int( prsm['POSN'][0][0] ),
                    0,
                    int( prsm['POSN'][0][1] ) )
                GL.glColor3f(
                    prsm['COLR'][0][1],
                    prsm['COLR'][0][2],
                    prsm['COLR'][0][3] )
                GL.glBegin( GL.GL_TRIANGLES );
            else:
                posn_x = int( prsm['POSN'][0][0] )
                posn_y = int( prsm['POSN'][0][1] )

            last_coord = None
            for coord in prsm['POLY'][0]['coords']:
                #screen.set_at(
                #    (coord[0] + offset + posn_x,
                #     coord[2] + offset + posn_y),
                #    color )
                if last_coord:
                    if kwargs['threed']:
                        render_face_3d( last_coord, coord )
                    else:
                        pygame.draw.line( screen, color,
                            (last_coord[0] + offset + int( prsm['POSN'][0][0] ),
                            last_coord[2] + offset + int( prsm['POSN'][0][1] )),
                            (coord[0] + offset + int( prsm['POSN'][0][0] ),
                            coord[2] + offset + int( prsm['POSN'][0][1] ) ))
                last_coord = coord

            # Link the end back up with the beginning.
            if kwargs['threed']:
                # TODO Last Face
                GL.glEnd()
                GL.glPopMatrix();
            else:
                pygame.draw.line( screen, color,
                    (last_coord[0] + offset + int( prsm['POSN'][0][0] ),
                    last_coord[2] + offset + int( prsm['POSN'][0][1] )),
                    (prsm['POLY'][0]['coords'][0][0] + offset + \
                        int( prsm['POSN'][0][0] ),
                    prsm['POLY'][0]['coords'][0][2] + offset + \
                        int( prsm['POSN'][0][1] )) )

        if kwargs['threed']:
            GL.glPopMatrix();

        for event in pygame.event.get():
            if pygame.QUIT == event.type:
                running = False
            elif pygame.KEYDOWN == event.type:
                if pygame.K_w == event.key:
                    cam_z += 10
                elif pygame.K_s == event.key:
                    cam_z -= 10
                elif pygame.K_d == event.key:
                    cam_rot += 10
                    if 360 <= cam_rot:
                        cam_rot = 0
                elif pygame.K_a == event.key:
                    cam_rot -= 10
                    if 0 > cam_rot:
                        cam_rot = 360

        pygame.display.flip()
        pygame.time.wait( 10 )

