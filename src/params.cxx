//
// params.cxx
//
// Copyright (c) 1999 Takashi Kanai; All rights reserved. 
//

#include "StdAfx.h"

#if defined(_DEBUG) && defined(_WINDOWS)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "smd.h"
#include "params.h"

void params_init( Params *params )
{
  params->nfile = 0;

  params->esp = FALSE;

  params->src_id = SMDNULL;
  params->dist_id = SMDNULL;

  params->save_path = FALSE;

  params->print = FALSE;
  
  // penalties
  params->gamma = 1.0;
  params->midp = 1;
}
  
