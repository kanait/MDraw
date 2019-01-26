//
// cylspm.cxx
// shape preserving cylindrical mapping functions
//
// Copyright (c) 2000 by Takashi Kanai
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

#include <stdio.h>
#include <math.h>
#include <assert.h>
#include "smd.h"
#include "tile.h"

#include "ppdvertex.h"
#include "ppdface.h"
#include "ppdedge.h"
#include "veclib.h"
#include "linbcg.h"

#include "floater.h"
#include "cylspm.h"

void cylspm( Sppd *ppd )
{

  // 初期化
  cylspm_initialize( ppd );
  
  // matrix creation process
  Semat *emat = cylspm_create_emat( ppd );
  cylspm_create_vector( ppd, emat );

  // iteration process
  int    iter;
  double rsq;
  (void) linbcg( emat, emat->bx, emat->xx, 1, SMDZEROEPS, 1000, &iter, &rsq );
  (void) linbcg( emat, emat->by, emat->yy, 1, SMDZEROEPS, 1000, &iter, &rsq );
  (void) linbcg( emat, emat->bz, emat->zz, 1, SMDZEROEPS, 1000, &iter, &rsq );

  // vectors -> ppdvertex
  cylspm_solvec_ppdvertex( ppd, emat );

  //free emat, vtxvt
  free_emat3( emat );

  cylspm_finish( ppd );
}  

// 
// 初期化
// 
void cylspm_initialize( Sppd *ppd )
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
// マトリクスの作成
// 
Semat *cylspm_create_emat( Sppd *ppd )
{
  // λの決定
  spm_lambda( ppd );

  // マトリックスの行数を数える
  int vn = 0;
  Spvt *vt;
  for ( vt = ppd->spvt; vt != NULL; vt = vt->nxt )
    {
      if ( vt->isBoundary == FALSE )
	{
	  vn += vt->n_degree;
	}
    }

  //
  // create indexed sparse matrix
  //
  Semat *emat = init_emat3( ppd->vn );
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
  for ( vt = ppd->spvt; vt != NULL; vt = vt->nxt )
    {
    
      emat->sa[vt->sid] = 1.0;
      
      if ( vt->isBoundary == FALSE )
	{
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

void cylspm_create_vector( Sppd *ppd, Semat *emat )
{
  // initialize vector
  for ( int i = 0; i < ppd->vn; ++i )
    {
      emat->bx[i] = emat->by[i] = emat->bz[i] = 0.0;
      emat->xx[i] = emat->yy[i] = emat->zz[i] = 0.0;
    }
  
  for ( Splp *lp = ppd->splp; lp != NULL; lp = lp->nxt )
    {
      double z = ( lp == ppd->splp ) ? 0.0 : 10.0;

      Splv* lv;
      // ループのエッジ長さの総和の計算
      double sum = 0.0;
      for ( lv = lp->splv; lv != NULL; lv = lv->nxt )
	{
	  Splv *nlv = (lv->nxt != NULL ) ? lv->nxt : lp->splv;
	  Sped *ed = find_ppdedge( lv->vt, nlv->vt );
	  assert( ed != NULL );
	  sum += ed->length;
	}

      // 円筒状の座標の計算
      double angle = 0;
      for ( lv = lp->splv; lv != NULL; lv = lv->nxt )
	{

	  int id = lv->vt->sid;
	  emat->bx[id] = RADIUS * cos(angle);
	  emat->by[id] = RADIUS * sin(angle);
	  emat->bz[id] = z;

	  Splv *nlv = (lv->nxt != NULL ) ? lv->nxt : lp->splv;
	  Sped *ed = find_ppdedge( lv->vt, nlv->vt );
	  assert( ed != NULL );
	  angle += (2 * SMDPI * ed->length / sum);
	}
      
    }
  
}

void cylspm_solvec_ppdvertex( Sppd *ppd, Semat *emat )
{
  for ( Spvt *vt = ppd->spvt; vt != NULL; vt = vt->nxt )
    {
      vt->vec.x = emat->xx[ vt->sid ];
      vt->vec.y = emat->yy[ vt->sid ];
      vt->vec.z = emat->zz[ vt->sid ];
    }
}

// 
// 終了
// 
void cylspm_finish( Sppd *ppd )
{
  // lambda, vec2d の領域削除
  for ( Spvt *vt = ppd->spvt; vt != NULL; vt = vt->nxt )
    {
      free( vt->lambda );  vt->lambda = NULL;
      free( vt->vec2d );  vt->vec2d = NULL;
    }
}





