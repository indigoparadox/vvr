#pragma endian big

import type.magic;

struct Fixed_Point_32 {
    u16 integer;
    u16 fraction;
} [[format_read( "print_fp" ), format_write( "read_fp" )]];

fn print_fp( auto v ) {
    return std::format( "{}.{}", v.integer, v.fraction );
};

fn read_fp( str v ) {
    /* s16 str_sz = std::string::length( v );
    s16 comma_pos = -1;
    for( s16 i = 0, str_sz > i, i += 1 ) {
        if( "," == std::string::at( v, i ) ) {
            comma_pos = i;
            break;
        }
    }
    str str_int = std::string::substr( v, 0, comma_pos );
    str str_frac = std::string::substr( v, comma_pos + 1, str_sz - (comma_pos + 1) ); */
    return std::string::parse_float( v, 0 );
};

// Seems to be used by PRISM for vertices... needs study.
struct Coord2D {
    s16 x;
    u16 x_pad;
    s16 y;
    u16 y_pad;
} [[format_read( "print_coord2d" ), format_write( "print_coord2d" )]];

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
    u32 size;
    Coord3D position;
    Coord3D rotation;
    Coord3D un1; // ???
    Coord3D zoom;
};

struct IFF_Chunk_COLR {
    u32 size;
    VVRColor color1;
    VVRColor color2;
} [[hex::inline_visualize("color", color1.r, color1.g, color1.b, 255)]];

using IFF_Chunk_POLY;
fn plot_poly_coords( IFF_Chunk_POLY p, u32 xy ) {
    float xy_out[p.num_coords];
    
    for( s16 i = 0, p.num_coords > i, i += 1 ) {
        if( 0 == xy ) {
            xy_out[i] = float( p.coords[i].x );
        } else {
            xy_out[i] = float( p.coords[i].y );
        }
    }
    
    return xy_out;
};

struct IFF_Chunk_POLY {
    u32 size;
    auto start_offset = $;
    u8 u1a; // ???
    u8 u1b; // ???
    u8 u1c; // ???
    u8 u1d; // ???
    u16 vertical_segments;
    u16 vertical_scale; // ???
    u16 u2; // ???
    u16 height; // ???
    u32 u3;
    u32 u4;
    u32 u5;
    u32 u6;
    u32 num_coords;
    Coord2D coords[while( $ < start_offset + size )];
} [[hex::visualize( "scatter_plot", plot_poly_coords( this, 0 ), plot_poly_coords( this, 1 ) )]];

struct IFF_Chunk_PGRP_Dispatch {
    char id[4];
    
    // Parse a PGRP (group of polygons in the scene) chunk.

    if( id == "PRSM" ) {
        // The PRSM chunk contains a bunch of chunks inside of it.
        u32 size;
        auto start_offset = $;
        IFF_Chunk_PRSM_Dispatch chunks[while( $ < start_offset + size )][[inline]];

    } else {
        match (id) {
            ("POSN"): IFF_Chunk_POSN [[inline]];
            ("POLY"): IFF_Chunk_POLY [[inline]];
            ("COLR"): IFF_Chunk_COLR [[inline]];
            (_): {
                u32 size;
                u8 raw[size];
            }
        }
    }
} [[ name( id )]];

struct IFF_Chunk_PRSM_Dispatch {
    char id[4];
    
    // Parse a PRSM (polygon in the scene) chunk.

    match (id) {
        ("POSN"): IFF_Chunk_POSN [[inline]];
        ("POLY"): IFF_Chunk_POLY;
        ("COLR"): IFF_Chunk_COLR [[inline]];
        (_): {
            u32 size;
            u8 raw[size];
        }
    }
} [[ name( id )]];

struct IFF_Chunk_ROOT_Dispatch {
    char id[4];
    
    // Parse 3D scene data children off of the ROOT chunk.

    if( id == "PRSM" ) {
        // The PRSM chunk contains a bunch of chunks inside of it.
        u32 size;
        auto start_offset = $;
        IFF_Chunk_PRSM_Dispatch chunks[while( $ < start_offset + size )][[inline]];

    } else if( id == "PGRP" ) {
        // The PRSM chunk contains a bunch of chunks inside of it.
        u32 size;
        auto start_offset = $;
        IFF_Chunk_PGRP_Dispatch chunks[while( $ < start_offset + size )][[inline]];

    } else {
        match (id) {
            ("COLR"): IFF_Chunk_COLR [[inline]];
            (_): {
                u32 size;
                u8 raw[size];
            }
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