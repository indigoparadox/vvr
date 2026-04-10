
import logging
import struct

CONTAINER_CHUNKS = [b'ROOT', b'PRSM']

def parse_chunk( header, chunk ):

    logger = logging.getLogger( 'parse.chunk' )

    while( 1 ):
        header = struct.unpack( '>4sl', header )
        body = vvr_file.read( header[1] )
        logger.debug( 'found chunk: %s (%d bytes)', header[0], header[1] )
        if header[0] in CONTAINER_CHUNKS:
            parse_chunk( header, body )


def parse_file( vvr_file, header=None, sz=0 ):

    logger = logging.getLogger( 'parse.file' )

    chunks_out = []

    if not header:
        # Read form header.
        frm = vvr_file.read( 12 )
        frm = struct.unpack( '>4sl4s', frm )
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
        chunk_header = struct.unpack( '>4sl', chunk_header )
        logger.debug( 'found chunk: %s (%d bytes)',
            chunk_header[0], chunk_header[1] )
        if chunk_header[0] in CONTAINER_CHUNKS:
            # Delve into parent section.
            chunk_header, chunk_body = parse_file(
                vvr_file, chunk_header, chunk_header[1] )
        else:
            # Read and count the raw bytes for unknown section.
            chunk_body = vvr_file.read( chunk_header[1] )
        bytes_read += chunk_header[1]

        # Store whatever we found in the tree we're building.
        chunks_out.append( (chunk_header, chunk_body) )

    logger.debug( 'finished parsing section of %s (%d bytes)!',
        chunk_header[0], chunk_header[1] )

    # Return the tree.
    return header, chunks_out

