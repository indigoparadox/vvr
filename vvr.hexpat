#pragma endian big

import type.magic;

struct Fixed_Point_32 {
    u16 integer;
    u16 fraction;
};

// Seems to be used by PRISM for vertices... needs study.
struct Coord2D {
    Fixed_Point_32 x;
    Fixed_Point_32 y;
};

// 3-couplets of numbers to control 3D properties.
struct Coord3D {
    Fixed_Point_32 x;
    Fixed_Point_32 y;
    Fixed_Point_32 z;
};

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

struct IFF_Chunk_PRSM_Dispatch {
    char id[4];
    u32 size;
    
    // Parse a PRSM (polygon in the scene) chunk.

    match (id) {
        ("POLY"): {
            auto start_offset = $;
            Coord2D vertices[while( $ < start_offset + size )];
        }
        ("POSN"): {
            // Seems to control polygon position and rotation.
            auto start_offset = $;
            Coord3D position;
            Coord3D rotation;
            Coord3D un1; // ???
            Coord3D un2; // ???
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
        IFF_Chunk_PRSM_Dispatch chunks[while( $ < start_offset + size )];
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
        IFF_Chunk_ROOT_Dispatch chunks[while( $ < start_offset + size )];
        
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
    IFF_Chunk_Dispatch chunks[while( $ < size )];
};

// =========================
// Root
// =========================

IFF_FORM f @ 0x0;