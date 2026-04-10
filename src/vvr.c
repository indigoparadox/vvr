
#include "vvr.h"

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
         sect->head.section[0],
         sect->head.section[1],
         sect->head.section[2],
         sect->head.section[3],
         vvr_fix_endian_32( sect->head.sz ), *cursor );
#endif /* VVR_DEBUG */

      sect = (struct VVR_SECT_GENERIC*)&(buf[*cursor]);
      if( 0 == strncmp( sect->head.section, "FORM", 4 ) ) {
         /* Detect FORM, which has weird VMDL trailing tag which otherwise
          * breaks parsing.
          */
#ifdef VVR_DEBUG
         printf( "skipping FORM\n" );
#endif /* VVR_DEBUG */
         *cursor += 12;
         continue;

      } else if( 0 == strncmp( sect->head.section, "ROOT", 4 ) ) {
         /* Skip ROOT head, since it encompasses many useful sections. */
#ifdef VVR_DEBUG
         printf( "skipping ROOT\n" );
#endif /* VVR_DEBUG */
         *cursor += 8;
         continue;
      }

      if( 0 == strncmp( sect->head.section, find, 4 ) ) {
         /* Skip the cursor and return the found struct. */
         return &(buf[*cursor]);
      }

      /* Skip to the next opaque section. */
#ifdef VVR_DEBUG
      printf( "skipping %d + %d from 0x%x...\n",
         vvr_fix_endian_32( sect->head.sz ), sizeof( struct VVR_SECT_HEAD ),
         *cursor );
#endif /* VVR_DEBUG */
      *cursor += vvr_fix_endian_32( sect->head.sz ) +
         sizeof( struct VVR_SECT_HEAD );
   }

   return NULL;
}

