//
// ppdpart.cxx
//
// Copyright (c) 1997-1999 Takashi Kanai; All rights reserved. 
//

#include "StdAfx.h"

#if defined(_DEBUG) && defined(_WINDOWS)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "smd.h"

/***************************************
  Part
****************************************/

Sppt *create_ppdpart(Sppd *ppd)
{
  Sppt *pt;

  pt = (Sppt *) malloc(sizeof(Sppt));
  
  pt->nxt = (Sppt *) NULL;
  if (ppd->sppt == (Sppt *) NULL) {
    pt->prv = (Sppt *) NULL;
    ppd->eppt = ppd->sppt = pt;
  } else {
    pt->prv   = ppd->eppt;
    ppd->eppt = ppd->eppt->nxt = pt;
  }

  // id
  pt->no   = ppd->pid;
  pt->sid = SMDNULL;

  //back solid
  pt->bpso = NULL;
  
  ++( ppd->pn );
  ++( ppd->pid );

  return pt;
}

void free_ppdpart(Sppt *pt, Sppd *ppd)
{
  if (pt == (Sppt *) NULL) return;
  
  if (ppd->sppt == pt) {
    if ((ppd->sppt = pt->nxt) != (Sppt *) NULL)
      pt->nxt->prv = (Sppt *) NULL;
    else {
      ppd->eppt = (Sppt *) NULL;
    }
  } else if (ppd->eppt == pt) {
    pt->prv->nxt = (Sppt *) NULL;
    ppd->eppt = pt->prv;
    
  } else {
    pt->prv->nxt = pt->nxt;
    pt->nxt->prv = pt->prv;
  }
  
  free(pt);
  --( ppd->pn );
}

Sppt *list_ppdpart( Sppd *ppd, int id )
{
  for ( Sppt *pt = ppd->sppt; pt != NULL; pt = pt->nxt ) {
    if ( pt->no == id ) return pt;
  }
  return NULL;
}
