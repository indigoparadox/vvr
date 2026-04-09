
#include "parse.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <GL/gl.h>
#include <GL/glut.h>

#define OGL_SCREEN_W 1024
#define OGL_SCREEN_H 768

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
      0.5f, 20.0f );
   glMatrixMode( GL_MODELVIEW );
   glClearColor( 0, 0, 0, 0 );
   glEnable( GL_CULL_FACE );
   glEnable( GL_NORMALIZE );
   glEnable( GL_DEPTH_TEST );

   return 0;
}

/* === */

void ogl_opengl_frame() {
   glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
   glPushMatrix();

   glTranslatef( 0, 0, -3.0f );

	/* TODO: Redraw loaded objects. */

   glPopMatrix();
   glFlush();

   glutSwapBuffers();
}

/* === */

int ogl_setup_vvr( uint8_t* buf, size_t sz ) {
   int retval = 0;

   return retval;
}

/* === */

int main( int argc, char* argv[] ) {
	int retval = 0;
   uint8_t verbose = 0;
   char c = 0;
   FILE* vvr_file = NULL;
   uint8_t* vvr_buf = NULL;
   char* vvr_path = NULL;
   size_t vvr_sz = 0,
      vvr_read = 0;

   /* Setup OpenGL. */

	glutInit( &argc, argv );
	glutInitWindowSize( OGL_SCREEN_W, OGL_SCREEN_H );
	glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
	glutCreateWindow( "VVR Test" );

   retval = ogl_opengl_setup();  

   glutDisplayFunc( ogl_opengl_frame );

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
   vvr_sz = ftell( vvr_file );
   fseek( vvr_file, 0, SEEK_SET );

   vvr_buf = calloc( vvr_sz, 1 );
   assert( NULL != vvr_buf );

   vvr_read = fread( vvr_buf, 1, vvr_sz, vvr_file );
   assert( vvr_read == vvr_sz );

   fclose( vvr_file );

   retval = ogl_setup_vvr( vvr_buf, vvr_sz );
   assert( 0 == retval );

   /* Start! */

   glutMainLoop();

	return retval;
}

