//
// ppdvertex.cxx
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

/***************************************
  Vertex
****************************************/

// create vertex
Spvt *create_ppdvertex( Sppd *ppd )
{
  Spvt *vt;

  vt = (Spvt *) malloc(sizeof(Spvt));

  vt->nxt = (Spvt *) NULL;
  if (ppd->spvt == (Spvt *) NULL) {
    vt->prv   = (Spvt *) NULL;
    ppd->epvt = ppd->spvt = vt;
  } else {
    vt->prv   = ppd->epvt;
    ppd->epvt = ppd->epvt->nxt = vt;
  }

  // 2D vector initialize (for mapping)
  vt->uvw.x = 0.0;
  vt->uvw.y = 0.0;
  
  // id 
  vt->no   = ppd->vid;

  // vertex color
  vt->col  = PNTGREEN;

	// save id
  vt->sid  = SMDNULL;

	// type (new)
  vt->type = VERTEX_ORIGINAL; 

  // back solid
  vt->bpso = (Spso *) NULL;

  // vt - halfedge
  vt->sphe = (Sphe *) NULL;
	
  // vt - edge
  vt->ven  = 0;
  vt->svted = (Vted *) NULL;
  vt->evted = (Vted *) NULL;
  
  // vt - normal
  vt->vnn  = 0;
  vt->svtnm = (Vtnm *) NULL;
  vt->evtnm = (Vtnm *) NULL;

  // vt - halfedge
  vt->sphe = (Sphe *) NULL;
  
  // for shortest path
  vt->sp_type = SP_VERTEX_NORMAL;
  vt->sp_ed   = NULL;
  vt->sp_val  = 0.0;
  vt->spn     = 0;
  //vt->lp      = NULL;
  
  // for creating hppdlod
//    vt->atr   = VTXINTERNAL;
  vt->isBoundary = FALSE;

  // for tile_grouping
  vt->tile_id = SMDNULL;
  
  // for ESPTree
  vt->tree = (ESPTree *) NULL;
  
  // for ESPVnode
  vt->vnode = (ESPVnode *) NULL;

  // for spm
  vt->lambda = NULL;
  vt->vec2d = NULL;

  // for remesh
  vt->tvec_id = SMDNULL;

  // for priority queue (used in spedprique.cxx)
  vt->pqc = NULL;
  vt->pq_type = EMPTY;
  vt->length = 0.0;

  ++( ppd->vn );
  ++( ppd->vid );

  return vt;
}

Spvt *list_ppdvertex( Sppd *ppd, Id id )
{
  Spvt *vt;
  
  for ( vt = ppd->spvt; vt != (Spvt *) NULL; vt = vt->nxt ) {
    if ( vt->no == id ) return vt;
  }
  return (Spvt *) NULL;
}

// number
int num_ppdvertex( Sppd *ppd )
{
  int i = 0;
  Spvt *vt;
  for ( vt = ppd->spvt; vt != NULL; vt = vt->nxt ) {
    if ( vt->type == VERTEX_ORIGINAL ) ++i;
  }
  return i;
}
    
void free_ppdvertex( Spvt *vt, Sppd *ppd )
{
  if (vt == (Spvt *) NULL) return;
  
  if (ppd->spvt == vt) {
    if ((ppd->spvt = vt->nxt) != (Spvt *) NULL)
      vt->nxt->prv = (Spvt *) NULL;
    else {
      ppd->epvt = (Spvt *) NULL;
    }
  } else if (ppd->epvt == vt) {
    vt->prv->nxt = (Spvt *) NULL;
    ppd->epvt = vt->prv;
  } else {
    vt->prv->nxt = vt->nxt;
    vt->nxt->prv = vt->prv;
  }

  free_ppdvertexedge( vt );

  // for spm
  if ( vt->lambda != NULL ) free( vt->lambda );
  if ( vt->vec2d != NULL) free( vt->vec2d );
  
  //display("deleted vt %d\n", vt->no );
  free( vt );
  --( ppd->vn );
}

//
// neighboring edges of a vertex
//

Vted *create_vtxed(Spvt *vertex, Sped *ed)
{
  Vted *ve;

  if (ed == (Sped *) NULL) return (Vted *) NULL;
  
  ve = (Vted *) malloc(sizeof(Vted));

  ve->nxt = (Vted *) NULL;
  if (vertex->svted == (Vted *) NULL) {
    ve->prv       = (Vted *) NULL;
    vertex->evted = vertex->svted = ve;
  } else {
    ve->prv       = vertex->evted;
    vertex->evted = vertex->evted->nxt = ve;
  }

  /* edge */
  ve->ed = ed;
  /* vertex */
  ve->vt    = vertex;
  ve->angle = 0.0;

/*   if (ed->ve1 == (Vted *) NULL) { */
/*     ed->ve1 = ve; */
/*   } else { */
/*     ed->ve2 = ve; */
/*   } */
/*   ve->rotation = SMD_OFF; */
  
  ++(vertex->ven);
  
  return ve;
}

Spvt *another_vt(Sped *ed, Spvt *vt)
{
  if (ed->sv == vt) return ed->ev;
  else return ed->sv;
}

/* Vted *another_vted(Sped *ed, Vted *ve) */
/* { */
/*   if ( (ed->ve1 == (Vted *) NULL) || (ed->ve2 == (Vted *) NULL) ) */
/*     display("okashii!!\n"); */
       
/*   if (ed->ve1 == ve) return ed->ve2; */
/*   else ed->ve1; */
/* } */

Vted *find_vtxed( Spvt *vt, Sped *ed )
{
  Vted *vted;
  
  for ( vted = vt->svted; vted != (Vted *) NULL; vted = vted->nxt ) {
    if ( vted->ed == ed ) return vted;
  }
  return (Vted *) NULL;
}

/* for ppdharmonic */
double calc_angle(Spvt *v1, Spvt *v2, Spvt *v3)
{
  double cs, cs_angle;
  int    i;
  Vec2d  sub1, sub2;
  Vec2d *V2Sub(Vec2d *, Vec2d *, Vec2d *);
  double V2Cosign(Vec2d *, Vec2d *);
  int    V2Signi(Vec2d *, Vec2d *);
  
  V2Sub( &(v2->uvw), &(v1->uvw), &(sub1) );
  V2Sub( &(v3->uvw), &(v1->uvw), &(sub2) );
  
  cs = V2Cosign(&(sub1), &(sub2));

  if ( fabs(cs + 1.0) < SMDZEROEPS ) {
    return 180.0;
  }
  
  /* + か - かの判定のみ */
  i = V2Signi(&(sub1), &(sub2));
/*   sn = V2Sign(&(sub1), &(sub2)); */

  cs_angle = RTOD * acos(cs);
/*   display("cs %g (cs_angle) %g sign %d\n", cs, cs_angle, i); */
  
  if (fabs(cs) < SMDZEROEPS) {
    if ( i == SMD_ON ) {    /* plus */
      return 90.0;
    } else { /* minus */
      return 270.0;
    }
  }
  
/*   if (fabs(sn) < SMDZEROEPS) { */
/*     if (cs > 0.0) */
/*       return 0.0; */
/*     if (cs < 0.0) */
/*       return 180.0; */
/*   } */
  
  if (!i) {
    cs_angle = ( 360.0 - cs_angle );
  }
  
  return cs_angle;
}

Vted *next_vted(Vted *ve)
{
  Vted *nve;
  
  nve = ve->nxt;
  if ( nve == NULL ) {
    nve = ve->vt->svted;
  }
  return nve;
}
	
Vted *previous_vted(Vted *ve)
{
  Vted *pve;
  
  pve = ve->prv;
  if ( pve == NULL) {
    pve = ve->vt->evted;
  }
  return pve;
}
	
void insert_vtxed(Spvt *vt, Sped *ed)
{
  Vted   *ve, *nve;
  double angle;
  Vted   *create_vtxed(Spvt *, Sped *);
  Spvt   *another_vt(Sped *, Spvt *);
  double calc_angle(Spvt *, Spvt *, Spvt *);
  
  /* first */
  if (vt->svted == (Vted *) NULL) {
    ve = create_vtxed(vt, ed);
    ve->angle = 360.0;
    return;
  }
  
  /* calc angle */
  ve    = vt->svted;
  angle = calc_angle(vt, another_vt(ve->ed, vt), another_vt(ed, vt));
  
  while ((ve != (Vted *) NULL) && (angle < ve->angle)) {
    ve = ve->nxt;
  }

  /* last */
  if (ve == (Vted *) NULL) {
    ve = create_vtxed(vt, ed);
    ve->angle = angle;
    return;
  } 

  nve = (Vted *) malloc(sizeof(Vted));

  nve->prv = ve->prv;
  nve->nxt = ve;
  if (ve->prv != (Vted *) NULL) 
    ve->prv->nxt = nve;
  ve->prv = nve;
  if (vt->svted == ve) vt->svted = nve;

  nve->ed = ed;
  nve->vt = vt;
  nve->angle = angle;
  
/*   if (ed->ve1 != (Vted *) NULL) { */
/*     ed->ve2 = nve; */
/*   } else { */
/*     ed->ve1 = nve; */
/*   } */
  
/*   nve->rotation = SMD_OFF; */
  
  ++( vt->ven );

  return;
}

void make_vtxedlink(Sppd *ppd)
{
  /* tmp */
//  Spvt *vt;
  Sped *ed;
//  Vted *ve;
  void insert_vtxed(Spvt *, Sped *);

  for (ed = ppd->sped; ed != (Sped *) NULL; ed = ed->nxt) {
    insert_vtxed(ed->sv, ed);
    insert_vtxed(ed->ev, ed);
  }
}

//
// for free_isolated_ppdedge
//
void FreeVted( Spvt *vt, Sped *ed )
{
  if ( ed == NULL ) return;

  for ( Vted *ve = vt->svted; ve != (Vted *) NULL; ve = ve->nxt ) {
    if ( ve->ed == ed ) {
      free_vtxed( ve, vt );
      return;
    }
  }
}

void free_ppdvertexedge(Spvt *vt)
{
  Vted *ve, *venxt;
  void free_vtxed(Vted *, Spvt *);

  for (ve = vt->svted; ve != (Vted *) NULL; ve = venxt) {
    venxt = ve->nxt;
    free_vtxed(ve, vt);
  }
  vt->ven = 0;
  vt->svted = vt->evted = (Vted *) NULL;
}

void free_vertexedge( Spvt *vt, Sped *ed )
{
  Vted *vted;
  void free_vtxed( Vted *, Spvt * );
  
  if ( ed == (Sped *) NULL ) return;
  
  for ( vted = vt->svted; vted != (Vted *) NULL; vted = vted->nxt ) {
    if ( vted->ed == ed ) {
      free_vtxed( vted, vt );
      return;
    }
  }

}
      
void free_vtxed( Vted *ve, Spvt *vt )
{
  if (ve == (Vted *) NULL) return;

  ve->ed = (Sped *) NULL;
  ve->vt = (Spvt *) NULL;
  
  if (vt->svted == ve) {
    if ((vt->svted = ve->nxt) != (Vted *) NULL)
      ve->nxt->prv = (Vted *) NULL;
    else {
      vt->evted = (Vted *) NULL;
    }
  } else if (vt->evted == ve) {
    ve->prv->nxt = (Vted *) NULL;
    vt->evted = ve->prv;
  } else {
    ve->prv->nxt = ve->nxt;
    ve->nxt->prv = ve->prv;
  }
  free(ve);
  --( vt->ven );
}

/*void printvtxed(Spvt *vt)
{
  Vted *ve;
  
  display("(vt) %d (link) %d\n", vt->no, vt->ven);
  for (ve = vt->svted; ve != (Vted *) NULL; ve = ve->nxt) 
    display("ed %d\tangle %g\n", ve->ed->no, ve->angle);
}*/

//
// neighboring normals of a vertex
//

Vtnm *create_vtxnm( Spvt *vertex, Spnm *nm )
{
  Vtnm *vn;

  if (nm == (Spnm *) NULL) return (Vtnm *) NULL;
  
  vn = (Vtnm *) malloc(sizeof(Vtnm));

  vn->nxt = (Vtnm *) NULL;
  if (vertex->svtnm == (Vtnm *) NULL) {
    vn->prv       = (Vtnm *) NULL;
    vertex->evtnm = vertex->svtnm = vn;
  } else {
    vn->prv       = vertex->evtnm;
    vertex->evtnm = vertex->evtnm->nxt = vn;
  }

  /* edge */
  vn->nm = nm;
  /* vertex */
  vn->vt    = vertex;
  vn->cnt   = 1;

  ++(vertex->vnn);
  
  return vn;
}

void free_ppdvertexnormal(Spvt *vt)
{
  Vtnm *vn,*vnnxt;
  void free_vtxnm(Vtnm *, Spvt *);

  for (vn = vt->svtnm; vn != (Vtnm *) NULL; vn = vnnxt) {
    vnnxt = vn->nxt;
    free_vtxnm(vn, vt);
  }
  vt->vnn = 0;
  vt->svtnm = vt->evtnm = (Vtnm *) NULL;
}

void free_vtxnm( Vtnm *vn, Spvt *vt )
{
  if (vn == (Vtnm *) NULL) return;

  vn->nm = (Spnm *) NULL;
  vn->vt = (Spvt *) NULL;
  
  if (vt->svtnm == vn) {
    if ((vt->svtnm = vn->nxt) != (Vtnm *) NULL)
      vn->nxt->prv = (Vtnm *) NULL;
    else {
      vt->evtnm = (Vtnm *) NULL;
    }
  } else if (vt->evtnm == vn) {
    vn->prv->nxt = (Vtnm *) NULL;
    vt->evtnm = vn->prv;
  } else {
    vn->prv->nxt = vn->nxt;
    vn->nxt->prv = vn->prv;
  }
  free(vn);
  --( vt->vnn );
  
}

Vtnm *find_ppdnormal( Spvt *vt, Vec *nrm, Sppd *ppd )
{
  Vtnm   *vn;
  double V3Cosign( Vec *, Vec * );
  
  for ( vn = vt->svtnm; vn != (Vtnm *) NULL; vn = vn->nxt ) {
    if ( V3Cosign( &(vn->nm->vec), nrm ) > ppd->smooth_angle ) {
      return vn;
    }
  }

  return (Vtnm *) NULL;
}

//
// for open_ppd
//
void free_ppdvertex_noedge( Sppd *ppd )
{
  Spvt *vt, *nvt;
  void free_ppdvertex( Spvt *, Sppd * );
  
  for ( vt = ppd->spvt; vt != NULL; vt = nvt ) {

    nvt = vt->nxt;
    if ( (!(vt->ven)) && ( vt->sp_type == SP_VERTEX_NORMAL) ) {
      free_ppdvertex( vt, ppd );
    }

  }
}

void ppdvertex_noedge_check( Sppd *ppd ) {
  
  for ( Spvt* vt = ppd->spvt; vt != NULL; vt = vt->nxt )
    {
      if ( !(vt->ven) ) vt->type = VERTEX_ADDED;
    }
}

//
// counter-clockwise order neighboring halfedges of a vertex
// 
Sphe *ppdvertex_ccwnext_halfedge( Sphe *he )
{
  if ( he == NULL ) return NULL;
  
  Sphe *phe = he->prv;
  return phe->mate;
} 

//
// clockwise order neighboring halfedges of a vertex
// 
Sphe *ppdvertex_cwnext_halfedge( Sphe *he )
{
  if ( he == NULL ) return NULL;
  if ( he->mate != NULL ) {
    return he->mate->nxt;
  } else {
    return NULL;
  }
} 

// number of neighboring faces of a vertex
int ppdvertex_num_face( Spvt *vt )
{
  int count = 0;
  Sphe *he;
  
  if ( (he = vt->sphe) == NULL ) return count;
//   printf("he %d fc %d\n", he->no, he->bpfc->no );
  do {
    ++count;
    he = ppdvertex_ccwnext_halfedge( he );
//     if ( he == NULL ) {
//       printf("he NULL\n");
//     } else {
//       printf("(new) he %d fc %d\n", he->no, he->bpfc->no );
//     }
  } while (  (he != vt->sphe) && (he != NULL) );
  
  return count;
}

// number of neighboring edges (vertices) of a vertex
// degree
int ppdvertex_num_edge( Spvt *vt )
{
  int count = 0;
  Sped *ed, *fed;
  ed = fed = ppdvertex_first_edge( vt );
  do {
    ++count;
    ed = ppdvertex_next_edge( ed, vt );
  } while ( (ed != NULL) && (ed != fed) );
  return count;
}
  
// re-attaching ppdvertex's halfedge link
// so that it links a beginning halfedge in counter-clockwise order.
void reattach_ppdvertex_sphe( Spvt *vt )
{
  // 処理するのは境界頂点のみ
  if ( vt->isBoundary == FALSE ) return;
  
  Sphe *he = vt->sphe;
  if ( he == NULL ) return;
  
  while ( he->mate != NULL ) {
    he = ppdvertex_cwnext_halfedge( he );
  }
  vt->sphe = he;
}

// re-attaching ppdvertex's halfedge link
// so that it links a beginning halfedge in counter-clockwise order.
void reattach_ppdvertex_halfedge( Sppd *ppd )
{
  Sphe *he;
  
  for ( Spvt *vt = ppd->spvt; vt != NULL; vt = vt->nxt ) {

//     display("vt %d\n", vt->no );
    // boundary vertex
    //he = vt->sphe;

    if ( vt->isBoundary == TRUE ) {
//      if ( vt->atr == VTXEXTERNAL ) {
      //display("vt %d (b) num %d \n", vt->no, ppdvertex_num_face( vt ) );
//       for ( he = vt->sphe; he != NULL; he = ppdvertex_ccwnext_halfedge( he ) ) {
// 	display("\t(b) ed %d\n", he->ed->no );
//       }
      he = vt->sphe;
      if ( he != NULL ) {
	while ( he->mate != NULL ) {
	  he = ppdvertex_cwnext_halfedge( he );
	}
	vt->sphe = he;
      }

    }
    
  }
}

//
//
Sped *ppdvertex_first_edge( Spvt *vt )
{
  if ( vt->sphe == NULL ) return NULL;
  return vt->sphe->ed;
}

Sped *ppdvertex_next_edge( Sped *ed, Spvt *vt )
{
  Sphe *he = NULL;
  if ( ed->lhe != NULL ) {
    if ( ed->lhe->vt == vt ) {
      he = ed->lhe;
    }
  }
  if ( ed->rhe != NULL ) {
    if ( ed->rhe->vt == vt ) {
      he = ed->rhe;
    }
  }
  if ( he == NULL ) return NULL;
  
  // おそらくこれはこんなに複雑でなくてよい
  if ( he->prv != NULL ) return he->prv->ed;
  else return NULL;
}

//
// fc に対し, vt を含まない ed を返す
//
Sped *ppdvertex_1ring_ppdedge( Spvt *vt, Spfc *fc )
{
  if ( vt == NULL ) return NULL;
  if ( fc == NULL ) return NULL;
  // TRIANGLE only
  if ( fc->hen != TRIANGLE ) return NULL;

  Sphe *he = fc->sphe;
  do {
    Sped *ed = he->ed;
    if ( (ed->sv != vt) && (ed->ev != vt) ) {
      return ed;
    }
  } while ( (he = he->nxt) != fc->sphe );

  return NULL;
}
  
  
  
  
