
#include "parse.h"

#include <assert.h>
#include <string.h>
#ifdef VVR_DEBUG
#  include <stdio.h>
#endif /* VVR_DEBUG */

uint8_t* next_sect(
   const char* find, uint8_t* buf, size_t buf_sz, int depth, int* cursor
) {
   int i = 0;
   struct VVR_SECT_GENERIC* sect = NULL;

   while( *cursor < buf_sz ) {
      sect = (struct VVR_SECT_GENERIC*)&(buf[*cursor]);

#ifdef VVR_DEBUG
      printf( "sect: \"%c%c%c%c\" - %d@0x%x\n",
         sect->section[0],
         sect->section[1],
         sect->section[2],
         sect->section[3],
         vvr_fix_endian_32( sect->sz ), *cursor );
#endif /* VVR_DEBUG */

      /* Detect FORM, which has weird VMDL trailing tag which otherwise breaks
       * parsing.
       */
      sect = (struct VVR_SECT_GENERIC*)&(buf[*cursor]);
      if( 0 == strncmp( sect->section, "FORM", 4 ) ) {
#ifdef VVR_DEBUG
         printf( "skipping FORM\n" );
#endif /* VVR_DEBUG */
         *cursor += 12;
         continue;
      }

      if(
         /* Check for divable section. */
         0 == strncmp( &(buf[*cursor]), "ROOT", 4 ) ||
         0 == strncmp( &(buf[*cursor]), "PRSM", 4 )
      ) {
         assert( vvr_fix_endian_32( sect->sz ) < buf_sz );
#ifdef VVR_DEBUG
         printf( "dive\n" );
#endif /* VVR_DEBUG */

         /* Dive into divable section. */
         *cursor += 8;
         continue;

      } else if( 0 == strncmp( &(buf[*cursor]), find, 4 ) ) {
         /* Skip the cursor and return the found struct. */
         *cursor += vvr_fix_endian_32( sect->sz ) + 8;
         return &(buf[*cursor]);
      }

      /* Skip to the next opaque section. */
#ifdef VVR_DEBUG
      printf( "skipping %d from 0x%x...\n",
         vvr_fix_endian_32( sect->sz ) + 8, *cursor );
#endif /* VVR_DEBUG */
      *cursor += vvr_fix_endian_32( sect->sz ) + 8;
   }

   return NULL;
}

