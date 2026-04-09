
#ifndef PARSE_H
#define PARSE_H

#include <stdint.h>
#include <stddef.h>

#define VVR_POLY_SZ_RECT 64
#define VVR_POLY_SZ_CIRCLE 160

#define vvr_fix_endian_32( x ) (((x >> 24) & 0xff) | ((x << 8) & 0xff0000) | ((x >> 8) & 0xff00) | ((x << 24) & 0xff000000))

#define vvr_fix_endian_16( x ) (((x << 8) & 0xff) | ((x >> 8) & 0xff))

struct IFF_FORM {
   char form[4];
   uint32_t vvr_sz;
   char vmdl[4];
};

struct VVR_FP32 {
   uint16_t integer;
   uint16_t fraction;
} __attribute__((packed));

struct VVR_COORD_3D {
   struct VVR_FP32 x;
   struct VVR_FP32 y;
   struct VVR_FP32 z;
} __attribute__((packed));

struct VVR_SECT_GENERIC {
   char section[4];
   uint32_t sz;
   uint8_t data[];
} __attribute__((packed));

struct VVR_SECT_POLY {
   char section[4];
   uint32_t sz;
} __attribute__((packed));

struct VVR_SECT_POSN {
   char section[4];
   uint32_t sz;
   struct VVR_FP32 x;
   struct VVR_FP32 y;
   struct VVR_FP32 z;
} __attribute__((packed));

struct VVR_PARSER {

};

uint8_t* next_sect(
   const char* sect, uint8_t* buf, size_t buf_sz, int depth, int* cursor );

#endif /* PARSE_H */

