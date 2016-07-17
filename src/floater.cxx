//
// floater.cxx
//
// Copyright (c) 1997-2000 Takashi Kanai; All rights reserved. 
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
#include "ppdvertex.h"
#include "ppdface.h"
#include "ppdedge.h"
#include "tile.h"
#include "veclib.h"
#include "linbcg.h"
#include "intsec.h"

#include "floater.h"

///////////////////////////////////////////////////////////////////////////
//
// shape-preserving parameterization (in [Floater97(CAGD)])
// for ppd with a boundary
//
///////////////////////////////////////////////////////////////////////////

void ppdspm( Sppd *ppd )
{

  // 初期化
  ppdspm_initialize( ppd );
  
  // matrix creation process
  Semat *emat = create_spm_emat( ppd );
  create_spm_vector( ppd, emat );

  // iteration process
  int    iter;
  double rsq;
  (void) linbcg( emat, emat->bx, emat->xx, 1, SMDZEROEPS, 1000, &iter, &rsq );
  (void) linbcg( emat, emat->by, emat->yy, 1, SMDZEROEPS, 1000, &iter, &rsq );

  // vectors -> ppdvertex
  solvec_ppdvertex( ppd, emat );

  //free emat, vtxvt
  free_emat( emat );

  ppdspm_finish( ppd );
}  

// 
// 初期化
// 
void ppdspm_initialize( Sppd *ppd )
{
  // エッジの持たない頂点を削除
  free_ppdvertex_noedge( ppd );
  
  // 次数の決定
  // lambda, vec2d の領域確保
  Spvt *vt;
  int i;
  for ( i = 0, vt = ppd->spvt; vt != NULL; vt = vt->nxt, ++i )
    {
    
      // 頂点番号の初期化
      vt->sid = i;

      // 次数の決定
      vt->n_degree = ppdvertex_num_edge( vt );
    
      // lambda, vec2d initialize
      vt->vec2d  = (Vec2d *) malloc( (vt->n_degree + 1) * sizeof(Vec2d) );
      vt->lambda = (double *) malloc( (vt->n_degree + 1) * sizeof(double) );
    
    }

  // エッジの長さ
  Sped *ed;
  for ( i = 0, ed = ppd->sped; ed != NULL; ed = ed->nxt, ++i )
    {
      ed->sid = i;
      ed->length = V3DistanceBetween2Points( &(ed->sv->vec),&(ed->ev->vec) );
    }

  // 面の番号付け，面積の計算
  Spfc *fc;
  for ( i = 0, fc = ppd->spfc; fc != NULL; fc = fc->nxt, ++i )
    {
      fc->sid = i;
      fc->area = calc_farea( fc );
    }
}

// 
// 初期化
// 
void ppdspm_finish( Sppd *ppd )
{
  // lambda, vec2d の領域削除
  for ( Spvt *vt = ppd->spvt; vt != NULL; vt = vt->nxt )
    {
      free( vt->lambda );  vt->lambda = NULL;
      free( vt->vec2d );  vt->vec2d = NULL;
    }
}

Semat *create_spm_emat( Sppd *ppd )
{
  // λの決定
  spm_lambda( ppd );

  // マトリックスの行数を数える
  int vn = 0;
  Spvt *vt;
  for ( vt = ppd->spvt; vt != NULL; vt = vt->nxt ) {
    if ( vt->isBoundary == FALSE ) {
      vn += vt->n_degree;
    }
  }

  //
  // create indexed sparse matrix
  //
 Semat *emat = init_emat( ppd->vn );
  emat->num = vn + ppd->vn + 1;
  emat->sa  = (double *) malloc(emat->num * sizeof(double));
  int i;
  for (i = 0; i < emat->num; ++i) emat->sa[i] = 0.0;
  emat->ija = (int *) malloc(emat->num * sizeof(int));
  
  //
  // indexed sparse matrix 各行の要素に値を格納する
  //
  emat->ija[0] = emat->cnum + 1;
  int j = emat->cnum;
  Sped *ed, *fed;
  for ( vt = ppd->spvt; vt != NULL; vt = vt->nxt ) {
    
    emat->sa[vt->sid] = 1.0;

    if ( vt->isBoundary == FALSE ) {
      ed = ppdvertex_first_edge( vt );
      fed = ed;
      i = 1;
      do {
	++j;
	emat->sa[j]  = -1.0 * vt->lambda[i];
	emat->ija[j] = another_vt( ed, vt )->sid;
	++i;
	ed = ppdvertex_next_edge( ed, vt );
      } while ( (ed != NULL) && (ed != fed) );
    }
    
    emat->ija[vt->sid + 1] = j+1;
  }
  
  return emat;
}

#define RADIUS 1.0

static double cornerx[4] = {0.0, 1.0, 1.0, 0.0};
static double cornery[4] = {0.0, 0.0, 1.0, 1.0};

void create_spm_vector( Sppd *ppd, Semat *emat )
{
  // initialize vector
  int i;
  for ( i = 0; i < ppd->vn; ++i ) {
    emat->bx[i] = emat->by[i] = 0.0;
    emat->xx[i] = emat->yy[i] = 0.0;
  }

  // outer loop のエッジの長さの計算
  Splp *lp = ppd->splp;

  // ループのエッジ長さの総和
  Splv *lv;
  double sum = 0.0;
  for ( lv = lp->splv; lv != NULL; lv = lv->nxt ) {
    Splv *nlv = (lv->nxt != NULL ) ? lv->nxt : lp->splv;
    Sped *ed = find_ppdedge( lv->vt, nlv->vt );
    assert( ed != NULL );
    sum += ed->length;
  }

  // lvn:  ループの頂点の数
  // lcvn: ループの角頂点の数
  // ループの一番最初の点 (lp->splv) は，角の点
  Vec2d* vtb = (Vec2d*) malloc( lp->lcvn * sizeof(Vec2d) );
  double *midsum = (double *) malloc( lp->lcvn * sizeof(double) );
  for ( i = 0; i < lp->lcvn; ++i ) midsum[i] = 0.0;
  int cnt = -1;
  double angle = 0;
  for ( lv = lp->splv; lv != NULL; lv = lv->nxt ) {

    if ( lv->type == LOOPVT_CORNER ) {
      ++cnt;
      int id = lv->vt->sid;
      // for uv mapping
      vtb[cnt].x = emat->bx[id] = cornerx[cnt];
      vtb[cnt].y = emat->by[id] = cornery[cnt];
//        vtb[cnt].x = emat->bx[id] = RADIUS * cos(angle);
//        vtb[cnt].y = emat->by[id] = RADIUS * sin(angle);
    }

    Splv *nlv = (lv->nxt != NULL ) ? lv->nxt : lp->splv;
    Sped *ed = find_ppdedge( lv->vt, nlv->vt );
    assert( ed != NULL );
    midsum[cnt] += ed->length;
    angle += (2 * SMDPI * ed->length / sum);
    
  }

  cnt = -1;
  double a = 0.0;
  for ( lv = lp->splv; lv != NULL; lv = lv->nxt ) {

    if ( lv->type == LOOPVT_CORNER )
      {
	++cnt;
	a = 0.0;
      }
    else
      {
	int p = cnt;
	int q = (cnt != lp->lcvn - 1) ? cnt+1 : 0;
	double b = a / midsum[p];
	int id = lv->vt->sid;
	emat->bx[id] = vtb[p].x + b * (vtb[q].x - vtb[p].x);
	emat->by[id] = vtb[p].y + b * (vtb[q].y - vtb[p].y);
//  	emat->bx[id] = ( b * vtb[p].x + a * vtb[q].x ) / midsum[p];
//  	emat->by[id] = ( b * vtb[p].y + a * vtb[q].y ) / midsum[p];
      }

    Splv *nlv = (lv->nxt != NULL ) ? lv->nxt : lp->splv;
    Sped *ed = find_ppdedge( lv->vt, nlv->vt );
    assert( ed != NULL );
    a += ed->length;
    
  }
  
  free( vtb );
  free( midsum );
  
}

// λを決めるための関数
void spm_lambda( Sppd *ppd )
{
  for (Spvt *vt = ppd->spvt; vt != NULL; vt = vt->nxt ) {

    // geodesic polar map
    ppdvertex_gpm( vt );

    // make lambda
    ppdvertex_spm_lambda( vt );
    
  }
}

//
// geodesic polar map functions
// in Welch & Witkin's SIGGRAPH 94 paper
//
// lambda: マトリックスに格納するパラメータ
// vec2d:  測地線極写像のパラメータ値 (x, y)
// 次数(n_degree)+1(その頂点) の数だけ確保
// 頂点まわりの反時計周りの順序
//
void ppdvertex_gpm( Spvt *vt )
{
  // 近傍の頂点まわりの角度を格納
  double *angle;
  angle = (double *) malloc( vt->n_degree * sizeof(double) );

  // vt を原点におく
  vt->vec2d[0].x = 0.0;
  vt->vec2d[0].y = 0.0;

  // 角度の計算
  Spvt *nvt, *pvt;
  Vec vec0, vec1;
  Sphe *he = vt->sphe;
  double nangle = 0.0;
  int n = 0;
  do {
    nvt = he->nxt->vt;
    pvt = he->prv->vt;
    V3Sub( &(nvt->vec), &(vt->vec), &vec0 );
    V3Sub( &(pvt->vec), &(vt->vec), &vec1 );
    angle[n] = acos( V3Cosign( &vec0, &vec1 ) );
    nangle += angle[n];
    ++n;
    he = ppdvertex_ccwnext_halfedge( he );
  } while ( (he != vt->sphe) && (he != NULL) );

  double mul = 2.0 * SMDPI / nangle;
  int i;
  for ( i = 0; i < n; ++i ) angle[i] *= mul;

  // パラメータの計算
  Sped *ed = ppdvertex_first_edge( vt );
  Sped *fed = ed;
  double tangle = 0.0;
  i = 0;
  do {
    vt->vec2d[i+1].x = ed->length * cos( tangle );
    vt->vec2d[i+1].y = ed->length * sin( tangle );
    tangle += angle[i];
    ++i;
    ed = ppdvertex_next_edge( ed, vt );
  } while ( (ed != NULL) && (ed != fed) );

  free( angle );
}

//
// 頂点近傍のλを決定
//
void ppdvertex_spm_lambda( Spvt *vt )
{
  double area;

  //
  // 次数が 3 より大きいとき
  //
  if ( vt->n_degree > 3 ) {

    int i;
    for ( i = 1; i <= vt->n_degree; ++i ) {
      vt->lambda[i] = 0.0;
    }

    for ( i = 1; i <= vt->n_degree; ++i ) {

      int j = 1;
      do {
	int v1 = j;
	int v2 = (j != vt->n_degree) ? j+1 : 1;
	if ( (v1 != i) && (v2 != i) ) {
	  // vt->vec2d[i], vt->vec2d[0] を通る直線と，
	  // 線分 vt->vec2d[v1], vt->vec2d[v2] の交差判定
	  if ( isLeftSide(&(vt->vec2d[i]), &(vt->vec2d[0]), &(vt->vec2d[v1]))
	       != isLeftSide(&(vt->vec2d[i]), &(vt->vec2d[0]), &(vt->vec2d[v2])) ) {

	    // i, v1, v2 の三点が対象
	    area = V2TriArea( &(vt->vec2d[i]), &(vt->vec2d[v1]), &(vt->vec2d[v2]) );
	    vt->lambda[i]  += (V2TriArea( &(vt->vec2d[0]), &(vt->vec2d[v1]), &(vt->vec2d[v2]) )
			       / area);
	    vt->lambda[v1] += (V2TriArea( &(vt->vec2d[i]), &(vt->vec2d[0]), &(vt->vec2d[v2]) )
			       / area);
	    vt->lambda[v2] += (V2TriArea( &(vt->vec2d[i]), &(vt->vec2d[v1]), &(vt->vec2d[0]) )
			       / area);
	    break;
	  }
	}
	++j;
      } while ( j <= vt->n_degree );
      
    }

    for ( i = 1; i <= vt->n_degree; ++i )
      vt->lambda[i] /= (double) vt->n_degree;

    
  } else if ( vt->n_degree == 3 ) {
    // 次数が 3 のとき
    area = V2TriArea( &(vt->vec2d[1]), &(vt->vec2d[2]), &(vt->vec2d[3]) );
    vt->lambda[1] = V2TriArea( &(vt->vec2d[0]), &(vt->vec2d[2]), &(vt->vec2d[3]) ) / area;
    vt->lambda[2] = V2TriArea( &(vt->vec2d[1]), &(vt->vec2d[0]), &(vt->vec2d[3]) ) / area;
    vt->lambda[3] = V2TriArea( &(vt->vec2d[1]), &(vt->vec2d[2]), &(vt->vec2d[0]) ) / area;
    
  }
  
  double sum=0.0;
  for ( int j = 1; j <= vt->n_degree; ++j ) {
    sum += vt->lambda[j];
  }
}

void solvec_ppdvertex( Sppd *ppd, Semat *emat )
{
  for ( Spvt *vt = ppd->spvt; vt != NULL; vt = vt->nxt ) {
    vt->uvw.x = emat->xx[ vt->sid ];
    vt->uvw.y = emat->yy[ vt->sid ];
  }
}

///////////////////////////////////////////////////////////////////////////
//
// shape-preserving parameterization (in [Floater97(CAGD)])
// for tile faces
//
///////////////////////////////////////////////////////////////////////////

void tfcspm( TFace *tfc )
{

  // 初期化
  tfcspm_initialize( tfc );
  
  // matrix creation process
  Semat *emat = create_tfcspm_emat( tfc );
  create_tfcspm_vector( tfc, emat );

  //printemat( emat );
  
  // iteration process
  int    iter;
  double rsq;
  (void) linbcg( emat, emat->bx, emat->xx, 1, SMDZEROEPS, 1000, &iter, &rsq );
  (void) linbcg( emat, emat->by, emat->yy, 1, SMDZEROEPS, 1000, &iter, &rsq );

  // vectors -> ppdvertex
  solvec_tvector( tfc, emat );

  //free emat, vtxvt
  free_emat( emat );

  tfcspm_finish( tfc );

  // parameterization flag
  tfc->isParameterized = TRUE;
}

// 
// 初期化
// 
void tfcspm_initialize( TFace *tfc )
{
  // 頂点への tvector の番号の埋め込み
  // 次数の決定
  // lambda, vec2d の領域確保
  TVector *tvec;
  for ( int i = 0; i < tfc->n_tvector; ++i ) {

    tvec = &(tfc->tvec[i]);
    Spvt *vt = tvec->vt;

    // 頂点への tvector の番号の埋め込み
    vt->tvec_id = i;

    // 次数の決定
    tvec->n_degree = ppdvertex_num_edge( vt );
    
    // lambda, vec2d initialize
    tvec->vec2d  = (Vec2d *) malloc( (tvec->n_degree + 1) * sizeof(Vec2d) );
    tvec->lambda = (double *) malloc( (tvec->n_degree + 1) * sizeof(double) );
  }

}

Semat *create_tfcspm_emat( TFace *tfc )
{
  // λの決定
  tfcspm_lambda( tfc );

  //
  // マトリックスの行数を数える
  //
  int i;
  int vn = 0;
  TVector *tvec;
  for ( i = 0; i < tfc->n_tvector; ++i ) {
    tvec = &(tfc->tvec[i]);
    if ( tvec->vt->sp_type != SP_VERTEX_NORMAL ) continue;
    vn += tvec->n_degree;
  }
  
  //
  // create indexed sparse matrix
  //
  Semat *emat = init_emat( tfc->n_tvector );
  emat->num = vn + tfc->n_tvector + 1;
  emat->sa  = (double *) malloc( emat->num * sizeof(double) );
  for ( i = 0; i < emat->num; ++i ) emat->sa[i] = 0.0;
  emat->ija = (int *) malloc( emat->num * sizeof(int) );
  
  //
  // indexed sparse matrix 各行の要素に値を格納する
  //
  emat->ija[0] = emat->cnum + 1;
  int j = emat->cnum;
  for ( i = 0; i < tfc->n_tvector; ++i ) {
    tvec = &(tfc->tvec[i]);
    Spvt *vt = tvec->vt;
    
    emat->sa[vt->tvec_id] = 1.0;

    // 内部頂点のみ
    if ( vt->sp_type == SP_VERTEX_NORMAL ) {

      Sped *ed = ppdvertex_first_edge( vt );
      Sped *fed = ed;
      int k = 1;
      do {
	++j;
	emat->sa[j]  = -1.0 * tvec->lambda[k];
	emat->ija[j] = another_vt( ed, vt )->tvec_id;
	++k;
	ed = ppdvertex_next_edge( ed, vt );
      } while ( (ed != NULL) && (ed != fed) );
    }
      
    emat->ija[vt->tvec_id + 1] = j+1;
  }
  
  return emat;
}

//
// λを決めるための関数
//
void tfcspm_lambda( TFace *tfc )
{
  for ( int i = 0; i < tfc->n_tvector; ++i ) {

    //Spvt *vt = tfc->tvec[i].vt;
    
    // geodesic polar map
    tvector_gpm( &(tfc->tvec[i]) );

    // make lambda
    tvector_spm_lambda( &(tfc->tvec[i]) );
    
  }
}

//
// geodesic polar map functions
// in Welch & Witkin's SIGGRAPH 94 paper
//
// lambda: マトリックスに格納するパラメータ
// vec2d:  測地線極写像のパラメータ値 (x, y)
// 次数(n_degree)+1(その頂点) の数だけ確保
// 頂点まわりの反時計周りの順序
//
void tvector_gpm( TVector *tvec )
{
  // 近傍の頂点まわりの角度を格納
  double *angle;
  angle = (double *) malloc( tvec->n_degree * sizeof(double) );

  // vt を原点におく
  tvec->vec2d[0].x = 0.0;
  tvec->vec2d[0].y = 0.0;

  // 角度の計算
  Spvt *vt = tvec->vt;
  Spvt *nvt, *pvt;
  Vec vec0, vec1;
  Sphe *he = vt->sphe;
  double nangle = 0.0;
  int n = 0;
  do {
    nvt = he->nxt->vt;
    pvt = he->prv->vt;
    V3Sub( &(nvt->vec), &(vt->vec), &vec0 );
    V3Sub( &(pvt->vec), &(vt->vec), &vec1 );
    angle[n] = acos( V3Cosign( &vec0, &vec1 ) );
    nangle += angle[n];
    ++n;
    he = ppdvertex_ccwnext_halfedge( he );
  } while ( (he != vt->sphe) && (he != NULL) );

  double mul = 2.0 * SMDPI / nangle;
  int i;
  for ( i = 0; i < n; ++i ) angle[i] *= mul;

  // パラメータの計算
  Sped *ed = ppdvertex_first_edge( vt );
  Sped *fed = ed;
  double tangle = 0.0;
  i = 0;
  do {
    tvec->vec2d[i+1].x = ed->length * cos( tangle );
    tvec->vec2d[i+1].y = ed->length * sin( tangle );
    tangle += angle[i];
    ++i;
    ed = ppdvertex_next_edge( ed, vt );
  } while ( (ed != NULL) && (ed != fed) );

  free( angle );
}

//
// 頂点近傍のλを決定
//
void tvector_spm_lambda( TVector *tvec )
{
  double area;
  
  // 次数が 3 より大きいとき
  if ( TRUE ) {

    int i;
    for ( i = 1; i <= tvec->n_degree; ++i ) {
      tvec->lambda[i] = 0.0;
    }

    for ( i = 1; i <= tvec->n_degree; ++i ) {

      int j = 1;
      do {
	int v1 = j;
	int v2 = (j != tvec->n_degree) ? j+1 : 1;
	if ( (v1 != i) && (v2 != i) ) {
	  // tvec->vec2d[i], tvec->vec2d[0] を通る直線と，
	  // 線分 tvec->vec2d[v1], tvec->vec2d[v2] の交差判定
	 if ( isLeftSide(&(tvec->vec2d[i]), &(tvec->vec2d[0]), &(tvec->vec2d[v1]))
	       != isLeftSide(&(tvec->vec2d[i]), &(tvec->vec2d[0]), &(tvec->vec2d[v2])) ) {

	    // i, v1, v2 の三点が対象
	   area = V2TriArea( &(tvec->vec2d[i]), &(tvec->vec2d[v1]), &(tvec->vec2d[v2]) );
	   tvec->lambda[i] += (V2TriArea( &(tvec->vec2d[0]), &(tvec->vec2d[v1]), &(tvec->vec2d[v2]) ) / area);
	   tvec->lambda[v1] += (V2TriArea( &(tvec->vec2d[i]), &(tvec->vec2d[0]), &(tvec->vec2d[v2]) ) / area);
	   tvec->lambda[v2] += (V2TriArea( &(tvec->vec2d[i]), &(tvec->vec2d[v1]), &(tvec->vec2d[0]) ) / area);
	    break;
	  }
	}
	++j;
      } while ( j <= tvec->n_degree );
      
    }

    for ( i = 1; i <= tvec->n_degree; ++i )
      tvec->lambda[i] /= (double) tvec->n_degree;

    
  } else if ( tvec->n_degree == 3 ) {
    // 次数が 3 のとき
     area = V2TriArea( &(tvec->vec2d[1]), &(tvec->vec2d[2]), &(tvec->vec2d[3]) );
     tvec->lambda[1] = V2TriArea( &(tvec->vec2d[0]), &(tvec->vec2d[2]), &(tvec->vec2d[3]) ) / area;
     tvec->lambda[2] = V2TriArea( &(tvec->vec2d[1]), &(tvec->vec2d[0]), &(tvec->vec2d[3]) ) / area;
     tvec->lambda[3] = V2TriArea( &(tvec->vec2d[1]), &(tvec->vec2d[2]), &(tvec->vec2d[0]) ) / area;
     
  }
}

//
// 方程式のベクトル決め (Ax = b の b を決める関数)
//
void create_tfcspm_vector( TFace *tfc, Semat *emat )
{
  // initialize vector
  int i;
  for ( i = 0; i < tfc->n_tvector; ++i ) {
    emat->bx[i] = emat->by[i] = 0.0;
    emat->xx[i] = emat->yy[i] = 0.0;
  }

  // outer loop のエッジの長さの計算
  Splp *lp = tfc->clp;
  Splv *lv;

  // ループのエッジ長さの総和
  double sum = 0.0;
  for ( lv = lp->splv; lv != NULL; lv = lv->nxt )
    {
      Splv *nlv = (lv->nxt != NULL ) ? lv->nxt : lp->splv;
      Sped *ed = find_ppdedge( lv->vt, nlv->vt );
      assert( ed != NULL );
      sum += ed->length;
    }

  //
  // 角の点の uv 座標値の決定
  //
  // lvn:  ループの頂点の数
  // tfc->n_tedge: ループの角頂点の数
  // ループの一番最初の点 (lp->splv) は，角の点
  //
  Vec2d* vtb = (Vec2d*) malloc( tfc->n_tedge * sizeof(Vec2d) );
  double *midsum = (double *) malloc( tfc->n_tedge * sizeof(double) );
  for ( i = 0; i < tfc->n_tedge; ++i ) midsum[i] = 0.0;
  int cnt = -1;
  double angle = 0;
  for ( lv = lp->splv; lv != NULL; lv = lv->nxt ) {

    if ( lv->vt->sp_type == SP_VERTEX_TVERTEX ) {
      ++cnt;
      int id = lv->vt->tvec_id;
      vtb[cnt].x = emat->bx[id] = RADIUS * cos(angle);
      vtb[cnt].y = emat->by[id] = RADIUS * sin(angle);
    }

    Splv *nlv = (lv->nxt != NULL ) ? lv->nxt : lp->splv;
    Sped *ed = find_ppdedge( lv->vt, nlv->vt );
    assert( ed != NULL );
    midsum[cnt] += ed->length;
    angle += (2 * SMDPI * ed->length / sum);
    
  }

  //
  // 角以外の境界の uv 座標値の決定
  //
  cnt = -1;
  double a = 0.0;
  for ( lv = lp->splv; lv != NULL; lv = lv->nxt ) {

    if ( lv->vt->sp_type == SP_VERTEX_TVERTEX )
      {
	++cnt;
	a = 0.0;
      }
    else
      {
	int p = cnt;
	int q = (cnt != tfc->n_tedge - 1) ? cnt+1 : 0;
	int id = lv->vt->tvec_id;
	double b = a / midsum[p];
	emat->bx[id] = vtb[p].x + b * (vtb[q].x - vtb[p].x);
	emat->by[id] = vtb[p].y + b * (vtb[q].y - vtb[p].y);
      }

    Splv *nlv = (lv->nxt != NULL ) ? lv->nxt : lp->splv;
    Sped *ed = find_ppdedge( lv->vt, nlv->vt );
    assert( ed != NULL );
    a += ed->length;
    
  }
  
  free( vtb );
  free( midsum );
}

void solvec_tvector( TFace *tfc, Semat *emat )
{
  for ( int i = 0; i < tfc->n_tvector; ++i ) {
    TVector *tvec = &(tfc->tvec[i]);
    //Spvt *vt = tvec->vt;
    tvec->uv.x = emat->xx[i];
    tvec->uv.y = emat->yy[i];
  }
}

//
// 終了手続
// 
void tfcspm_finish( TFace *tfc )
{
  // lambda, vec2d の領域削除
  for ( int i = 0; i < tfc->n_tvector; ++i ) {
    TVector *tvec = &(tfc->tvec[i]);
    free( tvec->lambda );  tvec->lambda = NULL;
    free( tvec->vec2d );  tvec->vec2d = NULL;
  }
}

