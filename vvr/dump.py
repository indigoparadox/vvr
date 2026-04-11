
from pprint import pprint

from . import vvr

def dump_vvr( **kwargs ):
    with open( kwargs['vvr_path'], 'rb' ) as vvr_file:
        pprint( vvr.parse_file( vvr_file ) )

