
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#define fix_endian( x ) (((x >> 24) & 0xff) | ((x << 8) & 0xff0000) | ((x >> 8) & 0xff00) | ((x << 24) & 0xff000000))

struct IFF_FORM {
   char form[4];
   uint32_t vvr_sz;
};

void dump_posn( uint8_t* buf, size_t buf_sz, int depth ) {
   int i = 0,
      j = 0;
   uint32_t* pos_p = NULL;

   while( j * 4 < buf_sz ) {
      for( i = 0 ; depth > i ; i++ ) {
         printf( " " );
      }
      pos_p = (uint32_t*)&(buf[j * 4]);
      printf(
         "posn %d: les: 0x%08x/%d, bes: 0x%08x/%d, leu: %u, beu: %u, "
         "lef: %f, bef: %f, bed: %f)\n",
         j,
         (int32_t)fix_endian( *pos_p ),
         (int32_t)fix_endian( *pos_p ),
         (int32_t)(*pos_p),
         (int32_t)(*pos_p),
         (int32_t)fix_endian( *pos_p ),
         (uint32_t)(*pos_p),
         (float)fix_endian( *pos_p ),
         (float)(*pos_p),
         (double)(*pos_p)
         );
      j++;
   }
}

void dump_sect( uint8_t* buf, size_t buf_sz, int depth ) {
   size_t dump_at = 0;
   int i = 0;
   struct IFF_FORM* vvr_form_p;

   while( dump_at < buf_sz ) {
      for( i = 0 ; depth > i ; i++ ) {
         printf( " " );
      }
      printf( "reading at offset 0x%04lx of 0x%04lx:\n", dump_at, buf_sz );
      vvr_form_p = (struct IFF_FORM*)&(buf[dump_at]);
      if( 0 != buf[dump_at + 4] ) {
         dump_at += 4;
      } else {
         for( i = 0 ; depth > i ; i++ ) {
            printf( " " );
         }
         printf( "form: %c%c%c%c, sz: 0x%04x\n", 
            vvr_form_p->form[0], vvr_form_p->form[1],
            vvr_form_p->form[2], vvr_form_p->form[3],
            fix_endian( vvr_form_p->vvr_sz ) );

         if(
            0 == strncmp( &(buf[dump_at]), "ROOT", 4 ) ||
            0 == strncmp( &(buf[dump_at]), "PRSM", 4 )
         ) {
            /* Dive into divable section. */
            assert( fix_endian( vvr_form_p->vvr_sz ) < buf_sz );
            dump_sect( &(buf[dump_at + 8]),
               fix_endian( vvr_form_p->vvr_sz ), depth + 1 );

         } else if(
            0 == strncmp( &(buf[dump_at]), "POSN", 4 )
         ) {
            dump_posn( &(buf[dump_at + 8]),
               fix_endian( vvr_form_p->vvr_sz ), depth + 1 );
         }

         dump_at += fix_endian( vvr_form_p->vvr_sz ) + 8;
      }
   }
}

int main( int argc, char* argv[] ) {
   FILE* vvr_file = NULL;
   uint8_t* vvr_buf = NULL;
   size_t vvr_sz = 0,
      vvr_read = 0;
   struct IFF_FORM* vvr_form_p;

   vvr_file = fopen( argv[1], "rb" );
   fseek( vvr_file, 0, SEEK_END );
   vvr_sz = ftell( vvr_file );
   fseek( vvr_file, 0, SEEK_SET );

   vvr_buf = calloc( vvr_sz, 1 );
   assert( NULL != vvr_buf );

   vvr_read = fread( vvr_buf, 1, vvr_sz, vvr_file );
   assert( vvr_read == vvr_sz );

   vvr_form_p = (struct IFF_FORM*)vvr_buf;
   printf( "form: %c%c%c%c, sz: %u\n", 
      vvr_form_p->form[0], vvr_form_p->form[1],
      vvr_form_p->form[2], vvr_form_p->form[3],
      fix_endian( vvr_form_p->vvr_sz ) );

   dump_sect( &(vvr_buf[sizeof( struct IFF_FORM )]), vvr_sz, 1 );

   return 0;
}

