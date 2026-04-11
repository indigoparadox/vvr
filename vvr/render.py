
import pygame

from . import vvr

def render_vvr( **kwargs ):

    vvr_parms = None
    with open( kwargs['vvr_path'], 'rb' ) as vvr_file:
        vvr_parms = vvr.parse_file( vvr_file )

    for prsm in vvr_parms[1]['ROOT'][0]['PRSM']:
        for coord in prsm['POLY'][0]['coords']:
            print( coord )

