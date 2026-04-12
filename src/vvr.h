
#ifndef VVR_H
#define VVR_H

#include <stdint.h>
#include <stddef.h>

#define VVR_POLYPROF_SOLID 1
#define VVR_POLYPROF_PYRAMID 2
#define VVR_POLYPROF_SYMMETRIC 3
#define VVR_POLYPROF_DOME 4

#define vvr_fix_endian_32( x ) (((x >> 24) & 0xff) | ((x << 8) & 0xff0000) | ((x >> 8) & 0xff00) | ((x << 24) & 0xff000000))

#define vvr_fix_endian_16( x ) \
   (int16_t)(((uint16_t)(x) >> 8) | ((uint16_t)(x) << 8))

#define vvr_float_from_fix( fp ) \
   ((float)((vvr_fix_endian_16( (fp)->integer ) << 16) | \
      (vvr_fix_endian_16( (fp)->fraction ))) / 65535.0f)

struct IFF_FORM {
   char form[4];
   uint32_t vvr_sz;
   char vmdl[4];
};

struct VVR_COORD_2D {
   int16_t x;
   uint16_t x_pad;
   int16_t y;
   uint16_t y_pad;
} __attribute__((packed));

struct VVR_FP32 {
   uint16_t integer;
   uint16_t fraction;
} __attribute__((packed));

struct VVR_COORD_3D {
   struct VVR_FP32 x;
   struct VVR_FP32 y;
   struct VVR_FP32 z;
} __attribute__((packed));

struct VVR_COLOR {
   uint8_t u1;
   uint8_t r;
   uint8_t g;
   uint8_t b;
} __attribute__((packed));

struct VVR_SECT_HEAD {
   char section[4];
   uint32_t sz;
} __attribute__((packed));

struct VVR_SECT_GENERIC {
   struct VVR_SECT_HEAD head;
   uint8_t data[];
} __attribute__((packed));

struct VVR_SECT_POLY {
   struct VVR_SECT_HEAD head;
   uint8_t u1[3];
   uint8_t vprofile;
   uint16_t vsegs;
   int16_t z1;
   uint16_t u2;
   int16_t z2;
   uint32_t u3[4];
   uint32_t coords_ct;
   struct VVR_COORD_2D coords[];
} __attribute__((packed));

struct VVR_SECT_POSN {
   struct VVR_SECT_HEAD head;
   struct VVR_FP32 x;
   struct VVR_FP32 y;
   struct VVR_FP32 z;
   struct VVR_FP32 rx;
   struct VVR_FP32 ry;
   struct VVR_FP32 rz;
} __attribute__((packed));

struct VVR_SECT_COLR {
   struct VVR_SECT_HEAD head;
   struct VVR_COLOR color1;
   struct VVR_COLOR color2;
} __attribute__((packed));

struct VVR_PARSER {

};

uint8_t* next_sect(
   const char* sect, uint8_t* buf, size_t buf_sz, int depth, int* cursor );

#endif /* VVR_H */

