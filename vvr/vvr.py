
import logging
import struct

CONTAINER_CHUNKS = [b'ROOT', b'PRSM']

def parse_colr( header : tuple[str, int], body : bytes ):

    logger = logging.getLogger( 'parse.sections.colr' )

    body_out = []
    body_out.append( struct.unpack( '>BBBB', body[0:4] ) )
    body_out.append( struct.unpack( '>BBBB', body[4:8] ) )

    return header, body_out

def parse_poly( header : tuple[str, int], body : bytes ):

    logger = logging.getLogger( 'parse.sections.poly' )

    body_out = []

    print( len( body ) )
    print( type( body ) )
    print( body )
    meta = struct.unpack( '>4BHHHHLLLLL', body[0:32] )

    coord_idx = 0
    while coord_idx < meta[12]:
        coord = struct.unpack( '>hHhH',
            body[32 + (coord_idx * 8):32 + (coord_idx * 8) + 8] )
        logger.debug( 'coord %d of %d: %s', coord_idx, meta[12], str( coord ) )
        body_out.append( coord )
        coord_idx += 1

    return header, body_out

def parse_posn( header, body ):

    logger = logging.getLogger( 'parse.sections.posn' )

    body_out = []

    return header, body_out

KNOWN_CHUNKS = {
    b'COLR': parse_colr,
    b'POLY': parse_poly,
    b'POSN': parse_posn,
}

def parse_file( vvr_file, header=None, sz=0 ):

    logger = logging.getLogger( 'parse.file' )

    chunks_out = []

    if not header:
        # Read form header.
        frm = vvr_file.read( 12 )
        frm = struct.unpack( '>4sL4s', frm )
        assert( b'FORM' == frm[0] )
        assert( b'VMDL' == frm[2] )
        logger.debug( 'found form for file of %d bytes...', frm[1] )
        sz = frm[1]

    bytes_read = 0
    while( bytes_read < sz ):
        chunk_header = vvr_file.read( 8 )
        if b'' == chunk_header:
            logger.debug( 'file finished!' )
            return header, chunks_out
        bytes_read += 8
        chunk_header = struct.unpack( '>4sL', chunk_header )
        logger.debug( 'found chunk: %s (%d bytes)',
            chunk_header[0], chunk_header[1] )
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
        chunks_out.append( (chunk_header, chunk_body) )

    logger.debug( 'finished parsing section of %s (%d bytes)!',
        chunk_header[0], chunk_header[1] )

    # Return the tree.
    return header, chunks_out

