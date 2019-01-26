//
// ppdnormal.cxx
//
// Copyright (c) 1997-1999 Takashi Kanai
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
//

#include "StdAfx.h"

#if defined(_DEBUG) && defined(_WINDOWS)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include <math.h>
#include "smd.h"
#include "ppdvertex.h"
#include "veclib.h"

/***************************************
  Normal
****************************************/

Spnm *create_ppdnormal(Sppd *ppd)
{
  Spnm *nm;

  nm = (Spnm *) malloc(sizeof(Spnm));

  nm->nxt = (Spnm *) NULL;
  if (ppd->spnm == (Spnm *) NULL) {
    nm->prv = (Spnm *) NULL;
    ppd->epnm = ppd->spnm = nm;
  } else {
    nm->prv   = ppd->epnm;
    ppd->epnm = ppd->epnm->nxt = nm;
  }

  /* id */
  nm->no   = ppd->nid;
  nm->vn  = 0;
  /* back solid */
  nm->bpso = (Spso *) NULL;
  /* save id */
  nm->sid  = SMDNULL;
  ++( ppd->nn );
  ++( ppd->nid );

  return nm;
}

void free_ppdnormal(Spnm *nm, Sppd *ppd)
{
  if (nm == (Spnm *) NULL) return;
  
  if (ppd->spnm == nm) {
    if ((ppd->spnm = nm->nxt) != (Spnm *) NULL)
      nm->nxt->prv = (Spnm *) NULL;
    else {
      ppd->epnm = (Spnm *) NULL;
    }
  } else if (ppd->epnm == nm) {
    nm->prv->nxt = (Spnm *) NULL;
    ppd->epnm = nm->prv;
  } else {
    nm->prv->nxt = nm->nxt;
    nm->nxt->prv = nm->prv;
  }

  free(nm);
  --( ppd->nn );
}

void ppdnorm( Sppd *ppd, double angle )
{
  if ( ppd == NULL ) return;
  
  // normal
  Spnm *nm, *nnm;
  for (nm = ppd->spnm; nm != (Spnm *) NULL; nm = nnm) {
    nnm = nm->nxt;
    free_ppdnormal( nm, ppd );
  }
  ppd->spnm = ppd->epnm = (Spnm *) NULL;
  ppd->nn = 0;
  ppd->nid = 0;

  double cosign = cos( angle * SMDPI / 180.0 );
  Spvt *vt;
  Sphe *he;
  Spfc *fc, *ofc;
  for ( vt = ppd->spvt; vt != NULL; vt = vt->nxt ) {
    he = vt->sphe;
    fc = he->bpfc;
    nm = create_ppdnormal( ppd );
    nm->vec.x = fc->nrm.x;
    nm->vec.y = fc->nrm.y;
    nm->vec.z = fc->nrm.z;
    nm->bpso  = ppd->spso;
    nm->vn = 1;
    he->nm = nm;
    ofc = NULL;
    do {
      fc = he->bpfc;
      if ( he != vt->sphe ) {
	if ( V3Cosign( &(ofc->nrm), &(fc->nrm) ) < cosign ) {
	  nm = create_ppdnormal( ppd );
	  nm->vec.x = fc->nrm.x;
	  nm->vec.y = fc->nrm.y;
	  nm->vec.z = fc->nrm.z;
	  nm->bpso  = ppd->spso;
	  nm->vn = 1;
	} else {
	  nm->vec.x = (nm->vec.x * (double) nm->vn + fc->nrm.x) / (nm->vn + 1);
	  nm->vec.y = (nm->vec.y * (double) nm->vn + fc->nrm.y) / (nm->vn + 1);
	  nm->vec.z = (nm->vec.z * (double) nm->vn + fc->nrm.z) / (nm->vn + 1);
	  ++(nm->vn);
	}
	he->nm = nm;
	ofc = he->bpfc;
      } else {
	ofc = fc;
      }
      
      he = ppdvertex_ccwnext_halfedge( he );
    } while ( (he != vt->sphe) && (he != NULL) );
  }

  for ( nm = ppd->spnm; nm != NULL; nm = nm->nxt ) {
    V3Normalize( &(nm->vec) );
  }

}

void freeppdnorm( Sppd *ppd )
{
  if ( ppd == NULL ) return;
  
  for ( Spfc *fc = ppd->spfc; fc != NULL; fc = fc->nxt ) {
    Sphe *he = fc->sphe;
    do {
      he->nm = NULL;
    } while ( (he = he->nxt) != fc->sphe );
  }

  Spnm *nm, *nnm;
  for ( nm = ppd->spnm; nm != (Spnm *) NULL; nm = nnm ) {
    nnm = nm->nxt;
    free_ppdnormal( nm, ppd );
  }
  ppd->spnm = ppd->epnm = NULL;
}
      

