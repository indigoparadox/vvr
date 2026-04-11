
#include "vvr.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <unistd.h>
#include <GL/gl.h>
#include <GL/glut.h>

#define OGL_SCREEN_W 1024
#define OGL_SCREEN_H 768

uint8_t* g_vvr_buf = NULL;
size_t g_vvr_sz = 0;
float g_rot = 180.0f;
float g_z = 0;
float g_x = 0;

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
   glEnable( GL_LIGHTING );
   glEnable( GL_NORMALIZE );
   glEnable( GL_COLOR_MATERIAL );

   /*
   glColorMaterial( GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE );
   glShadeModel( GL_SMOOTH );
   */

   return 0;
}

/* === */

void ogl_top(
   struct VVR_SECT_POLY* poly, float height, int r, int g, int b
) {
   int i = 0, cx = 0, cy = 0, lx = 0, ly = 0, hx = 0, hy = 0;

   /* Figure out extreme vertices. */

   hx = vvr_fix_endian_16( poly->coords[0].x );
   hy = vvr_fix_endian_16( poly->coords[0].y );
   lx = vvr_fix_endian_16( poly->coords[0].x );
   ly = vvr_fix_endian_16( poly->coords[0].y );

#ifdef DEBUG
   printf( "drawing top...\n" );
#endif /* DEBUG */

#define hxlx_compare( xy ) \
   if( vvr_fix_endian_16( poly->coords[i].xy ) > h ## xy ) { \
      h ## xy = vvr_fix_endian_16( poly->coords[i].xy ); \
   } else if( vvr_fix_endian_16( poly->coords[i].xy ) < l ## xy ) { \
      l ## xy = vvr_fix_endian_16( poly->coords[i].xy ); \
   }

   for( i = 1 ; vvr_fix_endian_32( poly->coords_ct ) > i ; i++ ) {
#ifdef DEBUG
      printf( "%d: x: %d, y: %d\n", i,
         vvr_fix_endian_16( poly->coords[i].x ),
         vvr_fix_endian_16( poly->coords[i].y ) );
#endif /* DEBUG */

      hxlx_compare( x );
      hxlx_compare( y );
   }

#ifdef DEBUG
   printf( "lx: %d, ly: %d, hx: %d, hy: %d\n", lx, ly, hx, hy );
#endif /* DEBUG */

   assert( hx > lx );
   assert( hy > ly );

   /* Figure out center point. */

   cx = (0 > lx ? (hx + lx) : (hx - lx)) / 2;
   cx = (0 > ly ? (hy + ly) : (hy - ly)) / 2;
#ifdef DEBUG
   printf( "cx: %d, cy: %d\n", cx, cy );
#endif /* DEBUG */

   /* Draw top faces. */

   for( i = 1 ; vvr_fix_endian_32( poly->coords_ct ) > i ; i++ ) {
      glBegin( GL_TRIANGLES );
      glNormal3f( 0, height, 0 );
      glColor3f( r, g, b );

      glVertex3f( /* Outer Left */
         vvr_fix_endian_16( poly->coords[i - 1].x ),
         height,
         vvr_fix_endian_16( poly->coords[i - 1].y ) );
      glVertex3f( /* Outer Right */
         vvr_fix_endian_16( poly->coords[i].x ),
         height,
         vvr_fix_endian_16( poly->coords[i].y ) );
      glVertex3f( cx, height, cy ); /* Center */
      glEnd();
   }

   /* Draw final top face. */

   glBegin( GL_TRIANGLES );
   glNormal3f( 0, height, 0 );
   glColor3f( r, g, b );
   glVertex3f( /* Outer Left */
      vvr_fix_endian_16( poly->coords[i - 1].x ),
      height,
      vvr_fix_endian_16( poly->coords[i - 1].y ) );
   glVertex3f( /* Outer Right */
      vvr_fix_endian_16( poly->coords[0].x ),
      height,
      vvr_fix_endian_16( poly->coords[0].y ) );
   glVertex3f( cx, height, cy ); /* Center */
   glEnd();
}

/* === */

void ogl_face(
   struct VVR_SECT_POLY* poly, int i, int j, float height, int r, int g, int b
) {
   glBegin( GL_TRIANGLES );
   glNormal3f(
      vvr_fix_endian_16( poly->coords[i].x ),
      0,
      0 );
   glColor3f( r, g, b );

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
   glEnd();

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

   glEnable( GL_LIGHT0 );

   glPushMatrix();

   glRotatef( g_rot, 0, 1, 0 );

   glTranslatef( g_x, -20.0f, g_z );

#ifdef DEBUG
   printf( "drawing...\n" );
#endif /* DEBUG */

	/* Redraw loaded objects. */
   while( NULL != (next = next_sect( "PRSM", g_vvr_buf, g_vvr_sz, 1, &i )) ) {
      prsm = (struct VVR_SECT_GENERIC*)&(g_vvr_buf[i]);
#ifdef DEBUG
      printf( "found PRSM @ 0x%x, diving...\n", i );
#endif /* DEBUG */

      glPushMatrix();

      /* Dive into the PRSM section for COLR sections. */
      j = i + sizeof( struct VVR_SECT_HEAD );
      colr = (struct VVR_SECT_COLR*)next_sect(
         "COLR", g_vvr_buf, g_vvr_sz, 1, &j );
      assert( NULL != colr );

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

         for( k = 1 ; vvr_fix_endian_32( poly->coords_ct ) > k ; k++ ) {
            ogl_face( poly, k - 1, k, 20.0f,
               colr->color1.r, colr->color1.g, colr->color1.b );
         }
         ogl_face( poly, k - 1, 0, 20.0f,
            colr->color1.r, colr->color1.g, colr->color1.b );
         ogl_top( poly, 20.0f,
            colr->color1.r, colr->color1.g, colr->color1.b );

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
         g_z += cosf( (g_rot * (2.0f * 3.14f)) / 360.0f ) * 0.5f;
         g_x -= sinf( (g_rot * (2.0f * 3.14f)) / 360.0f ) * 0.5f;
         glutPostRedisplay();
         break;

      case 's':
         g_z -= cosf( (g_rot * (2.0f * 3.14f)) / 360.0f ) * 0.5f;
         g_x += sinf( (g_rot * (2.0f * 3.14f)) / 360.0f ) * 0.5f;
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

