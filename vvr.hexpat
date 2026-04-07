#pragma endian big

import type.magic;

struct Fixed_Point_32 {
    u16 integer;
    u16 fraction;
};

struct Coord2D {
    Fixed_Point_32 x;
    Fixed_Point_32 y;
};

struct Coord3D {
    Fixed_Point_32 x;
    Fixed_Point_32 y;
    Fixed_Point_32 z;
};

// =========================
// Chunk Dispatchers
// =========================

struct IFF_Chunk_PREF_Dispatch {
    char id[4];
    u32 size;

    match (id) {
        (_): u8 raw[size];
    }
};

///

struct IFF_Chunk_PRSM_Dispatch {
    char id[4];
    u32 size;

    match (id) {
        ("POLY"): {
            auto start_offset = $;
            Coord2D vertices[while( $ < start_offset + size )];
        }
        ("POSN"): {
            auto start_offset = $;
            Coord3D pos;
            Coord3D rot;
            Coord3D un1; // ???
            Coord3D un2; // ???
        }
        (_): u8 raw[size];
    }
};

///

struct IFF_Chunk_ROOT_Dispatch {
    char id[4];
    u32 size;

    if( id == "PRSM" ) {
        // The PRSM chunk contains a bunch of chunks inside of it.
        auto start_offset = $;
        IFF_Chunk_PRSM_Dispatch chunks[while( $ < start_offset + size )];
    } else {
        match (id) {
            (_): u8 raw[size];
        }
    }
};

///

struct IFF_Chunk_Dispatch {
    char id[4];
    u32 size;
    
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
};

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