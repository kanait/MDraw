//
// ppdface.cxx
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
#include "ppdvertex.h"
#include "ppdface.h"
#include "ppdedge.h"
#include "veclib.h"

//
// PPD Face functions
//

// create face
Spfc *create_ppdface( Sppd *ppd )
{
  Spfc *fc;

  fc = (Spfc *) malloc(sizeof(Spfc));
  
  fc->nxt = (Spfc *) NULL;

  if (ppd->spfc == (Spfc *) NULL) {
    fc->prv = (Spfc *) NULL;
    ppd->epfc = ppd->spfc = fc;
  } else {
    fc->prv   = ppd->epfc;
    ppd->epfc = ppd->epfc->nxt = fc;
  }

  // half edge
  fc->hen   = 0;
  fc->sphe  = (Sphe *) NULL;
  // back part
  fc->bppt  = (Sppt *) NULL;
  // back solid
  fc->bpso  = (Spso *) NULL;
  // id
  fc->no    = ppd->fid;
  fc->sid   = SMDNULL;
  // face color
  fc->col   = FACEBLUE;

  // for copy_ppd
//    fc->nfc   = (Spfc *) NULL;

  fc->tile_id = SMDNULL;

  ++( ppd->fn );
  ++( ppd->fid );

  return fc;
}

// next ccw vertex in a face
Spvt *other_vertices_face_left( Spfc *fc, Spvt *vt )
{
  Sphe *he;
  
  he = fc->sphe;

  do {
    if ( he->vt == vt ) {
      return he->nxt->vt;
    }
  } while ( (he = he->nxt) != fc->sphe );

  return NULL;
}

// next cw vertex in a face
Spvt *other_vertices_face_right( Spfc *fc, Spvt *vt )
{
  Sphe *he;
  
  he = fc->sphe;

  do {
    if ( he->vt == vt ) {
      return he->prv->vt;
    }
  } while ( (he = he->nxt) != fc->sphe );
  
  return NULL;
}

// free ppdface
void free_ppdface( Spfc *fc, Sppd *ppd )
{
  if ( fc == NULL ) return;
  
//    display("fc %d\n", fc->no );
  // free halfedge
  Sphe *he = fc->sphe;
  while ( he != NULL ) {
    he = free_ppdhalfedge( he, fc );
  }

//    display("spfc %d\n", ppd->spfc->no );
  if ( ppd->spfc == fc ) {
//      display("aaa\n");
    if ( (ppd->spfc = fc->nxt) != NULL )
      fc->nxt->prv = NULL;
    else {
      ppd->epfc = NULL;
    }
  } else if ( ppd->epfc == fc ) {
//      display("bbb\n");
    fc->prv->nxt = NULL;
    ppd->epfc = fc->prv;
    
  } else {
//      display("ccc\n");
    fc->prv->nxt = fc->nxt;
    fc->nxt->prv = fc->prv;
  }

  free(fc);
  --( ppd->fn );
}

//
// for tface_subdivide_ppdface
//
void free_isolated_ppdface( Spfc *fc, Sppd *ppd )
{
  if ( fc == NULL ) return;
  
  // free halfedge
  Sphe *he = fc->sphe;
  while ( he != NULL ) {
    he = free_isolated_ppdhalfedge( he, fc );
  }
  
  if ( ppd->spfc == fc ) {
    if ((ppd->spfc = fc->nxt) != (Spfc *) NULL)
      fc->nxt->prv = (Spfc *) NULL;
    else {
      ppd->epfc = (Spfc *) NULL;
    }
  } else if (ppd->epfc == fc) {
    fc->prv->nxt = (Spfc *) NULL;
    ppd->epfc = fc->prv;
    
  } else {
    fc->prv->nxt = fc->nxt;
    fc->nxt->prv = fc->prv;
  }

  free(fc);
  --( ppd->fn );
}

// number
int num_ppdface( Sppd *ppd )
{
  int i = 0;
  Spfc *fc;
  for ( fc = ppd->spfc; fc != NULL; fc = fc->nxt ) {
    ++i;
  }
  
  return i;
}

// an halfedge opposite to a vertex in a face
// this function is valid for only triangle faces
Sphe *ppdface_opposite_halfedge( Spfc *fc, Spvt *vt )
{
  Sphe *he;
  Sped *ed;
  
  he = fc->sphe;
  do {
    ed = he->ed;
    if ( (ed->sv != vt) && (ed->ev != vt) ) return he;
  } while ( (he = he->nxt) != fc->sphe );
  
  return NULL;
}

// compute face normal (all)
void ppdfacenormal( Sppd *ppd )
{
  if ( ppd == NULL ) return;

  for ( Spfc *fc = ppd->spfc; fc != NULL; fc = fc->nxt ) {
    calc_fnorm( fc );
//      display("fc %d nrm %g %g %g\n", fc->no, fc->nrm.x, fc->nrm.y, fc->nrm.z );
  }
}

// compute face normal
void calc_fnorm( Spfc *face )
{
  Sphe   *h;
  Spvt   *v1, *v2, *v3;
  Vec    vec1, vec2, vec3, vec4, vec5;

  h = face->sphe;
  v1 = h->vt;
  v2 = h->nxt->vt;
  v3 = h->nxt->nxt->vt;
  vec1.x = v1->vec.x;
  vec1.y = v1->vec.y;
  vec1.z = v1->vec.z;
  vec2.x = v2->vec.x;
  vec2.y = v2->vec.y;
  vec2.z = v2->vec.z;
  vec3.x = v3->vec.x;
  vec3.y = v3->vec.y;
  vec3.z = v3->vec.z;
  V3Sub( &vec2, &vec1, &vec4 );
  V3Sub( &vec3, &vec1, &vec5 );
  V3Cross( &vec4, &vec5, &(face->nrm) );
  V3Normalize( &(face->nrm) );
}

//
// PPD HalfEdge functions
//

// create halfedge
Sphe *create_ppdhalfedge( Spfc *fc )
{
  Sphe *he;

  he = (Sphe *) malloc( sizeof(Sphe) );
  
  if (fc->sphe == (Sphe *) NULL) {	/* first */
    fc->sphe = he;
    he->prv  = he;
    he->nxt  = he;
  } else {
    he->prv  = fc->sphe->prv;
    he->nxt  = fc->sphe;
    fc->sphe->prv->nxt = he;
    fc->sphe->prv = he;
  }
  // id
  he->no   = fc->hen;
  
  // back face
  he->bpfc = fc;

  // edge
  he->ed   = (Sped *) NULL;
  
  // start vertex
  he->vt  = (Spvt *) NULL;
  
  // start normal
  he->nm  = (Spnm *) NULL;

  // mate
  he->mate  = (Sphe *) NULL;

  // for copy_ppd
//    he->nhe = NULL;

  // for esptree
//   he->occupied = NULL;
  
  ++( fc->hen );

  return he;
}

// free halfedge
Sphe *free_ppdhalfedge( Sphe *he, Spfc *face )
{
  Sphe *newhe;
  Sped *ed = he->ed;

  if ( ed != NULL ) {
    // link off left halfedge
    if ( ed->lhe == he ) {
      ed->lhe = NULL;
//        --(ed->fn);
    }
    if ( ed->rhe == he ) {
      ed->rhe = NULL;
//        --(ed->fn);
    }
  }
  he->ed = NULL;
    
  // cut off the mate
  if ( he->mate != NULL ) {
    if ( he->mate->mate == he ) {
      he->mate->mate = NULL;
    }
  }
  he->mate = NULL;
      
  
  if ( he->nxt == he ) {
    free( he );
    --( face->hen );
    return (Sphe *) NULL;
  } else {
    newhe = he->nxt;
    he->prv->nxt = he->nxt;
    he->nxt->prv = he->prv;
    free(he);
    --(face->hen);
    return newhe;
  }
}

//
// free halfedge
// for free_isolated_ppdface
Sphe *free_isolated_ppdhalfedge( Sphe *he, Spfc *face )
{
  Sphe *newhe;

  he->ed = NULL;
  he->mate = NULL;
  if ( he->nxt == he ) {
    free( he );
    --( face->hen );
    return (Sphe *) NULL;
  } else {
    newhe = he->nxt;
    he->prv->nxt = he->nxt;
    he->nxt->prv = he->prv;
    free(he);
    --(face->hen);
    return newhe;
  }
}

// not used
Spfc *find_ppdface_ppdedge( Sped *ed, Spvt *sv, Spvt *ev )
{
  if ( ed == NULL ) return NULL;

  if ( (ed->sv == sv) && (ed->ev == ev ) ) {
    
    if ( ed->lhe != NULL ) {
      return ed->lhe->bpfc;
    } else return NULL;
    
  } else if ( (ed->sv == ev) && (ed->ev == sv ) ) {

    if ( ed->rhe != NULL ) {
      return ed->rhe->bpfc;
    } else return NULL;
    
  }
  
  return NULL;
}

// conversion from a face to a plane
void ppdface_to_plane( Spfc *fc,
		       double *a, double *b, double *c, double *d )
{
  Vec *v1, *v2, *v3;
  Vec p1, p2, pc;

  v1 = &(fc->sphe->vt->vec);
  v2 = &(fc->sphe->nxt->vt->vec);
  v3 = &(fc->sphe->nxt->nxt->vt->vec);
  V3Sub( v2, v1, &p1 );
  V3Sub( v3, v1, &p2 );
  V3Cross( &p1, &p2, &pc );
  *a = pc.x; *b = pc.y; *c = pc.z;
  *d = - ( pc.x * v1->x + pc.y * v1->y + pc.z * v1->z );
}

// halfedge to vector
void ppdhalfedge_to_vec( Sphe *he, Vec *vec )
{
  V3Sub( &(he->nxt->vt->vec), &(he->vt->vec), vec );
  //V3Sub( &(he->vt->vec), &(he->nxt->vt->vec), vec );
  //V3Normalize( vec );
}

// find barycentric coordinates using area 
void ppdface_find_barycentric_coordinate( Spfc *fc, double xparam, double yparam,
					  double *a1, double *a2, double *a3 )
{
  Vec2d vec;
  vec.x = xparam; vec.y = yparam;
  Vec2d *vec1 = &(fc->sphe->vt->uvw);
  Vec2d *vec2 = &(fc->sphe->nxt->vt->uvw);
  Vec2d *vec3 = &(fc->sphe->nxt->nxt->vt->uvw);

  double area = V2TriArea( vec1, vec2, vec3 );
  *a1 = V2TriArea( &vec, vec2, vec3 ) / area;
  *a2 = V2TriArea( &vec, vec3, vec1 ) / area;
  *a3 = V2TriArea( &vec, vec1, vec2 ) / area;
  //display("area %g a1 %g a2 %g a3 %g\n", area, *a1, *a2, *a3 );
}

// find barycentric coordinates using linear-equation
void ppdface_calc_barycentric_coordinates( Spfc *fc, Vec2d *vec,
					   double *a1, double *a2, double *a3 )
{
  Vec2d *vec0 = &(fc->sphe->vt->uvw);
  Vec2d *vec1 = &(fc->sphe->nxt->vt->uvw);
  Vec2d *vec2 = &(fc->sphe->nxt->nxt->vt->uvw);

  double a = vec1->x - vec0->x;
  double b = vec2->x - vec0->x;
  double c = vec1->y - vec0->y;
  double d = vec2->y - vec0->y;
  double x = vec->x  - vec0->x;
  double y = vec->y  - vec0->y;

  double sub = a * d - b * c;

  *a2 = (d * x - b * y) / sub;
  *a3 = (a * y - c * x) / sub;
  *a1 = 1 - *a2 - *a3;
}

// barycentric coordinates
void ppdface_barycentric_coordinate( Spfc *fc, double a1, double a2, double a3,
				     Vec *vec )
{
  Vec *vec1 = &(fc->sphe->vt->vec);
  Vec *vec2 = &(fc->sphe->nxt->vt->vec);
  Vec *vec3 = &(fc->sphe->nxt->nxt->vt->vec);
  vec->x = a1 * vec1->x + a2 * vec2->x + a3 * vec3->x;
  vec->y = a1 * vec1->y + a2 * vec2->y + a3 * vec3->y;
  vec->z = a1 * vec1->z + a2 * vec2->z + a3 * vec3->z;
}

//
// パラメータ座標から座標値を求める
//
void ppdface_uv_to_coord( Spfc* fc, Vec2d* uv, Vec* vec )
{
  // 頂点の座標値を計算
  double a1, a2, a3;
  ppdface_find_barycentric_coordinate( fc, uv->x, uv->y, &a1, &a2, &a3 );
  ppdface_barycentric_coordinate( fc, a1, a2, a3, vec );
}

//
// triangulation
//
void ppdface_triangulation( Spfc *fc, Sppd *ppd )
{
  int n = fc->hen;
  Sped **edarray = (Sped **) malloc( n * sizeof( Sped * ) );
  Spvt **vtarray = (Spvt **) malloc( n * sizeof( Spvt * ) );

  int i;
  Sphe *he;
  for( i = 0, he = fc->sphe; i < n; ++i, he = he->nxt ) {
    vtarray[i] = he->vt;
    edarray[i] = he->ed;
  }

  // 分割面の生成（とりあえず）
  Spfc **fcarray = (Spfc **) malloc( (n - 2) * sizeof( Spfc * ) );
  for ( i = 0; i < n - 2; ++i ) {
    Spfc *newfc;
    newfc = fcarray[i] = create_ppdface( ppd );
    newfc->bpso = fc->bpso;
    newfc->bppt = fc->bppt;
  }

  // 頂点の格納
  Spvt **newvtarray = (Spvt **) malloc( (n - 1) * sizeof( Spvt * ) );
  for ( i = 0; i < n - 1; ++i ) {
    newvtarray[i] = vtarray[i+1];
  }
  Sped **newedarray = (Sped **) malloc( (n - 1) * sizeof( Sped * ) );
  newedarray[0] = fc->sphe->ed;
  newedarray[ n - 2 ] = fc->sphe->prv->ed;

  // 分割エッジの生成
  for ( i = 1; i < n - 2; ++i ) {
    Sped *ed;
    ed = newedarray[i] = create_ppdedge( ppd );
    ed->sv = vtarray[0];
    ed->ev = newvtarray[i];
    ed->length = V3DistanceBetween2Points( &(ed->sv->vec), &(ed->ev->vec) );
    (void) create_vtxed( newedarray[i]->sv, ed );
    (void) create_vtxed( newedarray[i]->ev, ed );
  }

  // ハーフエッジの生成とリンクの生成
  for ( i = 0; i < n - 2; ++i ) {

    for ( int j = 0; j < TRIANGLE; ++j ) {

      Spvt *vt; Sped *ed;
      if ( j == 0 ) {
	vt = vtarray[0]; ed = newedarray[i];
      } else if ( j == 1 ) {
	vt = newvtarray[i]; ed = edarray[i+1];
      } else {
	vt = newvtarray[i+1]; ed = newedarray[i+1];
      }

      Sphe *he = create_ppdhalfedge( fcarray[i] );
      he->vt = vt; he->ed = ed;

      // mate の付けかえ
      if ( ed->sv == he->vt ) {
	ed->lhe = he;
	if ( ed->rhe != NULL ) {
	  he->mate = ed->rhe;
	  ed->rhe->mate = he;
	}
      } else {
	ed->rhe = he;
	if ( ed->lhe != NULL ) {
	  he->mate = ed->lhe;
	  ed->lhe->mate = he;
	}
      }
      // vertex -> halfedge のリンクのつけかえ
      he->vt->sphe = he;
    }
  }

  // 法線の定義
  for ( i = 0; i < n - 2; ++i ) {
    calc_fnorm( fcarray[i] );
  }
  
  free(edarray); free(newedarray);
  free(vtarray); free(newvtarray);
  free(fcarray); 
	
}  

// compute face area
double calc_farea( Spfc *face )
{
  Sphe *h = face->sphe;
  Spvt *v1 = h->vt;
  Spvt *v2 = h->nxt->vt;
  Spvt *v3 = h->nxt->nxt->vt;
  Vec vec4, vec5, nrm;
  V3Sub( &(v2->vec), &(v1->vec), &vec4 );
  V3Sub( &(v3->vec), &(v1->vec), &vec5 );
  V3Cross( &vec4, &vec5, &(nrm) );

  return 0.5 * V3Length( &(nrm) );
}
