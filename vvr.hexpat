#pragma endian big

import type.magic;

struct Fixed_Point_32 {
    u16 integer;
    u16 fraction;
} [[format_read( "print_fp32" )]];

fn print_fp32( auto v ) {
    return std::format( "{}.{}", v.integer, v.fraction );
};

// Seems to be used by PRISM for vertices... needs study.
struct Coord2D {
    Fixed_Point_32 x;
    Fixed_Point_32 y;
} [[format_read( "print_coord2d" )]];

fn print_coord2d( auto v ) {
    return std::format( "{}, {}", v.x, v.y );
};

// 3-couplets of numbers to control 3D properties.
struct Coord3D {
    Fixed_Point_32 x;
    Fixed_Point_32 y;
    Fixed_Point_32 z;
} [[format_read( "print_coord3d" )]];

fn print_coord3d( auto v ) {
    return std::format( "{}, {}, {}", v.x, v.y, v.z );
};

struct VVRColor {
    u8 u1;
    u8 r;
    u8 g;
    u8 b;
} [[hex::inline_visualize("color", r, g, b, 255)]];

// =========================
// Handle chunks inside PREF (config data).
// =========================

struct IFF_Chunk_PREF_Dispatch {
    char id[4];
    u32 size;

    match (id) {
        (_): u8 raw[size];
    }
} [[ name( id )]];

// =========================
/// Handle chunks inside ROOT (3D scene data).
// =========================

using IFF_Chunk_PRSM_Dispatch;

struct IFF_Chunk_POSN {
    Coord3D position;
    Coord3D rotation;
    Coord3D un1; // ???
    Coord3D zoom;
};

struct IFF_Chunk_PGRP_Dispatch {
    char id[4];
    u32 size;
    
    // Parse a PRSM (polygon in the scene) chunk.

    if( id == "PRSM" ) {
        // The PRSM chunk contains a bunch of chunks inside of it.
        auto start_offset = $;
        IFF_Chunk_PRSM_Dispatch chunks[while( $ < start_offset + size )][[inline]];
    } else {
        match (id) {
            ("POSN"): IFF_Chunk_POSN [[inline]];
            (_): u8 raw[size];
        }
    }
} [[ name( id )]];

struct IFF_Chunk_PRSM_Dispatch {
    char id[4];
    u32 size;
    
    // Parse a PRSM (polygon in the scene) chunk.

    match (id) {
        ("POLY"): {
            auto start_offset = $;
            Coord2D vertices[while( $ < start_offset + size )];
        }
        ("POSN"): IFF_Chunk_POSN [[inline]];
        ("COLR"): {
            VVRColor color1;
            VVRColor color2;
        }
        (_): u8 raw[size];
    }
} [[ name( id )]];

struct IFF_Chunk_ROOT_Dispatch {
    char id[4];
    u32 size;
    
    // Parse 3D scene data children off of the ROOT chunk.

    if( id == "PRSM" ) {
        // The PRSM chunk contains a bunch of chunks inside of it.
        auto start_offset = $;
        IFF_Chunk_PRSM_Dispatch chunks[while( $ < start_offset + size )][[inline]];
    } else if( id == "PGRP" ) {
        // The PRSM chunk contains a bunch of chunks inside of it.
        auto start_offset = $;
        IFF_Chunk_PGRP_Dispatch chunks[while( $ < start_offset + size )][[inline]];
    } else {
        match (id) {
            (_): u8 raw[size];
        }
    }
} [[ name( id )]];

// =========================
/// Handle top-level chunks under FORM.
// =========================

struct IFF_Chunk_Dispatch {
    char id[4];
    u32 size;
    
    // Parse chunks off of the root FORM, of which PREF and ROOT have children.
    
    if( id == "PREF" ) {
        // The PREF chunk contains a FORM with various other chunks inside of it.
        type::Magic<"FORM"> form;
        u32 form_size;
        type::Magic<"VPRF"> vprf;
        IFF_Chunk_PREF_Dispatch chunks[while( $ < form_size )];
        
    } else if( id == "ROOT" ) {
        // The ROOT chunk contains a bunch of chunks inside of it.
        auto start_offset = $;
        IFF_Chunk_ROOT_Dispatch chunks[while( $ < start_offset + size )][[inline]];
        
    } else {
        match (id) {
            (_): { u8 raw[size]; }
        }
    }
} [[ name( id )]];

struct IFF_FORM {
    type::Magic<"FORM"> id;
    u32 size;
    type::Magic<"VMDL"> vmdl;
    IFF_Chunk_Dispatch chunks[while( $ < size )][[inline]];
};

// =========================
// Root
// =========================

IFF_FORM f @ 0x0;