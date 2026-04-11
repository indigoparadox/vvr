
import argparse
import logging

from . import render, dump

def main():
    parser = argparse.ArgumentParser()

    parser.add_argument( '-v', '--verbose', action='store_true' )

    subparsers = parser.add_subparsers()

    parser_dump = subparsers.add_parser( 'dump' )
    parser_dump.add_argument( 'vvr_path' )
    parser_dump.set_defaults( func=dump.dump_vvr )

    parser_render = subparsers.add_parser( 'render' )
    parser_render.add_argument( 'vvr_path' )
    parser_render.set_defaults( func=render.render_vvr )

    args = parser.parse_args()

    level = logging.WARNING
    if args.verbose:
        level = logging.DEBUG
    logging.basicConfig( level=level )
    logger = logging.getLogger( 'main' )

    args_dict = vars( args )
    args.func( **args_dict )

if '__main__' == __name__:
    main()

