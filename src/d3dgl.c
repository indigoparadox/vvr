
#include "vvr.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <GL/gl.h>
#include <GL/glut.h>

#define OGL_SCREEN_W 1024
#define OGL_SCREEN_H 768

#define trans_coord( poly, idx, posn, xy ) \
   (vvr_fix_endian_16( (poly)->coords[idx].xy))

uint8_t* g_vvr_buf = NULL;
size_t g_vvr_sz = 0;
int g_rot = 180;
int g_z = 0;

int ogl_opengl_setup() {
   int retval = 0;
   float aspect_ratio = 0;
   float rzoom = 0;

   glViewport( 0, 0, OGL_SCREEN_W, OGL_SCREEN_H );
   aspect_ratio = OGL_SCREEN_W / OGL_SCREEN_H;
   glMatrixMode( GL_PROJECTION );
   glLoadIdentity();
   rzoom = 1.0f;
   glFrustum(
      -1.0f * rzoom * aspect_ratio,
      rzoom * aspect_ratio,
      -1.0f * rzoom,
      rzoom,
      0.5f, 200.0f );
   glMatrixMode( GL_MODELVIEW );
   glClearColor( 0, 0, 0, 0 );
   glEnable( GL_CULL_FACE );
   glEnable( GL_NORMALIZE );
   glEnable( GL_DEPTH_TEST );

   return 0;
}

/* === */

void ogl_face(
   struct VVR_SECT_POLY* poly, struct VVR_SECT_POSN* posn, int i, int j,
   float height
) {
   /*
   printf( "%d: %d, %d\n", i,
      trans_coord( poly, i, posn, x ),
      trans_coord( poly, i, posn, y ) );
   */

   /* Lower Triangle */
   glVertex3f( /* Left Low */
      vvr_fix_endian_16( poly->coords[i].x ),
      0,
      vvr_fix_endian_16( poly->coords[i].y ) );
   glVertex3f( /* Right Low */
      vvr_fix_endian_16( poly->coords[j].x ),
      0,
      vvr_fix_endian_16( poly->coords[j].y ) );
   glVertex3f( /* Right High */
      vvr_fix_endian_16( poly->coords[j].x ),
      height,
      vvr_fix_endian_16( poly->coords[j].y ) );

   /* Upper Triangle */
   glVertex3f( /* Right High */
      vvr_fix_endian_16( poly->coords[j].x ),
      height,
      vvr_fix_endian_16( poly->coords[j].y ) );
   glVertex3f( /* Left High */
      vvr_fix_endian_16( poly->coords[i].x ),
      height,
      vvr_fix_endian_16( poly->coords[i].y ) );
   glVertex3f( /* Left Low */
      vvr_fix_endian_16( poly->coords[i].x ),
      0,
      vvr_fix_endian_16( poly->coords[i].y ) );

}

/* === */

void ogl_opengl_frame() {
   uint8_t* next = NULL;
   int i = 0, j = 0, k = 0;
   struct VVR_SECT_POSN* posn = NULL;
   struct VVR_SECT_GENERIC* prsm = NULL;
   struct VVR_SECT_POLY* poly = NULL;
   struct VVR_SECT_COLR* colr = NULL;

   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
   glPushMatrix();

   glRotatef( g_rot, 0, 1, 0 );

   glTranslatef( 0, -20.0f, g_z );

   printf( "drawing...\n" );

	/* Redraw loaded objects. */
   while( NULL != (next = next_sect( "PRSM", g_vvr_buf, g_vvr_sz, 1, &i )) ) {
      prsm = (struct VVR_SECT_GENERIC*)&(g_vvr_buf[i]);
      printf( "found PRSM @ 0x%x, diving...\n", i );

      glPushMatrix();

      /* Dive into the PRSM section for COLR sections. */
      j = i + sizeof( struct VVR_SECT_HEAD );
      colr = (struct VVR_SECT_COLR*)next_sect(
         "COLR", g_vvr_buf, g_vvr_sz, 1, &j );
      assert( NULL != colr );

      glColor3f( colr->color1.r, colr->color1.g, colr->color2.b );

      /* Dive into the PRSM section for POSN sections. */
      j = i + sizeof( struct VVR_SECT_HEAD );
      posn = (struct VVR_SECT_POSN*)next_sect(
         "POSN", g_vvr_buf, g_vvr_sz, 1, &j );
      assert( NULL != posn );

      glScalef( 0.5f, 0.5f, 0.5f );

      glTranslatef(
         vvr_fix_endian_16( posn->x.integer ),
         0,
         vvr_fix_endian_16( posn->y.integer ) );

      /* Dive into the PRSM section for POLY sections. */
      j = i + sizeof( struct VVR_SECT_HEAD );
      while(
         NULL != (next = next_sect( "POLY", g_vvr_buf, g_vvr_sz, 1, &j ))
      ) {
         poly = (struct VVR_SECT_POLY*)next;
         /* TODO */

         glBegin( GL_TRIANGLES );
         for( k = 1 ; vvr_fix_endian_32( poly->coords_ct ) > k ; k++ ) {
            ogl_face( poly, posn, k - 1, k, 20.0f );
         }
         ogl_face( poly, posn, k - 1, 0, 20.0f );
         glEnd();

         /* Skip to section after POSN (size plus sz/sect fields). */
         j += vvr_fix_endian_32( poly->head.sz ) +
            sizeof( struct VVR_SECT_HEAD );
      }

      glPopMatrix();
      
      /* Skip to section after PRSM (size plus sz/sect fields). */
      i += vvr_fix_endian_32( prsm->head.sz ) + sizeof( struct VVR_SECT_HEAD );
   }

   glPopMatrix();

   glutSwapBuffers();
}

/* === */

void ogl_key_in( unsigned char key, int x, int y ) {
   switch( key ) {
      case 'w':
         g_z += 2;
         glutPostRedisplay();
         break;

      case 's':
         g_z -= 2;
         glutPostRedisplay();
         break;

      case 'd':
         g_rot += 10;
         if( 360 <= g_rot ) {
            g_rot = 0;
         }
         glutPostRedisplay();
         break;

      case 'a':
         g_rot -= 10;
         if( 0 > g_rot ) {
            g_rot = 350;
         }
         glutPostRedisplay();
         break;
   }
}

/* === */

int main( int argc, char* argv[] ) {
	int retval = 0;
   uint8_t verbose = 0;
   char c = 0;
   FILE* vvr_file = NULL;
   char* vvr_path = NULL;
   size_t vvr_read = 0;

   /* Setup OpenGL. */

	glutInit( &argc, argv );
	glutInitWindowSize( OGL_SCREEN_W, OGL_SCREEN_H );
	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
	glutCreateWindow( "VVR Test" );

   retval = ogl_opengl_setup();  

   glutDisplayFunc( ogl_opengl_frame );
   glutKeyboardFunc( ogl_key_in );

   /* Load the VVR. */

   while( -1 != (c = getopt( argc, argv, ":v" )) ) {
      switch( c ) {
         case 'v':
            verbose = 1;
            break;

         case '?':
            exit( 1 );
            break;
      }
   }
   
   vvr_path = argv[optind];
   if( NULL == vvr_path ) {
      fprintf( stderr, "no vvr file specified!\n" );
      exit( 1 );
   }

   vvr_file = fopen( vvr_path, "rb" );
   fseek( vvr_file, 0, SEEK_END );
   g_vvr_sz = ftell( vvr_file );
   fseek( vvr_file, 0, SEEK_SET );

   g_vvr_buf = calloc( g_vvr_sz, 1 );
   assert( NULL != g_vvr_buf );

   vvr_read = fread( g_vvr_buf, 1, g_vvr_sz, vvr_file );
   assert( vvr_read == g_vvr_sz );

   fclose( vvr_file );

   /* Start! */

   glutMainLoop();

	return retval;
}

