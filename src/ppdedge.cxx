//
// ppdedge.cxx
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

#include "smd.h"
#include "ppdvertex.h"
#include "ppdedge.h"
#include "veclib.h"

//
// Edge
//

Sped *create_ppdedge( Sppd *ppd )
{
  Sped *ed;

  ed = (Sped *) malloc( sizeof(Sped) );

  ed->nxt = (Sped *) NULL;
  if (ppd->sped == NULL) {
    ed->prv   = NULL;
    ppd->eped = ppd->sped = ed;
  } else {
    ed->prv   = ppd->eped;
    ppd->eped = ppd->eped->nxt = ed;
  }

  // edge color
  ed->col = EDGEWHITE;
  
  // IDs
  ed->no  = ppd->eid;
  
  // type
  ed->type = EDGE_ORIGINAL;

  // links
  ed->sv  = NULL;
  ed->ev  = NULL;
  ed->lhe = NULL;
  ed->rhe = NULL;

  // loop check
//    ed->used_loop = SMD_OFF;
  
  // for shortest-path
  ed->sp_type  = SP_EDGE_NORMAL;
  // used for shortest path (ONFACE only)
  ed->sp_fc = NULL;
//    ed->num_steiner_point = 0;
//    ed->cal_subgraph = SMD_OFF;
//    ed->org_ed   = NULL;

// ed->group_id = SMDNULL;

  // boundary attr.
  //ed->atr = EDGEINTERNAL;
  ed->isBoundary = FALSE;

  // for checking egde-face links
//    ed->fn = 0;

  // for SparseMatrix (fairing)
//    ed->ewt = 0.0;

  ++( ppd->en );
  ++( ppd->eid );
  
  return ed;
}

void free_ppdedge( Sped *ed, Sppd *ppd )
{
  if (ed == (Sped *) NULL) return;

//   free_ppdedgevertex( ed );

  // kill edge - halfedge links
  if ( ed->lhe != NULL ) {
    ed->lhe->ed = NULL;
    ed->lhe = NULL;
  }
  if ( ed->rhe != NULL ) {
    ed->rhe->ed = NULL;
    ed->rhe = NULL;
  }

  if (ppd->sped == ed) {
    if ((ppd->sped = ed->nxt) != (Sped *) NULL)
      ed->nxt->prv = (Sped *) NULL;
    else {
      ppd->eped = (Sped *) NULL;
    }
  } else if (ppd->eped == ed) {
    ed->prv->nxt = (Sped *) NULL;
    ppd->eped = ed->prv;
  } else {
    ed->prv->nxt = ed->nxt;
    ed->nxt->prv = ed->prv;
  }

  free(ed);
  --( ppd->en );
}

void free_ppdedge_withqtree(Sped *ed, Sppd *ppd)
{
/*   if (ed->qe != (QEdge *) NULL) { */
/*     qe = ed->qe; */
/*     qt = qe->qt; */
/*     delete_qtree_edge(qe, qt); */
/*   } */
  free_ppdedge(ed, ppd);
}

//
// for tface_subdivide_ppdface
//
void free_isolated_ppdedge( Sped *ed, Sppd *ppd )
{
  if ( ed == NULL ) return;
  
  ed->lhe = NULL;
  ed->rhe = NULL;
  FreeVted( ed->sv, ed );
  FreeVted( ed->ev, ed );
  free_ppdedge( ed, ppd );
}

// number
int num_ppdedge( Sppd *ppd )
{
  int i = 0;
  Sped *ed;
  for ( ed = ppd->sped; ed != NULL; ed = ed->nxt ) {
    if ( ed->type == EDGE_ORIGINAL ) ++i;
  }
  return i;
}

// /* vertex -> edge */
// Edvt *create_edvtx(Sped *edge, Spvt *vt)
// {
//   Edvt *ev;

//   if (vt == (Spvt *) NULL) return (Edvt *) NULL;
  
//   ev = (Edvt *) malloc(sizeof(Edvt));

//   ev->nxt = (Edvt *) NULL;
//   if (edge->sedvt == (Edvt *) NULL) {
//     ev->prv       = (Edvt *) NULL;
//     edge->eedvt = edge->sedvt = ev;
//   } else {
//     ev->prv       = edge->eedvt;
//     edge->eedvt = edge->eedvt->nxt = ev;
//   }

//   /* edge */
//   ev->ed = edge;
//   /* vertex */
//   ev->vt = vt;
//   ev->val = 0.0;
//   ev->int_ed = (Sped *) NULL;
  
//   ++(edge->evn);
  
//   return ev;
// }

// void insert_edvtx( Sped *ed, Spvt *vt, Sped *int_edge, double val )
// {
//   Edvt   *ev, *nev;

//   if ( ed == (Sped *) NULL ) return;
  
//   /* first */
//   if (ed->sedvt == (Edvt *) NULL) {
//     ev = create_edvtx(ed, vt);
//     ev->val = val;
//     ev->int_ed = int_edge;
//     return;
//   }
  
//   /* calc angle */
//   ev    = ed->sedvt;
//   while ((ev != (Edvt *) NULL) && (val > ev->val)) {
//     ev = ev->nxt;
//   }

//   /* last */
//   if (ev == (Edvt *) NULL) {
//     ev = create_edvtx(ed, vt);
//     ev->val = val;
//     ev->int_ed = int_edge;
//     return;
//   } 

//   nev = (Edvt *) malloc(sizeof(Edvt));

//   nev->prv = ev->prv;
//   nev->nxt = ev;
//   if (ev->prv != (Edvt *) NULL) 
//     ev->prv->nxt = nev;
//   ev->prv = nev;
//   if (ed->sedvt == ev) ed->sedvt = nev;

//   nev->ed = ed;
//   nev->vt = vt;
//   nev->val = val;
//   nev->int_ed = int_edge;

//   ++( ed->evn );
  
//   return;
// }

// void free_ppdedgevertex(Sped *ed)
// {
//   Edvt *ev, *evnxt;

//   for (ev = ed->sedvt; ev != (Edvt *) NULL; ev = evnxt) {
//     evnxt = ev->nxt;
//     free_edvtx(ev, ed);
    
//   }
//   ed->evn = 0;
//   ed->sedvt = ed->eedvt = (Edvt *) NULL;
// }

// void free_edvtx(Edvt *ev, Sped *ed)
// {
//   if (ev == (Edvt *) NULL) return;

//   ev->ed = (Sped *) NULL;
//   ev->vt = (Spvt *) NULL;
  
//   if (ed->sedvt == ev) {
//     if ((ed->sedvt = ev->nxt) != (Edvt *) NULL)
//       ev->nxt->prv = (Edvt *) NULL;
//     else {
//       ed->eedvt = (Edvt *) NULL;
//     }
//   } else if (ed->eedvt == ev) {
//     ev->prv->nxt = (Edvt *) NULL;
//     ed->eedvt = ev->prv;
//   } else {
//     ev->prv->nxt = ev->nxt;
//     ev->nxt->prv = ev->prv;
//   }
//   free(ev);
//   --( ed->evn );
// }

void edge_length( Sppd *ppd )
{
  int  i;
  Sped *ed;
  
  for ( i = 0, ed = ppd->sped; ed != (Sped *) NULL; ed = ed->nxt, ++i ) {

    ed->length = V3DistanceBetween2Points( &(ed->sv->vec), &(ed->ev->vec) );
    if ( i ) {
      
      if ( ed->length > ppd->max_edge_length ) {
	ppd->max_edge_length = ed->length;
      }
      
    } else {
      ppd->max_edge_length = ed->length;
    }
    
  }
}

Sped *find_ppdedge(Spvt *sv, Spvt *ev)
{
  Vted *ve;
  
  ve = sv->svted;
  while (ve != (Vted *) NULL) {
    if ( ( (ve->ed->sv == sv) && (ve->ed->ev == ev) ) ||
	 ( (ve->ed->sv == ev) && (ve->ed->ev == sv) ) ) {
      return ve->ed;
    }
    ve = ve->nxt;
  }
  return (Sped *) NULL;
}

Sped *find_ppdedge_new( Spvt *sv, Spvt *ev )
{
  Sped *ed, *fed;
  //display("sv %d ven %d\n", sv->no, sv->ven );
  ed = fed = ppdvertex_first_edge( sv );
  do {
    //display("ed %d\n", ed->no );
    if ( ( (ed->sv == sv) && (ed->ev == ev) ) ||
	 ( (ed->sv == ev) && (ed->ev == sv) ) ) 
      return ed;
    
    ed = ppdvertex_next_edge( ed, sv );
  } while ( (ed != NULL) && (ed != fed) );

  return NULL;
}

double normalize_ppdedge( Sppd *ppd )
{
  int  i;
  Spvt *v;
  double maxlength, scale;
  double xlength, ylength, zlength;
  double maxx, minx, maxy, miny, maxz, minz;
  
  maxx=minx=maxy=miny=maxz=minz=0.0;
  for (i = 0, v = ppd->spvt; v != (Spvt *) NULL; ++i, v = v->nxt) {
    if (i) {
      if (v->vec.x > maxx) maxx = v->vec.x;
      if (v->vec.x < minx) minx = v->vec.x;
      if (v->vec.y > maxy) maxy = v->vec.y;
      if (v->vec.y < miny) miny = v->vec.y;
      if (v->vec.z > maxz) maxz = v->vec.z;
      if (v->vec.z < minz) minz = v->vec.z;
    }	else {
      maxx = minx = v->vec.x;
      maxy = miny = v->vec.y;
      maxz = minz = v->vec.z;
    }
  }

  xlength = maxx - minx;
  ylength = maxy - miny;
  zlength = maxz - minz;

  if (xlength > ylength)   maxlength = xlength;
  else                     maxlength = ylength;
  if (zlength > maxlength) maxlength = zlength;

  scale = 1.0 / maxlength;

  return scale;
}

void ppdedge_to_vec( Sped *ed, Vec *vec )
{
  V3Sub( &(ed->ev->vec), &(ed->sv->vec), vec );
  //V3Normalize( vec );
}

Spfc *ppdedge_another_fc( Sped *ed, Spfc *fc )
{
  if ( ed->lhe != NULL ) {
    if ( ed->lhe->bpfc != fc ) return ed->lhe->bpfc;
  }
  if ( ed->rhe != NULL ) {
    if ( ed->rhe->bpfc != fc ) return ed->rhe->bpfc;
  }
  return NULL;
}

  
