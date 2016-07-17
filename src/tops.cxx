//
// ppd2ps.cxx
// shape preserving mapping functions
// based on Floater's CAGD paper
//
// Copyright (c) 1999-2000 by Takashi Kanai; All rights reserved. 
//

#include "StdAfx.h"

#if defined(_DEBUG) && defined(_WINDOWS)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include <math.h>
#include <assert.h>
#include "smd.h"
#include "screen.h"

#include "ppd.h"
#include "ppdsolid.h"
#include "ppdpart.h"
#include "ppdlist.h"
#include "ppdvertex.h"
#include "ppdface.h"
#include "ppdedge.h"
#include "ppdloop.h"
#include "tile.h"
#include "veclib.h"
#include "matlib.h"
#include "linbcg.h"
#include "intsec.h"
#include "file.h"
#include "mytime.h"
#include "remesh.h"
#include "ps.h"

int main( int argc, char *argv[] )
{
  if ( argc != 3 ) {
    fprintf( stderr, "usage: %s in.ppd out.ps\n", argv[0] );
    exit(1);
  }

  // open ppd
  Sppd *ppd = (Sppd *) NULL;
  if ( (ppd  = open_ppd( argv[1], FALSE )) == NULL ) {
    fprintf( stderr, "Error: can't open %s.\n", argv[1] );
    exit(1);
  }

  FILE *fp = open_ps_ppd( argv[2], ppd );
  // エッジ
  psppdedgeA( fp, ppd );

#if 0
  // 円
  Vec2d center; center.x = center.y = 0.0;
  pscircle( fp, &center, 1.0 );
  // 軸
  pslinewidth( fp, 0.3 );
  Vec2d axl, axr;
  axl.x = -1.2; axl.y = 0.0;
  axr.x =  1.2; axr.y = 0.0;
  psline( fp, &(axl), &(axr) );
  axl.x = 0.0; axl.y = -1.2;
  axr.x = 0.0; axr.y =  1.2;
  psline( fp, &(axl), &(axr) );
#endif  
  
  close_ps( fp );
  
  free_ppd( ppd );

  return TRUE;
}

