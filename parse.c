
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <unistd.h>

#define fix_endian_32( x ) (((x >> 24) & 0xff) | ((x << 8) & 0xff0000) | ((x >> 8) & 0xff00) | ((x << 24) & 0xff000000))

#define fix_endian_16( x ) (((x << 8) & 0xff) | ((x >> 8) & 0xff))

struct IFF_FORM {
   char form[4];
   uint32_t vvr_sz;
};

/* === */

void dump_posn(
   uint8_t* buf, size_t buf_sz, int depth, uint8_t verbose, uint8_t cols
) {
   int i = 0,
      j = 0;
   int16_t* pos_dec_p = NULL;
   int16_t* pos_frac_p = NULL;

   /*
   while( j * 4 < buf_sz ) {
      for( i = 0 ; depth > i ; i++ ) {
         printf( " " );
      }
      pos_dec_p = (int16_t*)&(buf[j * 4]);
      pos_frac_p = (int16_t*)&(buf[(j * 4) + 2]);
      printf(
         "posn %d.%d\n",
         (int16_t)fix_endian_16( *pos_dec_p ),
         (int16_t)fix_endian_16( *pos_frac_p )
         );
      j++;
   }
   */

   printf( "pos: X: %d.%d, Y: %d.%d, Z: %d.%d\n",
      fix_endian_16( *(int16_t*)&(buf[0]) ), /* X.dec */
      fix_endian_16( *(int16_t*)&(buf[2]) ), /* X.frac */
      fix_endian_16( *(int16_t*)&(buf[4]) ), /* Y.dec */
      fix_endian_16( *(int16_t*)&(buf[6]) ), /* Y.frac */
      fix_endian_16( *(int16_t*)&(buf[8]) ), /* Z.dec */
      fix_endian_16( *(int16_t*)&(buf[10]) ) /* Z.frac */
      );
      
}

/* === */

void dump_raw(
   uint8_t* buf, size_t buf_sz, int depth, uint8_t verbose, uint8_t cols
) {
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

void dump_poly(
   uint8_t* buf, size_t buf_sz, int depth, uint8_t verbose, uint8_t cols
) {
   const char* shape_str = NULL,
      * shape_cub = "cube/pyramid",
      * shape_cyl = "cylinder",
      * shape_sph = "sphere",
      * shape_unk = "unknown";
   uint16_t vsegments = fix_endian_16( *((uint16_t*)&(buf[4])) );

      switch( buf_sz ) {
         case 64:
            shape_str = shape_cub;
            break;

         case 160:
            shape_str = vsegments > 1 ? shape_sph : shape_cyl;
            break;

         default:
            shape_str = shape_unk;
            break;
      }

      printf( "=== found POLY (%s) ===\n", shape_str, vsegments );
      dump_raw( buf, buf_sz, depth, verbose, cols );
}

/* === */

void dump_sect(
   uint8_t* buf, size_t buf_sz, int depth, uint8_t verbose, uint8_t cols
) {
   size_t dump_at = 0;
   int i = 0;
   struct IFF_FORM* vvr_form_p;

   while( dump_at < buf_sz ) {
      if( verbose ) {
         for( i = 0 ; depth > i ; i++ ) {
            printf( " " );
         }
         printf( "reading at offset 0x%04lx of 0x%04lx:\n", dump_at, buf_sz );
      }
      vvr_form_p = (struct IFF_FORM*)&(buf[dump_at]);
      if( 0 != buf[dump_at + 4] ) {
         dump_at += 4;
      } else {
         if( verbose ) {
            for( i = 0 ; depth > i ; i++ ) {
               printf( " " );
            }
            printf( "form: %c%c%c%c, sz: 0x%04x\n", 
               vvr_form_p->form[0], vvr_form_p->form[1],
               vvr_form_p->form[2], vvr_form_p->form[3],
               fix_endian_32( vvr_form_p->vvr_sz ) );
         }

         if(
            0 == strncmp( &(buf[dump_at]), "ROOT", 4 ) ||
            0 == strncmp( &(buf[dump_at]), "PRSM", 4 )
         ) {
            printf( "\n+++ found ROOT/PRSM +++\n" );
            assert( fix_endian_32( vvr_form_p->vvr_sz ) < buf_sz );

            /* Dive into divable section. */
            dump_sect( &(buf[dump_at + 8]),
               fix_endian_32( vvr_form_p->vvr_sz ), depth + 1, verbose, cols );

         } else if(
            0 == strncmp( &(buf[dump_at]), "POSN", 4 )
         ) {
            printf( "=== found POSN ===\n" );
            dump_posn( &(buf[dump_at + 8]),
               fix_endian_32( vvr_form_p->vvr_sz ), depth + 1, verbose, cols );

         } else if(
            0 == strncmp( &(buf[dump_at]), "POLY", 4 )
         ) {
            dump_poly( &(buf[dump_at + 8]),
               fix_endian_32( vvr_form_p->vvr_sz ), depth + 1, verbose, cols );
         }

         dump_at += fix_endian_32( vvr_form_p->vvr_sz ) + 8;
      }
   }
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
         fix_endian_32( vvr_form_p->vvr_sz ) );
   }

   dump_sect( &(vvr_buf[sizeof( struct IFF_FORM )]), vvr_sz, 1, verbose, cols );

   return 0;
}

