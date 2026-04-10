
import argparse
import logging

from . import vvr

def main():
    parser = argparse.ArgumentParser()

    parser.add_argument( '-v', '--verbose', action='store_true' )

    parser.add_argument( 'vvr_path' )

    args = parser.parse_args()

    level = logging.WARNING
    if args.verbose:
        level = logging.DEBUG
    logging.basicConfig( level=level )
    logger = logging.getLogger( 'main' )

    with open( args.vvr_path, 'rb' ) as vvr_file:
        print( vvr.parse_file( vvr_file ) )

if '__main__' == __name__:
    main()

