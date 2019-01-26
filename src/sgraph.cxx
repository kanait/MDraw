//
// sgraph.cxx
// Shortest path functions (SGraph version) 
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

//#include <unistd.h>

#include <math.h>
#include "smd.h"
#include "screen.h"

#include "ppdedge.h"
#include "sgraph.h"
#include "veclib.h"

/* SGraph functions */

SGraph *create_sgraph( void )
{
  SGraph *sg;
	
  sg = (SGraph *) malloc( sizeof(SGraph) );
	
  sg->sgvtn  = 0;
  sg->sgvtid = 0;
  sg->sgvt  = (SGvt *) NULL;
  sg->egvt  = (SGvt *) NULL;
  sg->sgedn  = 0;
  sg->sgedid = 0;
  sg->sged  = (SGed *) NULL;
  sg->eged  = (SGed *) NULL;
  sg->sgfcn  = 0;
  sg->sgfcid = 0;
  sg->sgfc  = (SGfc *) NULL;
  sg->egfc  = (SGfc *) NULL;

  sg->avrsp = 0.0;
  
  sg->src = sg->dist = (SGvt *) NULL;
  sg->lp  = (SGlp *) NULL;
  sg->sub = 1.0;

  sg->ppd = NULL;
	
  return sg;
}

void free_sgraph( SGraph *sg )
{
  if ( sg == NULL ) return;
	
  sg->src  = NULL;
  sg->dist = NULL;
  if ( sg->lp != (SGlp *) NULL ) {
    free_sglp( sg->lp );
    sg->lp = NULL;
  }
	
  FreeSGEdge( sg );
  FreeSGFace( sg );
  FreeSGVertex( sg );
	
  free( sg );
}

/* SGvt functions */
SGvt *create_sgvt( SGraph *sg )
{
  SGvt *vt;
	
  vt = (SGvt *) malloc(sizeof(SGvt));
	
  vt->nxt = (SGvt *) NULL;
  if (sg->sgvt == (SGvt *) NULL) {
    vt->prv   = (SGvt *) NULL;
    sg->egvt = sg->sgvt = vt;
  } else {
    vt->prv   = sg->egvt;
    sg->egvt = sg->egvt->nxt = vt;
  }
	
  // id
  vt->no   = sg->sgvtid;
  vt->sid  = SMDNULL;

  // type
  vt->type = VERTEX_ORIGINAL;
  vt->path = VERTEX_NONE;
	
  // vt - edge
  vt->sgvtedn = 0;
  vt->svted = (SGvted *) NULL;
  vt->evted = (SGvted *) NULL;

  // link to ppd elements
  vt->spvt = (Spvt *) NULL;
  vt->sped = (Sped *) NULL;
	
  // for shortest path
  vt->pq_type = EMPTY;
  vt->pqc     = (SGPQCont *) NULL;
	
  ++( sg->sgvtn );
  ++( sg->sgvtid );
	
  return vt;
}

SGvt *CreateSGvtfromSGvt( SGraph *sg, SGvt *sgvt )
{
  SGvt *nsgvt;
	
  nsgvt = create_sgvt( sg );
  nsgvt->vec.x = sgvt->vec.x;
  nsgvt->vec.y = sgvt->vec.y;
  nsgvt->vec.z = sgvt->vec.z;
    
  nsgvt->type = sgvt->type;
  if ( nsgvt->type == VERTEX_ORIGINAL ) {
    nsgvt->spvt = sgvt->spvt;
  } else {
    nsgvt->sped  = sgvt->sped;
  }
	
  return nsgvt;
}

void free_sgvt( SGvt *vt, SGraph *sg )
{
  void FreeSGVertexEdge( SGvt * );
	
  if (vt == (SGvt *) NULL) return;
	
  FreeSGVertexEdge( vt );
	
  if ( sg->sgvt == vt ) {
    if ( (sg->sgvt = vt->nxt) != (SGvt *) NULL )
      vt->nxt->prv = (SGvt *) NULL;
    else {
      sg->egvt = (SGvt *) NULL;
    }
  } else if (sg->egvt == vt) {
    vt->prv->nxt = (SGvt *) NULL;
    sg->egvt = vt->prv;
  } else {
    vt->prv->nxt = vt->nxt;
    vt->nxt->prv = vt->prv;
  }
	
  --( sg->sgvtn );
  free( vt );
}


SGvt *another_sgvt( SGed *ed, SGvt *vt )
{
  if ( ed->sv == vt ) return ed->ev;
  else return ed->sv;
}

SGvt *find_sgvt_from_spvt( SGraph *sg, Spvt *ppdvt )
{
  SGvt *vt;
	
  for ( vt = sg->sgvt; vt != (SGvt *) NULL; vt = vt->nxt ) {
    if ( vt->type == VERTEX_ORIGINAL ) {
      if ( vt->spvt == ppdvt ) return vt;
    }
  }
  return (SGvt *) NULL;
}

SGvt *find_sgvt_from_spvt_number( SGraph *sg, int id )
{
  SGvt *vt;
	
  for ( vt = sg->sgvt; vt != (SGvt *) NULL; vt = vt->nxt ) {
    if ( vt->type == VERTEX_ORIGINAL ) {
      if ( vt->spvt->no == id ) return vt;
    }
  }
  return (SGvt *) NULL;
}

SGvt *find_sgvt_from_sgfc( SGfc *fc, SGvt *vt )
{
  SGfcvt *fv;
	
  for ( fv = fc->sfcvt; fv != (SGfcvt *) NULL; fv = fv->nxt ) {
    if ( fv->vt == vt ) return vt;
  }
  return NULL;
}

void FreeSGVertex( SGraph *sg )
{
  SGvt *vt, *nvt;
	
  if ( sg == NULL ) return;
	
  for ( vt = sg->sgvt; vt != (SGvt *) NULL; vt = nvt ) {
    nvt = vt->nxt;
    free_sgvt( vt, sg );
  }
  sg->sgvtn = sg->sgvtid = 0;
  sg->sgvt = sg->egvt = (SGvt *) NULL;
}

/* SGvted functions */
SGvted *create_sgvted( SGvt *vt, SGed *ed )
{
  SGvted *ve;
	
  ve = (SGvted *) malloc( sizeof(SGvted) );
	
  ve->nxt = (SGvted *) NULL;
  if (vt->svted == (SGvted *) NULL) {
    ve->prv   = (SGvted *) NULL;
    vt->evted = vt->svted = ve;
  } else {
    ve->prv   = vt->evted;
    vt->evted = vt->evted->nxt = ve;
  }
	
  ve->ed = ed;
	
  ++( vt->sgvtedn );
	
  return ve;
}

void free_sgvted( SGvted *ve, SGvt *vt )
{
  if ( vt == (SGvt *) NULL ) return;
  if ( ve == (SGvted *) NULL ) return;
	
  if ( vt->svted == ve ) {
    if ( (vt->svted = ve->nxt) != (SGvted *) NULL )
      ve->nxt->prv = (SGvted *) NULL;
    else {
      vt->evted = (SGvted *) NULL;
    }
  } else if (vt->evted == ve) {
    ve->prv->nxt = (SGvted *) NULL;
    vt->evted = ve->prv;
  } else {
    ve->prv->nxt = ve->nxt;
    ve->nxt->prv = ve->prv;
  }
	
  --( vt->sgvtedn );
  free( ve );
}

void FreeSGvted( SGvt *vt, SGed *ed )
{
  SGvted *ve;
  void free_sgvted( SGvted *, SGvt * );
	
  if ( vt == NULL ) return;
	
  for ( ve = vt->svted; ve != (SGvted *) NULL; ve = ve->nxt ) {
    if ( ve->ed == ed ) {
      free_sgvted( ve, vt );
      return;
    }
  }
}

void FreeSGVertexEdge( SGvt *vt )
{
  SGvted *ve, *nve;
	
  if ( vt == NULL ) return;
	
  for ( ve = vt->svted; ve != (SGvted *) NULL; ve = nve ) {
    nve = ve->nxt;
    free_sgvted( ve, vt );
  }
  vt->sgvtedn = 0;
  vt->svted  = vt->evted = (SGvted *) NULL;
}

/* SGed functions */
SGed *create_sged( SGraph *sg )
{
  SGed *ed;
	
  ed = (SGed *) malloc(sizeof(SGed));
	
  ed->nxt = (SGed *) NULL;
  if (sg->sged == (SGed *) NULL) {
    ed->prv   = (SGed *) NULL;
    sg->eged = sg->sged = ed;
  } else {
    ed->prv   = sg->eged;
    sg->eged = sg->eged->nxt = ed;
  }
	
  /* id */
  ed->no   = sg->sgedid;
  ed->sid  = SMDNULL;
  /* type */
  ed->type = EDGE_ORIGINAL;
  ed->isPathEdge = FALSE;
	
  ed->sv = ed->ev = NULL;
  ed->lf = ed->rf = NULL;
	
  ed->sped = NULL;
  ed->spfc = NULL;
	
  ++( sg->sgedn );
  ++( sg->sgedid );
	
  return ed;
}

SGed *CreateSGed( SGraph *sg, SGvt *sv, SGvt *ev, Sped *ed )
{
  SGed   *sged;
	
  sged = create_sged( sg );
  sged->sped = ed;
	
  sged->sv = sv;
  sged->ev = ev;
  (void) create_sgvted( sv, sged );
  (void) create_sgvted( ev, sged );
	
  return sged;
}

SGed *CreateSGedfromSGed( SGraph *sg, SGvt *sv, SGvt *ev, SGed *sged )
{
  SGed *nsged;
	
  nsged = CreateSGed( sg, sv, ev, sged->sped );
  nsged->type = sged->type;
  if ( nsged->type == EDGE_ONFACE ) nsged->spfc = sged->spfc;
	
  return nsged;
}

void free_sged( SGed *ed, SGraph *sg )
{
  if ( ed == (SGed *) NULL ) return;
	
  if ( sg->sged == ed ) {
    if ( (sg->sged = ed->nxt) != (SGed *) NULL )
      ed->nxt->prv = (SGed *) NULL;
    else {
      sg->eged = (SGed *) NULL;
    }
  } else if (sg->eged == ed) {
    ed->prv->nxt = (SGed *) NULL;
    sg->eged = ed->prv;
  } else {
    ed->prv->nxt = ed->nxt;
    ed->nxt->prv = ed->prv;
  }
	
  --( sg->sgedn );
  free( ed );
}

void FreeSGed( SGed *ed, SGraph *sg )
{
  if ( ed == NULL ) return;
  FreeSGvted( ed->sv, ed );
  FreeSGvted( ed->ev, ed );
  free_sged( ed, sg );
}

void FreeSGEdge( SGraph *sg )
{
  SGed *ed, *ned;
	
  if ( sg == NULL ) return;
	
  for ( ed = sg->sged; ed != (SGed *) NULL; ed = ned ) {
    ned = ed->nxt;
    free_sged( ed, sg );
  }
  sg->sgedn = sg->sgedid = 0;
  sg->sged = sg->eged = (SGed *) NULL;
}

SGed *find_sged( SGvt *sv, SGvt *ev )
{
  SGvted *ve;
  SGed   *ed;
	
  for ( ve = sv->svted; ve != (SGvted *) NULL; ve = ve->nxt ) {
    ed = ve->ed;
    //    display("\ted %d sv %d ev %d\n", ed->no, sv->no, ev->no );
    if ( (ed->sv == sv) && (ed->ev == ev) ) {
      return ed;
    }
    if ( (ed->ev == sv) && (ed->sv == ev) ) {
      return ed;
    }
  }
	
  return (SGed *) NULL;
}


/* SGfc functions */
SGfc *create_sgfc( SGraph *sg )
{
  SGfc *fc;
	
  fc = (SGfc *) malloc(sizeof(SGfc));
	
  fc->nxt = (SGfc *) NULL;
  if (sg->sgfc == (SGfc *) NULL) {
    fc->prv   = (SGfc *) NULL;
    sg->egfc = sg->sgfc = fc;
  } else {
    fc->prv   = sg->egfc;
    sg->egfc = sg->egfc->nxt = fc;
  }
	
  /* id */
  fc->no   = sg->sgfcid; 
  fc->sid  = SMDNULL;
	
  /* fc - edge */
  fc->sgfcvtn = 0;
  fc->sfcvt = (SGfcvt *) NULL;
  fc->efcvt = (SGfcvt *) NULL;
	
  fc->spfc = (Spfc *) NULL;
	
  ++( sg->sgfcn );
  ++( sg->sgfcid );
	
  return fc;
}

void free_sgfc( SGfc *fc, SGraph *sg )
{
  if (fc == (SGfc *) NULL) return;
	
  FreeSGFaceVertex( fc );
	
  if ( sg->sgfc == fc ) {
    if ( (sg->sgfc = fc->nxt) != (SGfc *) NULL )
      fc->nxt->prv = (SGfc *) NULL;
    else {
      sg->egfc = (SGfc *) NULL;
    }
  } else if (sg->egfc == fc) {
    fc->prv->nxt = (SGfc *) NULL;
    sg->egfc = fc->prv;
  } else {
    fc->prv->nxt = fc->nxt;
    fc->nxt->prv = fc->prv;
  }
	
  --( sg->sgfcn );
  free( fc );
}

void FreeSGFace( SGraph *sg )
{
  SGfc *fc, *nfc;
	
  if ( sg == NULL ) return;
	
  for ( fc = sg->sgfc; fc != (SGfc *) NULL; fc = nfc ) {
    nfc = fc->nxt;
    free_sgfc( fc, sg );
  }
  sg->sgfcn = sg->sgfcid = 0;
  sg->sgfc  = sg->egfc = (SGfc *) NULL;
}

/* SGvted functions */
SGfcvt *create_sgfcvt( SGfc *fc, SGvt *vt )
{
  SGfcvt *fv;
	
  fv = (SGfcvt *) malloc( sizeof(SGfcvt) );
	
  fv->nxt = (SGfcvt *) NULL;
  if (fc->sfcvt == (SGfcvt *) NULL) {
    fv->prv   = (SGfcvt *) NULL;
    fc->efcvt = fc->sfcvt = fv;
  } else {
    fv->prv   = fc->efcvt;
    fc->efcvt = fc->efcvt->nxt = fv;
  }
	
  fv->vt = vt;
	
  ++( fc->sgfcvtn );
	
  return fv;
}

void free_sgfcvt( SGfcvt *fv, SGfc *fc )
{
  if ( fc == NULL ) return;
  if ( fv == (SGfcvt *) NULL ) return;
	
  if ( fc->sfcvt == fv ) {
    if ( (fc->sfcvt = fv->nxt) != (SGfcvt *) NULL )
      fv->nxt->prv = (SGfcvt *) NULL;
    else {
      fc->efcvt = (SGfcvt *) NULL;
    }
  } else if ( fc->efcvt == fv ) {
    fv->prv->nxt = (SGfcvt *) NULL;
    fc->efcvt = fv->prv;
  } else {
    fv->prv->nxt = fv->nxt;
    fv->nxt->prv = fv->prv;
  }
	
  --( fc->sgfcvtn );
  free( fv );
}

void FreeSGFaceVertex( SGfc *fc )
{
  SGfcvt *fv, *nfv;
	
  if ( fc == NULL ) return;
	
  for ( fv = fc->sfcvt; fv != (SGfcvt *) NULL; fv = nfv ) {
    nfv = fv->nxt;
    free_sgfcvt( fv, fc );
  }
  fc->sgfcvtn = 0;
  fc->sfcvt  = fc->efcvt = (SGfcvt *) NULL;
}

/* SGvt functions */
SGlp *create_sglp( void )
{
  SGlp *lp;
	
  lp = (SGlp *) malloc( sizeof(SGlp) );
	
  // lp - vt
  lp->sglpvtn = 0;
  lp->slpvt = (SGlpvt *) NULL;
  lp->elpvt = (SGlpvt *) NULL;
	
  // lp - ed
  lp->sglpedn = 0;
  lp->slped = (SGlped *) NULL;
  lp->elped = (SGlped *) NULL;

  // length
  lp->length = 0.0;
	
  // time
  lp->realtime = 0.0;
  lp->usertime = 0.0;
  lp->systime = 0.0;
  
  return lp;
}

void free_sglp( SGlp *lp )
{
  if ( lp == (SGlp *) NULL ) return;
	
  FreeSGLoopEdge( lp );
  FreeSGLoopVertex( lp );
	
  free( lp );
}

/* SGlpvt functions */
SGlpvt *create_sglpvt( SGlp *lp, SGvt *vt )
{
  SGlpvt *lv;
	
  lv = (SGlpvt *) malloc( sizeof(SGlpvt) );
	
  lv->nxt = (SGlpvt *) NULL;
  if (lp->slpvt == (SGlpvt *) NULL) {
    lv->prv   = (SGlpvt *) NULL;
    lp->elpvt = lp->slpvt = lv;
  } else {
    lv->prv   = lp->elpvt;
    lp->elpvt = lp->elpvt->nxt = lv;
  }
	
  lv->vt = vt;
	
  ++( lp->sglpvtn );
	
  return lv;
}

void free_sglpvt( SGlpvt *lv, SGlp *lp )
{
  if (lv == (SGlpvt *) NULL) return;
	
  if ( lp->slpvt == lv ) {
    if ( (lp->slpvt = lv->nxt) != (SGlpvt *) NULL )
      lv->nxt->prv = (SGlpvt *) NULL;
    else {
      lp->elpvt = (SGlpvt *) NULL;
    }
  } else if (lp->elpvt == lv) {
    lv->prv->nxt = (SGlpvt *) NULL;
    lp->elpvt = lv->prv;
  } else {
    lv->prv->nxt = lv->nxt;
    lv->nxt->prv = lv->prv;
  }
	
  --( lp->sglpvtn );
  free( lv );
}

void FreeSGLoopVertex( SGlp *lp )
{
  SGlpvt *lv, *nlv;
	
  if ( lp == NULL ) return;
	
  for ( lv = lp->slpvt; lv != (SGlpvt *) NULL; lv = nlv ) {
    nlv = lv->nxt;
    free_sglpvt( lv, lp );
  }
  lp->sglpvtn = 0;
  lp->slpvt  = lp->elpvt = (SGlpvt *) NULL;
}

// find
SGlpvt *find_sglv( SGlp *lp, SGvt *vt )
{
  for ( SGlpvt *lv = lp->slpvt; lv != NULL; lv = lv->nxt ) {
    if ( lv->vt == vt ) return lv;
  }
  return NULL;
}
		  
/* SGlped functions */
SGlped *create_sglped( SGlp *lp, SGed *ed )
{
  SGlped *le;
	
  le = (SGlped *) malloc( sizeof(SGlped) );
	
  le->nxt = (SGlped *) NULL;
  if (lp->slped == (SGlped *) NULL) {
    le->prv   = (SGlped *) NULL;
    lp->elped = lp->slped = le;
  } else {
    le->prv   = lp->elped;
    lp->elped = lp->elped->nxt = le;
  }
	
  le->ed = ed;
	
  ++( lp->sglpedn );
	
  return le;
}

void free_sglped( SGlped *le, SGlp *lp )
{
  if (le == (SGlped *) NULL) return;
	
  if ( lp->slped == le ) {
    if ( (lp->slped = le->nxt) != (SGlped *) NULL )
      le->nxt->prv = (SGlped *) NULL;
    else {
      lp->elped = (SGlped *) NULL;
    }
  } else if (lp->elped == le) {
    le->prv->nxt = (SGlped *) NULL;
    lp->elped = le->prv;
  } else {
    le->prv->nxt = le->nxt;
    le->nxt->prv = le->prv;
  }
	
  --( lp->sglpedn );
  free( le );
}

void FreeSGLoopEdge( SGlp *lp )
{
  SGlped *le, *nle;
	
  if ( lp == (SGlp *) NULL ) return;
	
  for ( le = lp->slped; le != (SGlped *) NULL; le = nle ) {
    nle = le->nxt;
    free_sglped( le, lp );
  }
  lp->sglpedn = 0;
  lp->slped  = lp->elped = (SGlped *) NULL;
}

// 表示のための SGvt から法線を計算するルーチン

void SGvtnrm( SGvt *vt, Vec *vnrm )
{
  int  i;
  Spvt *spvt;
  Sped *sped;
  Vec  *nrm;
  
  if ( vt->type == VERTEX_ORIGINAL ) {
    
    // その頂点のあるハーフエッジから法線をとってくる
    spvt = vt->spvt;
    nrm = &(spvt->sphe->nm->vec);
    vnrm->x = nrm->x;
    vnrm->y = nrm->y;
    vnrm->z = nrm->z;
    
  } else { // vt->type == VERTEX_ADDED

    // 頂点の下にあるエッジの両端の面の法線の平均

    sped = vt->sped;
    vnrm->x = vnrm->y = vnrm->z = 0.0;

    i = 0;
    if ( sped->lhe != NULL ) {
      vnrm->x += sped->lhe->bpfc->nrm.x;
      vnrm->y += sped->lhe->bpfc->nrm.y;
      vnrm->z += sped->lhe->bpfc->nrm.z;
      ++i;
    }
    if ( sped->rhe != NULL ) {
      vnrm->x += sped->rhe->bpfc->nrm.x;
      vnrm->y += sped->rhe->bpfc->nrm.y;
      vnrm->z += sped->rhe->bpfc->nrm.z;
      ++i;
    }
    vnrm->x /= (double) i;
    vnrm->y /= (double) i;
    vnrm->z /= (double) i;
  }
    
}

