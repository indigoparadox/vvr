
#include "parse.h"

#include <stdio.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>

void dump_posn( struct VVR_SECT_POSN* posn, uint8_t verbose, uint8_t cols ) {
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

void dump_bytes( uint8_t* buf, int buf_sz, uint8_t verbose, uint8_t cols ) {
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

void dump_poly( struct VVR_SECT_POLY* poly, uint8_t verbose, uint8_t cols ) {
   const char* shape_str = NULL,
      * shape_cub = "cube/pyramid",
      * shape_cyl = "cylinder",
      * shape_sph = "sphere",
      * shape_unk = "unknown";
   int seg_count = (vvr_fix_endian_32( poly->head.sz ) - 30) / 8;
   int i = 0;

   switch( poly->head.sz ) {
      case VVR_POLY_SZ_RECT:
         shape_str = shape_cub;
         break;

      case VVR_POLY_SZ_CIRCLE:
         shape_str = poly->vsegs > 1 ? shape_sph : shape_cyl;
         break;

      default:
         shape_str = shape_unk;
         break;
   }

   printf( "poly: %s (%d segs): ", shape_str, seg_count );
   for( i = 0 ; seg_count > i ; i++ ) {
      printf( "(%d) %d.%d, %d.%d; ", i,
         poly->coords[i].x.integer,
         poly->coords[i].x.fraction,
         poly->coords[i].y.integer,
         poly->coords[i].y.fraction );
   }
   printf( "\n" );
}

/* === */

void dump_color( struct VVR_COLOR* color, uint8_t verbose, uint8_t cols ) {
   printf( "r: 0x%02x, g: 0x%02x, b: 0x%02x\n", color->r, color->g, color->b );
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
   uint8_t* next = NULL;
   int i = 0, j = 0;
   struct VVR_SECT_POSN* posn = NULL;
   struct VVR_SECT_GENERIC* prsm = NULL;
   struct VVR_SECT_POLY* poly = NULL;
   struct VVR_SECT_COLR* colr = NULL;

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

   fclose( vvr_file );

   vvr_form_p = (struct IFF_FORM*)vvr_buf;
   if( verbose ) {
      printf( "form: %c%c%c%c, sz: %u\n", 
         vvr_form_p->form[0], vvr_form_p->form[1],
         vvr_form_p->form[2], vvr_form_p->form[3],
         vvr_fix_endian_32( vvr_form_p->vvr_sz ) );
   }

   while( NULL != (next = next_sect( "PRSM", vvr_buf, vvr_sz, 1, &i )) ) {
      prsm = (struct VVR_SECT_GENERIC*)&(vvr_buf[i]);
      printf( "found PRSM @ 0x%x, diving...\n", i );

      /* Dive into the PRSM section for COLR sections. */
      j = i + sizeof( struct VVR_SECT_HEAD );
      while( NULL != (next = next_sect( "COLR", vvr_buf, vvr_sz, 1, &j )) ) {
         colr = (struct VVR_SECT_COLR*)next;
         dump_color( &(colr->color1), verbose, cols );

         /* Skip to section after POSN (size plus sz/sect fields). */
         j += vvr_fix_endian_32( colr->head.sz ) +
            sizeof( struct VVR_SECT_HEAD );
      }

      /* Dive into the PRSM section for POSN sections. */
      j = i + sizeof( struct VVR_SECT_HEAD );
      while( NULL != (next = next_sect( "POSN", vvr_buf, vvr_sz, 1, &j )) ) {
         posn = (struct VVR_SECT_POSN*)next;
         dump_posn( posn, verbose, cols );

         /* Skip to section after POSN (size plus sz/sect fields). */
         j += vvr_fix_endian_32( posn->head.sz ) +
            sizeof( struct VVR_SECT_HEAD );
      }

      /* Dive into the PRSM section for POLY sections. */
      j = i + sizeof( struct VVR_SECT_HEAD );
      while( NULL != (next = next_sect( "POLY", vvr_buf, vvr_sz, 1, &j )) ) {
         poly = (struct VVR_SECT_POLY*)next;
         dump_poly( poly, verbose, cols );

         /* Skip to section after POSN (size plus sz/sect fields). */
         j += vvr_fix_endian_32( poly->head.sz ) +
            sizeof( struct VVR_SECT_HEAD );
      }
      
      /* Skip to section after PRSM (size plus sz/sect fields). */
      i += vvr_fix_endian_32( prsm->head.sz ) + sizeof( struct VVR_SECT_HEAD );
   }

   return 0;
}

