
#include "parse.h"

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>

void dump_posn( uint8_t* buf, size_t buf_sz, uint8_t verbose, uint8_t cols ) {
   int i = 0,
      j = 0;
   int16_t* pos_dec_p = NULL;
   int16_t* pos_frac_p = NULL;
   struct VVR_SECT_POSN* posn = (struct VVR_SECT_POSN*)buf;

   printf( "pos: X: %d.%d, Y: %d.%d, Z: %d.%d\n",
      vvr_fix_endian_16( posn->x.integer ), /* X.dec */
      vvr_fix_endian_16( posn->x.fraction ), /* X.frac */
      vvr_fix_endian_16( posn->y.integer ), /* Y.dec */
      vvr_fix_endian_16( posn->y.fraction ), /* Y.frac */
      vvr_fix_endian_16( posn->z.integer ), /* Z.dec */
      vvr_fix_endian_16( posn->z.fraction ) /* Z.frac */
      );
      
}

/* === */

void dump_raw( uint8_t* buf, size_t buf_sz, uint8_t verbose, uint8_t cols ) {
   int i = 0;

   for( i = 1 ; buf_sz + 1 > i ; i++ ) {
      printf( "0x%02x ", buf[i - 1] );
      if( 0 == i % cols ) {
         printf( "\n", i );
      }
   }
   if( 1 != i % cols ) {
      printf( "\n" );
   }
}

/* === */

void dump_poly( uint8_t* buf, size_t buf_sz, uint8_t verbose, uint8_t cols ) {
   const char* shape_str = NULL,
      * shape_cub = "cube/pyramid",
      * shape_cyl = "cylinder",
      * shape_sph = "sphere",
      * shape_unk = "unknown";
   uint16_t vsegments = vvr_fix_endian_16( *((uint16_t*)&(buf[4])) );

   switch( buf_sz ) {
      case VVR_POLY_SZ_RECT:
         shape_str = shape_cub;
         break;

      case VVR_POLY_SZ_CIRCLE:
         shape_str = vsegments > 1 ? shape_sph : shape_cyl;
         break;

      default:
         shape_str = shape_unk;
         break;
   }

   printf( "=== found POLY (%s) ===\n", shape_str, vsegments );
   dump_raw( buf, buf_sz, verbose, cols );
}

/* === */

int main( int argc, char* argv[] ) {
   FILE* vvr_file = NULL;
   uint8_t* vvr_buf = NULL;
   size_t vvr_sz = 0,
      vvr_read = 0;
   struct IFF_FORM* vvr_form_p;
   uint8_t verbose = 0;
   char c = 0;
   char* vvr_path = NULL;
   uint8_t cols = 16;
   struct VVR_SECT_POSN* posn = NULL;
   uint8_t* next = NULL;
   int cursor = 0;

   while( -1 != (c = getopt( argc, argv, ":vc:" )) ) {
      switch( c ) {
         case 'v':
            verbose = 1;
            break;

         case 'c':
            cols = atoi( optarg );
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

   vvr_form_p = (struct IFF_FORM*)vvr_buf;
   if( verbose ) {
      printf( "form: %c%c%c%c, sz: %u\n", 
         vvr_form_p->form[0], vvr_form_p->form[1],
         vvr_form_p->form[2], vvr_form_p->form[3],
         vvr_fix_endian_32( vvr_form_p->vvr_sz ) );
   }

   while( NULL != (next = next_sect(
      "POSN", vvr_buf, vvr_sz, 1, &cursor
   )) ) {
      posn = (struct VVR_SECT_POSN*)next;
      dump_posn( next, posn->sz, verbose, cols );
   }

   return 0;
}

