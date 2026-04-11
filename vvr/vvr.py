
import logging
import struct

CONTAINER_CHUNKS = [b'ROOT', b'PRSM', b'PREF', b'FORM']

def convert_fp( num, frac, frac_sz=16 ) -> float:
    return float( num + (float( frac ) / (2.0 ** float( frac_sz ))) )

def parse_colr( header : tuple[str, int], body : bytes ) -> list[tuple]:

    logger = logging.getLogger( 'parse.sections.colr' )

    body_out = []
    body_out.append( struct.unpack( '>BBBB', body[0:4] ) )
    body_out.append( struct.unpack( '>BBBB', body[4:8] ) )

    return header, body_out

def parse_poly( header : tuple[str, int], body : bytes ):

    logger = logging.getLogger( 'parse.sections.poly' )

    body_out = {}

    print( len( body ) )
    print( type( body ) )
    print( body )
    body_out['meta'] = struct.unpack( '>4BHHHHLLLLL', body[0:32] )

    body_out['coords'] = []
    coord_idx = 0
    while coord_idx < body_out['meta'][12]:
        coord = struct.unpack( '>hHhH',
            body[32 + (coord_idx * 8):32 + (coord_idx * 8) + 8] )
        logger.debug( 'coord %d of %d: %s',
            coord_idx, body_out['meta'][12], str( coord ) )
        body_out['coords'].append( coord )
        coord_idx += 1

    return header, body_out

def parse_posn( header, body ) -> list[tuple]:

    logger = logging.getLogger( 'parse.sections.posn' )

    def _parse_posn_coord( buffer : bytes ):
        dec, frac = struct.unpack( '>HH', buffer )
        return convert_fp( dec, frac )

    def _parse_posn_coord_tuple( buffer : bytes ):
        return (
            _parse_posn_coord( buffer[0:4] ),
            _parse_posn_coord( buffer[4:8] ),
            _parse_posn_coord( buffer[8:12] ) )

    body_out = []
    body_out.append( _parse_posn_coord_tuple( body[0:12] ) )
    body_out.append( _parse_posn_coord_tuple( body[12:24] ) )
    body_out.append( _parse_posn_coord_tuple( body[24:36] ) )
    body_out.append( _parse_posn_coord_tuple( body[36:48] ) )

    return header, body_out

KNOWN_CHUNKS = {
    b'COLR': parse_colr,
    b'POLY': parse_poly,
    b'POSN': parse_posn,
}

def parse_form( vvr_file, header ) -> int:
    frm = vvr_file.read( 8 )
    frm = struct.unpack( '>L4s', frm )
    assert( b'VMDL' == frm[1] )
    sz = frm[0]
    return frm[0]

def parse_file( vvr_file, header=None, sz=0 ):

    logger = logging.getLogger( 'parse.file' )

    tree = {}

    if not header:
        # Read form header.
        frm_header = vvr_file.read( 4 )
        assert( b'FORM' == frm_header )
        sz = parse_form( vvr_file, frm_header )
        logger.debug( 'found form for file of %d bytes...', sz )

    bytes_read = 0
    while( bytes_read < sz ):
        chunk_header = vvr_file.read( 8 )
        if b'' == chunk_header:
            logger.debug( 'file finished!' )
            return header, tree
        bytes_read += 8
        chunk_header = struct.unpack( '>4sL', chunk_header )
        chunk_key = chunk_header[0].decode( 'utf-8' )
        logger.debug( 'found chunk: %s (%d bytes)',
            chunk_header[0], chunk_header[1] )

        if b'FORM' == chunk_header[0]:
            # Drain trailing V* in FORM header.
            vvr_file.read( 4 )

        if chunk_header[0] in CONTAINER_CHUNKS:
            # Delve into parent section.
            chunk_header, chunk_body = parse_file(
                vvr_file, chunk_header, chunk_header[1] )

        else:
            # Read and count the raw bytes for unknown section.
            chunk_body = vvr_file.read( chunk_header[1] )
            if chunk_header[0] in KNOWN_CHUNKS:
                chunk_header, chunk_body = \
                    KNOWN_CHUNKS[chunk_header[0]]( chunk_header, chunk_body )
        bytes_read += chunk_header[1]

        # Store whatever we found in the tree we're building.
        if not chunk_key in tree:
            tree[chunk_key] = []
        if isinstance( chunk_body, list ):
            for i in chunk_body:
                tree[chunk_key].append( i )
        else:
            tree[chunk_key].append( chunk_body )

    logger.debug( 'finished parsing section of %s (%d bytes)!',
        chunk_header[0], chunk_header[1] )

    # Return the tree.
    return header, tree

