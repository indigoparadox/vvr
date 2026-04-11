
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

#define STATIC_HEIGHT 30.0f

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
   glColorMaterial( GL_FRONT, GL_AMBIENT_AND_DIFFUSE );

   /*
   glShadeModel( GL_SMOOTH );
   */

   return 0;
}

/* === */

void ogl_draw_face_seg(
   float x_outside1, float y_outside1,
   float x_outside2, float y_outside2,
   float x_inside1, float y_inside1,
   float x_inside2, float y_inside2,
   float y_bottom, float y_top,
   float* color
) {
   glBegin( GL_TRIANGLES );
   glNormal3f( 0, 1.0f, 0 );
   glColor4fv( color );

   glVertex3f( /* Outer Left */
      x_outside1, y_bottom, y_outside1 );
   glVertex3f( /* Outer Right */
      x_outside2, y_bottom, y_outside2 );
   glVertex3f( /* Center Left */
      x_inside1, y_top, y_inside1 );

   if( x_inside1 != x_inside2 || y_inside1 != y_inside2 ) {
      glVertex3f( /* Center Right */
         x_inside2, y_top, y_inside2 );
      glVertex3f( /* Center Left */
         x_inside1, y_top, y_inside1 );
      glVertex3f( /* Outer Right */
         x_outside2, y_bottom, y_outside2 );
   }

   glEnd();
}

/* === */

void ogl_draw_top(
   struct VVR_SECT_POLY* poly, float height, int layer, float* color
) {
   int i = 0, cx = 0, cy = 0, lx = 0, ly = 0, hx = 0, hy = 0;
   float y_edges = 0, y_center = height;

   switch( poly->vprofile ) {
   case VVR_POLYPROF_SOLID:
      y_edges = height;
      break;
   }

   /* Figure out extreme vertices. */

   hx = vvr_fix_endian_16( poly->coords[0].x );
   hy = vvr_fix_endian_16( poly->coords[0].y );
   lx = vvr_fix_endian_16( poly->coords[0].x );
   ly = vvr_fix_endian_16( poly->coords[0].y );

#ifdef DEBUG
   printf( "drawing top...\n" );
#endif /* DEBUG */

   /* Find outermost coords in of the poly. */
   for( i = 1 ; vvr_fix_endian_32( poly->coords_ct ) > i ; i++ ) {
#ifdef DEBUG
      printf( "%d: x: %d, y: %d\n", i,
         vvr_fix_endian_16( poly->coords[i].x ),
         vvr_fix_endian_16( poly->coords[i].y ) );
#endif /* DEBUG */

      /* Find highest/lowest X. */
      if( vvr_fix_endian_16( poly->coords[i].x ) > hx ) {
         hx = vvr_fix_endian_16( poly->coords[i].x );
      } else if( vvr_fix_endian_16( poly->coords[i].x ) < lx ) {
         lx = vvr_fix_endian_16( poly->coords[i].x );
      }

      /* Find highest/lowest Y. */
      if( vvr_fix_endian_16( poly->coords[i].y ) > hy ) {
         hy = vvr_fix_endian_16( poly->coords[i].y );
      } else if( vvr_fix_endian_16( poly->coords[i].y ) < ly ) {
         ly = vvr_fix_endian_16( poly->coords[i].y );
      }
   }

#ifdef DEBUG
   printf( "lx: %d, ly: %d, hx: %d, hy: %d\n", lx, ly, hx, hy );
#endif /* DEBUG */

   assert( hx > lx );
   assert( hy > ly );

   /* Figure out center point. */
   cx = (0 > lx ? (hx + lx) : (hx - lx)) / 2;
   cy = (0 > ly ? (hy + ly) : (hy - ly)) / 2;
#ifdef DEBUG
   printf( "cx: %d, cy: %d\n", cx, cy );
#endif /* DEBUG */

   /* Draw top faces. */
   for( i = 1 ; vvr_fix_endian_32( poly->coords_ct ) > i ; i++ ) {
      ogl_draw_face_seg( 
         vvr_fix_endian_16( poly->coords[i - 1].x ),
         vvr_fix_endian_16( poly->coords[i - 1].y ),
         vvr_fix_endian_16( poly->coords[i].x ),
         vvr_fix_endian_16( poly->coords[i].y ),
         cx, cy, cx, cy, y_edges, y_center, color );
   }

   /* Draw final top face. */
   ogl_draw_face_seg( 
      vvr_fix_endian_16( poly->coords[i - 1].x ),
      vvr_fix_endian_16( poly->coords[i - 1].y ),
      vvr_fix_endian_16( poly->coords[0].x ),
      vvr_fix_endian_16( poly->coords[0].y ),
      cx, cy, cx, cy, y_edges, y_center, color );
}

/* === */

void ogl_draw_poly( struct VVR_SECT_POLY* poly, float* color ) {
   int i = 0;

   /* Iterate around each side. */
   for( i = 1 ; vvr_fix_endian_32( poly->coords_ct ) > i ; i++ ) {
      if( VVR_POLYPROF_SOLID == poly->vprofile ) {
         /* Solid shape gets vertical walls. */
         ogl_draw_face_seg( 
            vvr_fix_endian_16( poly->coords[i - 1].x ),
            vvr_fix_endian_16( poly->coords[i - 1].y ),
            vvr_fix_endian_16( poly->coords[i].x ),
            vvr_fix_endian_16( poly->coords[i].y ),
            vvr_fix_endian_16( poly->coords[i - 1].x ),
            vvr_fix_endian_16( poly->coords[i - 1].y ),
            vvr_fix_endian_16( poly->coords[i].x ),
            vvr_fix_endian_16( poly->coords[i].y ),
            0, STATIC_HEIGHT, color );
      }
   }
   if( VVR_POLYPROF_SOLID == poly->vprofile ) {
      /* Solid shape gets one last vertical wall. */
      ogl_draw_face_seg( 
         vvr_fix_endian_16( poly->coords[i - 1].x ),
         vvr_fix_endian_16( poly->coords[i - 1].y ),
         vvr_fix_endian_16( poly->coords[0].x ), /* Wrap around to the */
         vvr_fix_endian_16( poly->coords[0].y ), /* first coord! */
         vvr_fix_endian_16( poly->coords[i - 1].x ),
         vvr_fix_endian_16( poly->coords[i - 1].y ),
         vvr_fix_endian_16( poly->coords[0].x ),
         vvr_fix_endian_16( poly->coords[0].y ),
         0, STATIC_HEIGHT, color );
   }

   /* Draw top (flat or angled segments that converge in the center. */
   ogl_draw_top( poly, STATIC_HEIGHT, 0, color );
}

/* === */

void ogl_opengl_frame() {
   uint8_t* next = NULL;
   int i = 0, j = 0, k = 0;
   struct VVR_SECT_POSN* posn = NULL;
   struct VVR_SECT_GENERIC* prsm = NULL;
   struct VVR_SECT_POLY* poly = NULL;
   struct VVR_SECT_COLR* colr = NULL;
   float color[4] = { 0, 0, 0, 1.0f };

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

      color[0] = colr->color1.r;
      color[1] = colr->color1.g;
      color[2] = colr->color1.b;

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

         /* Draw the geometry into vertices. */
         ogl_draw_poly( poly, color );

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

