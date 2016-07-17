//
// remesh.cxx
// 
// Copyright (c) 1999 Takashi Kanai; All rights reserved. 
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

#include "esp.h"

#include "remesh.h"
#include "tfcremesh.h"

#if defined(_DEBUG)
// 測地線定義用
Sppd* tmp1ppd = NULL;
#endif

//
// grid mesh for tfc
//
Sppd *tolatticemesh( TFace *tfc, int div,
		     int pc_type // parameter correction type
		     )
{
  if ( tfc == NULL ) return NULL;
  if ( tfc->n_tedge != RECTANGLE ) return NULL;

#if defined(_DEBUG)
  tmp1ppd = swin->screenatr.current_ppd;
#endif

  display("-->rem 1\n");
  // 初期化
  // 頂点への tvector の番号の埋め込み
  TVector *tvec;
  for ( int j = 0; j < tfc->n_tvector; ++j ) {
    tvec = &(tfc->tvec[j]);
    Spvt *vt = tvec->vt;
    // 頂点への tvector の番号の埋め込み
    vt->tvec_id = j;
  }

  // isBoundary の設定
  tfc_calc_boundary( tfc );
  
  // ppdvertex array
  int vn = (div + 1) * (div + 1);
  Spvt **pvt  = (Spvt **) malloc( vn * sizeof(Spvt *) );

  display("-->rem 2\n");
  //
  // Lattice 型 remesh の生成，頂点配列，面配列への格納
  //
  PLArray* pla = init_plarray();
  Sppd *newppd = tfc_remL_init( div, pvt, pla );

  display("-->rem 3\n");
  //
  // 角の点の座標値の決定
  //
  Vec2d uvcorner[RECTANGLE];
  tfc_remL_corner( tfc, uvcorner, div, pvt );
  
  display("-->rem 4\n");
  //
  // 境界上の点（角をのぞく）の座標値の決定
  //
  tfc_remL_boundary( tfc, uvcorner, div, pvt, pla );
  
  display("-->rem 5\n");
  //
  // 内部点（境界をのぞく）の座標値の決定 (補正なし)
  //
  tfc_remL_internal( tfc, uvcorner, div, pvt, pla );

  display("-->rem 6\n");

  // 補正をしない場合ここで終わり
  if ( pc_type == DIFF_NO )
    {
      free_plarray( pla );
      free( pvt );
      return newppd;
    }
  
  //
  // 内部点（境界をのぞく）の座標値のパラメータ補正
  //
  // オリジナルのパラメータ
  Vec2d* ouv  = (Vec2d*) malloc( vn * sizeof(Vec2d) );
  // 補正分のパラメータ，ベクトル
  Vec2d* duv  = (Vec2d*) malloc( vn * sizeof(Vec2d) );
  Vec*   dvec = (Vec*) malloc( vn * sizeof(Vec) );
  for ( int k = 0; k < vn; ++k )
    {
      ouv[k].x = duv[k].x = pvt[k]->uvw.x;
      ouv[k].y = duv[k].y = pvt[k]->uvw.y;
      dvec[k].x = pvt[k]->vec.x;
      dvec[k].y = pvt[k]->vec.y;
      dvec[k].z = pvt[k]->vec.z;

    }

  // 補正
  tfc_remL_internal_update( tfc, vn, duv, dvec, pla, pc_type );

  // 一旦アップデート
  for ( int i = 0; i < vn; ++i )
    {
      pvt[i]->uvw.x = duv[i].x;
      pvt[i]->uvw.y = duv[i].y;
      pvt[i]->vec.x = dvec[i].x;
      pvt[i]->vec.y = dvec[i].y;
      pvt[i]->vec.z = dvec[i].z;

      // 後の再パラメータ化で使う
      pvt[i]->tvec_id = i;
    }

  //
  // ppd のパラメータの再パラメータ化
  // newppd のパラメータが，vt->uvw から ouv に変形したときの ppd のパラメータ
  // の変分を求める
  //
  //  tfc_remL_reparam( ppd, newppd, ouv );

  free( ouv );
  free( duv );
  free( dvec );

  display("-->rem 7\n");
  
  free_plarray( pla );
  
  free( pvt );
  
  return newppd;
}

//
// Lattice 型 remesh の生成，頂点配列への格納
//
Sppd* tfc_remL_init( int div, Spvt** pvt, PLArray* pla )
{
  // ppd, ppdsolid, ppdpart
  Sppd *ppd = create_ppd();
  Spso *newso = create_ppdsolid( ppd );
  Sppt *newpt = create_ppdpart( ppd );

  int i;
  // ppdvertex
  int vn = (div + 1) * (div + 1);
  for ( i = 0; i < vn; ++i )
    {
      pvt[i] = create_ppdvertex( ppd );
      pvt[i]->vec.x = 0.0;
      pvt[i]->vec.y = 0.0;
      pvt[i]->vec.z = 0.0;
      pvt[i]->bpso = newso;
    }

  // ppdface array
  int fn = div * div * 2;
  for ( i = 0; i < fn; ++i )
    {
      Spfc *fc = create_ppdface( ppd );
      fc->bpso = newso;
      fc->bppt = newpt;
    }

  // vertex-face connectivity
  Spfc *fc = ppd->spfc;
  for ( i = 0; i < div; ++i )
    {
      for ( int j = 0; j < div; ++j )
	{
	  Spvt* v1 = pvt[i * (div + 1) + j];
	  Spvt* v2 = pvt[i * (div + 1) + j + 1];
	  Spvt* v3 = pvt[(i + 1) * (div + 1) + j];
	  Spvt* v4 = pvt[(i + 1) * (div + 1) + j + 1];
	  Spfc* f1 = fc; fc = fc->nxt;
	  Spfc* f2 = fc; fc = fc->nxt;
      
	  // create halfedge
	  Sphe *he;
	  he = create_ppdhalfedge(f1); he->vt = v1;
	  he = create_ppdhalfedge(f1); he->vt = v3;
	  he = create_ppdhalfedge(f1); he->vt = v2;
	  he = create_ppdhalfedge(f2); he->vt = v2;
	  he = create_ppdhalfedge(f2); he->vt = v3;
	  he = create_ppdhalfedge(f2); he->vt = v4;
	  
	}
    }

  ///////////////////////////////////////////////////////////////////////////
  //
  // parameter line initialization
  // parameter line の数 = (div+1) + (div+1)
  // XPARAM の方から先に格納する
  //
  pla->n_line = (div+1) + (div+1);
  pla->line = (ParamLine*) malloc( pla->n_line * sizeof(ParamLine) );
  int k = 0; // line の数
  for ( i = 0; i <= div; ++i )
    {
      // vt の格納
      ParamLine* line = &(pla->line[k]);
      init_paramline( line );
      if ( (i == 0) || (i == div) ) line->type = PL_BOUNDARY;
      else line->type = PL_INTERNAL;

      line->n_vt = div+1;
      line->vt    = (Spvt**) malloc( line->n_vt * sizeof(Spvt*) );
      line->vt_id = (int*) malloc( line->n_vt * sizeof(int) );
      for ( int j = 0; j <= div; ++j )
	{
	  line->vt_id[j] = i * (div+1) + j;
	  line->vt[j]    = pvt[line->vt_id[j]];
	}
      ++k;
    }
  for ( i = 0; i <= div; ++i )
    {
      ParamLine* line = &(pla->line[k]);
      init_paramline( line );
      if ( (i == 0) || (i == div) ) line->type = PL_BOUNDARY;
      else line->type = PL_INTERNAL;
      
      line->n_vt = div+1;
      line->vt    = (Spvt**) malloc( line->n_vt * sizeof(Spvt*) );
      line->vt_id = (int*) malloc( line->n_vt * sizeof(int) );
      for ( int j = 0; j <= div; ++j )
	{
	  line->vt_id[j] = j * (div+1) + i;
	  line->vt[j]    = pvt[line->vt_id[j]];
	}
      ++k;
    }
  //
  ///////////////////////////////////////////////////////////////////////////
  
#if 0
  // make edges
  ppd_make_edges( ppd );
  
  //
  // set boundary attributes
  calc_ppd_boundary( ppd );
	
  //
  // re-attaching ppdvertex to halfedge pointer.
  // This function must be executed with ``calc_ppd_boundary''.
  reattach_ppdvertex_halfedge( ppd );
#endif

  return ppd;
}
  

//
// Lattice 型 remesh における内部点の３次元座標値の決定
//
void tfc_remL_internal( TFace* tfc, Vec2d* uvcorner, int div, Spvt **pvt,
			PLArray* pla )
{
  Splp *lp = tfc->clp;
  
#if 0  
  // パラメータ空間上での境界上の始点と終点の格納
  Vec2d *uvstart = (Vec2d *) malloc( (div+1) * sizeof(Vec2d) );
  Vec2d *uvend   = (Vec2d *) malloc( (div+1) * sizeof(Vec2d) );
  Vec2d subs, sube;
  subs.x = uvcorner[1].x - uvcorner[0].x;
  subs.y = uvcorner[1].y - uvcorner[0].y;
  sube.x = uvcorner[2].x - uvcorner[3].x;
  sube.y = uvcorner[2].y - uvcorner[3].y;
  double ddiv = (double) div;
  int i;
  for ( i = 0; i <= div; ++i )
    {
      uvstart[i].x = uvcorner[0].x + (double) i * subs.x / ddiv;
      uvstart[i].y = uvcorner[0].y + (double) i * subs.y / ddiv;
      uvend[i].x   = uvcorner[3].x + (double) i * sube.x / ddiv;
      uvend[i].y   = uvcorner[3].y + (double) i * sube.y / ddiv;
    }
#endif  

  int i;
  double ddiv = (double) div;
  // 始点と終点を結ぶ直線上を探索
  for ( i = 1; i < div; ++i )
    {
#if 0
      // uvstart[i].x, uvstart[i].y の入っている境界エッジを見つける
      Sped *ed = remesh_find_boundaryedge( tfc, lp, uvstart[i].x, uvstart[i].y );
      assert( ed != NULL );

      // 境界エッジに隣接する面を探す
      Spfc *fc = NULL;
      if ( ed->lhe != NULL ) {
	if ( ed->lhe->bpfc->tile_id == tfc->no ) fc = ed->lhe->bpfc;
      }
      if ( ed->rhe != NULL ) {
	if ( ed->rhe->bpfc->tile_id == tfc->no ) fc = ed->rhe->bpfc;
      }
      assert( fc != NULL );
#endif

      ParamLine* line = &(pla->line[i]);
      Sped* ed = line->ed;
      Spfc* fc = line->fc;

      Vec2d uvstart, uvend;
      uvstart.x = line->vt[0]->uvw.x;
      uvstart.y = line->vt[0]->uvw.y;
      uvend.x   = line->vt[line->n_vt-1]->uvw.x;
      uvend.y   = line->vt[line->n_vt-1]->uvw.y;
      
//        display("line %d ed (org %d found %d) fc (org %d found %d)\n",
//  	      i, line->ed->no, ed->no, line->fc->no, fc->no );
      // xparam, yparam の入っている格子点の三次元座標値を求める
      Vec sub;
//        sub.x = uvend[i].x - uvstart[i].x;
//        sub.y = uvend[i].y - uvstart[i].y;
      sub.x = uvend.x - uvstart.x;
      sub.y = uvend.y - uvstart.y;
      for ( int j = 1; j < line->n_vt-1; ++j )
	{

	  //	  int n = i * (div + 1) + j;
	  int n = line->vt_id[j];
	  double xparam = uvstart.x + (double) j * sub.x / ddiv;
	  double yparam = uvstart.y + (double) j * sub.y / ddiv;

	  // point location algorithm ?
	  // xparam, yparam の入っている面を見つける
	  while ( TRUE )
	    {
	      // xparam, yparam が面 fc の中に入っている
	      if ( remesh_isParamsInFace( tfc, fc, xparam, yparam ) == TRUE )
		{
	  
		  // 頂点の座標値を計算
		  Vec2d vec2;
		  vec2.x = xparam; vec2.y = yparam;
		  remesh_uv_to_coord( tfc, fc, &vec2, &(pvt[n]->vec) );
		  
		  // パラメータ座標の設定
		  pvt[n]->uvw.x = xparam;
		  pvt[n]->uvw.y = yparam;

		  break;
	  
		}
	      else // この面にはないので次の面へ
		{ 
	  
		  // 面のエッジのうち，ed ではなく uvstart, uvend によって定義される
		  // 直線と交わるエッジを探す
		  ed = remesh_nextedge( tfc, fc, ed, &(uvstart), &(uvend), NULL );
		  assert( ed != NULL );
		
		  // fc の反対側の面
		  fc = ppdedge_another_fc( ed, fc );
		  assert( fc != NULL );
		}
	    }
	}
    }

#if 0  
  free( uvstart );
  free( uvend );
#endif  
}

#define XPARAM 0
#define YPARAM 1

//
// Lattice 型 remesh における内部点の３次元座標値の
// 測地線距離によるアップデート
//
void tfc_remL_internal_update( TFace* tfc, 
			       int vn,     // pvt の頂点数
			       Vec2d* ouv, // 補正分のパラメータ
			       Vec* ovec,  // 補正分のベクトル
			       PLArray* pla, // パラメータ線群
			       int type )
{
  // パラメータ化補正
  int cnt = 0;
//    for ( int l = 0; l < 2; ++l )
  while ( TRUE )
    {
      ++cnt;
      double valx;
      valx = tfc_uv_isoparam_diff( tfc, vn, ouv, ovec, pla, type );
//        valx = tfc_uv_isoparam_diff( tfc, div, uvcorner, ouv, ovec, type, XPARAM );
//        valy = tfc_uv_isoparam_diff( tfc, div, uvcorner, ouv, ovec, type, YPARAM );

      display("cnt %d valx %g\n", cnt, valx );
//        if ( ((valx < 1.0e-05) && (valy < 1.0e-05)) || (cnt > 20) )
      if ( (valx < 1.0e-05) || (cnt > 20) )
	{
  	  display("cnt %d\n", cnt );
	  break;
	}
//        display("cnt %d\n", cnt );
    }

}

//
// Lattice 型 remesh における境界点の３次元座標値の決定
//
void tfc_remL_boundary( TFace* tfc, Vec2d* uvcorner, int div, Spvt** pvt,
			PLArray* pla )
{
  Splp *lp = tfc->clp;
  
  int u = 0;
  int i;
  int k = 0; // line の数
  for ( i = 0; i < RECTANGLE; ++i )
    {
      // １辺の始点，終点を定める
      Vec2d *s = NULL, *e = NULL;
      if ( i == 0 )
	{
	  u = 0;
	  s = &(uvcorner[0]);
	  e = &(uvcorner[1]);
	}
      else if ( i == 1 )
	{
	  u = div;
	  s = &(uvcorner[3]);
	  e = &(uvcorner[2]);
	}
      else if ( i == 2 )
	{
	  u = 0;
	  s = &(uvcorner[0]);
	  e = &(uvcorner[3]);
	}
      else if ( i == 3 )
	{
	  u = div;
	  s = &(uvcorner[1]);
	  e = &(uvcorner[2]);
	}

      Vec2d sub;
      sub.x = e->x - s->x;
      sub.y = e->y - s->y;

      //
      // 境界の内部点における座標値の計算
      //
      double ddiv = (double) div;

      if ( i == 0 )
	{
	  k = 1; // XPARAM の一番先頭
	}
      else if ( i == 2 )
	  {
	    k = div + 2; // YPARAM の一番先頭
	  }

      //
      // pvt[n], ed, fc の計算
      //
      for ( int j = 1; j < div; ++j )
	{

	  int n;
	  
	  if ( i < 2 )
	    {
	      n = j * (div + 1) + u; // XPARAM
	    } else {
	      n = u * (div + 1) + j; // YPARAM
	    }
      
	  double xparam = s->x + (double) j * sub.x / ddiv;
	  double yparam = s->y + (double) j * sub.y / ddiv;

	  Sped *ed = remesh_find_boundaryedge( tfc, lp, xparam, yparam );
	  assert( ed != NULL );

	  Spfc *fc = NULL;
	  if ( ed->lhe != NULL ) {
	    if ( ed->lhe->bpfc->tile_id == tfc->no ) fc = ed->lhe->bpfc;
	  }
	  if ( ed->rhe != NULL ) {
	    if ( ed->rhe->bpfc->tile_id == tfc->no ) fc = ed->rhe->bpfc;
	  }
	  assert( fc != NULL );

	  // ed, fc を line に格納
	  if ( (i == 0) || (i == 2) )
	    {
	      ParamLine* line = &(pla->line[k]); 
	      line->ed = ed;
	      line->fc = fc;
	      k++;
	    }
	  
	  Vec2d vec2;
	  vec2.x = xparam; vec2.y = yparam;
	  remesh_uv_to_coord( tfc, fc, &vec2, &(pvt[n]->vec) );

	  // パラメータ座標の設定
	  pvt[n]->uvw.x = xparam;
	  pvt[n]->uvw.y = yparam;
	}
    }
}
  
//
// Lattice 型 remesh における角の点の３次元座標値の決定
//
void tfc_remL_corner( TFace* tfc, Vec2d* uvcorner, int div, Spvt** pvt )
{
  Splp *lp = tfc->clp;
  //
  // 角の点の座標値の決定
  //
  int nn[RECTANGLE];
  nn[0] = 0;
  nn[1] = div * (div + 1);
  nn[2] = (div + 1) * (div + 1) - 1;
  nn[3] = div;
  
  // 角のパラメータ値を格納
  int cnt = -1;
  for ( Splv *lv = lp->splv; lv != NULL; lv = lv->nxt )
    {
      if ( lv->vt->sp_type == SP_VERTEX_TVERTEX ) 
	{
	  ++cnt;
	  Spvt *vt = lv->vt;
	  int id = vt->tvec_id;

	  TVector *tvec = &(tfc->tvec[id]);
	  uvcorner[cnt].x = tvec->uv.x;
	  uvcorner[cnt].y = tvec->uv.y;

	  // 座標値の決定
	  pvt[nn[cnt]]->vec.x = vt->vec.x;
	  pvt[nn[cnt]]->vec.y = vt->vec.y;
	  pvt[nn[cnt]]->vec.z = vt->vec.z;

	  // パラメータ座標の設定
	  pvt[nn[cnt]]->uvw.x = uvcorner[cnt].x;
	  pvt[nn[cnt]]->uvw.y = uvcorner[cnt].y;
	}
    }
}

#define MUL 1000

//
// パラメータ線分 start, end 間の測地線距離の計算
//
//  double tfc_uv_geodesic_distance( TFace* tfc, Sped* edS, Spfc* fcS, int div,
//  				 int q, Vec2d* ouv, int type )
double tfc_uv_geodesic_distance( ParamLine* line, TFace* tfc, Vec2d* ouv )
{
//    display("geodesic\n");
  // 最初のエッジと探索面

  Sped* ed = line->ed; if ( ed == NULL ) return 0;
  Spfc* fc = line->fc; if ( fc == NULL ) return 0;
  
//    Sped* ed = edS; if ( ed == NULL ) return 0;
//    Spfc* fc = fcS; if ( fc == NULL ) return 0;

  //#if defined(_DEBUG)
#if 0
  // 測地線定義用ループ
  Splp *lp = create_splp();
#endif
  
  // 交点のパラメータ座標
  // 境界上 (j = 0) 
//    int n = ( type == XPARAM ) ? q * (div + 1) : q;
  int n = line->vt_id[0];
  Vec2d uv; V2Equal( &uv, &(ouv[n]) );
  // 交点の三次元座標
  Vec pos;
  remesh_uv_to_coord( tfc, fc, &uv, &pos );

//  #if defined(_DEBUG)
#if 0
  Splv *lplv; Spvt *lpvt;
  lplv = create_ppdloopvertex( lp );
  lpvt = create_ppdvertex( tmp1ppd );
  lpvt->vec.x = pos.x;
  lpvt->vec.y = pos.y;
  lpvt->vec.z = pos.z;
  lplv->vt = lpvt;
#endif

  // 測地線距離
  double distance = 0.0;

//  #if defined(_DEBUG)
#if 0
  double tmpdis = 0.0;
#endif

  // 探索面がなくなるまで続ける
//    for ( int j = 1; j <= div; ++j )
  for ( int j = 1; j < line->n_vt; ++j )
    {
      Vec2d nuv;
      int n1 = line->vt_id[j];
      V2Equal( &(nuv), &(ouv[n1]) );
      
      // 交差判定を確実に行うためにすこし延ばす
      Vec2d start, end;
      start.x = uv.x;
      start.y = uv.y;
      end.x   = uv.x + MUL * ( nuv.x - uv.x );
      end.y   = uv.y + MUL * ( nuv.y - uv.y );

      // パラメータ uv から nuv までの測地線距離を求める
      double disL = V2DistanceBetween2Points( &uv, &nuv );
      while ( TRUE )
	{
	  // 面のエッジのうち，ed ではなく uv, nuv によって定義される
	  // 直線と交わるエッジを探す (パラメータ交点つき)
	  Vec2d intsec2;
	  Sped* ned = remesh_nextedge( tfc, fc, ed, &start, &end, &intsec2 );
	  assert( ned != NULL );

	  // パラメータ交点の座標値への変換
	  Vec intsec;
	  remesh_uv_to_coord( tfc, fc, &intsec2, &intsec );

	  double pdis = V2DistanceBetween2Points( &uv, &intsec2 );

	  if ( disL > pdis ) // まだ続く
	    {
	      double dis0 = V3DistanceBetween2Points( &pos, &intsec );
	      distance += dis0;
//  #if defined(_DEBUG)
#if 0
	      tmpdis += dis0;
	      tmpdis = 0.0;
#endif
	      
	      // 座標，パラメータ長さのアップデート
	      disL -= pdis;
	      V2Equal( &uv, &intsec2 );
	      V3Equal( &pos, &intsec );

	      
//  #if defined(_DEBUG)
#if 0
	      Splv *lplv; Spvt *lpvt;
	      lplv = create_ppdloopvertex( lp );
	      lpvt = create_ppdvertex( tmp1ppd );
	      lpvt->vec.x = pos.x;
	      lpvt->vec.y = pos.y;
	      lpvt->vec.z = pos.z;
	      lplv->vt = lpvt;
#endif

	      // エッジが境界エッジのときにはループを抜ける
	      if ( ned->sp_type == SP_EDGE_BOUNDARY ) break;
	  
	      // fc の反対側の面
	      fc = ppdedge_another_fc( ned, fc );
	      ed = ned;
	  
	    }
	  else // 終わり
	    {
//  #if defined(_DEBUG)	      
#if 0
	      Vec2d tmpuv;
	      double den = disL / pdis;
	      tmpuv.x = uv.x + den * ( intsec2.x - uv.x );
	      tmpuv.y = uv.y + den * ( intsec2.y - uv.y );
#endif
	      // 終了点はかならず nuv になる
	      Vec fin;
	      remesh_uv_to_coord( tfc, fc, &nuv, &fin );
	      double dis0 = V3DistanceBetween2Points( &pos, &fin );
	      distance += dis0;
	      
//  #if defined(_DEBUG)
#if 0
	      tmpdis += dis0;
#endif
	      
	      // 次のための座標，パラメータ長さのアップデート
	      V2Equal( &uv, &nuv );
	      V3Equal( &pos, &fin );

#if 0
//  #if defined(_DEBUG)
	      if ( j == line->n_vt )
		{
		  Splv *lplv; Spvt *lpvt;
		  lplv = create_ppdloopvertex( lp );
		  lpvt = create_ppdvertex( tmp1ppd );
		  lpvt->vec.x = pos.x;
		  lpvt->vec.y = pos.y;
		  lpvt->vec.z = pos.z;
		  lplv->vt = lpvt;
		}
#endif
	      
	      ed = NULL;
	      break;
	    }

	}
      
    }

//  #if defined(_DEBUG)
#if 0
  // 測地線ファイル出力
  char filename[BUFSIZ];
  sprintf( filename, "geodesic%d_%02d.sp", type, q );
  write_sp_file( lp, filename, "");

  // 開放
  for ( lplv = lp->splv; lplv != NULL; lplv = lplv->nxt )
    {
      free_ppdvertex( lplv->vt, tmp1ppd );
    }
  free_splp( lp );
#endif
  

  return distance;
}

//
// パラメータ線分 start, end 間の弦長の計算
//
//  double uv_chord_length( int div, int q, Vec* ovec, int dir )
double tfc_uv_chord_length( ParamLine* line, Vec* ovec )
{
  // 測地線距離
  double distance = 0.0;

  // 境界上 (j = 0) 
//    int n = ( dir == XPARAM ) ? q * (div + 1) : q;
  int n = line->vt_id[0];
  // 探索面がなくなるまで続ける
  for ( int j = 1; j < line->n_vt; ++j )
    {
//        int n1 = ( dir == XPARAM ) ? (q * (div + 1) + j) : (j * (div + 1) + q);
      int n1 = line->vt_id[j];
      distance += V3DistanceBetween2Points( &(ovec[n]), &(ovec[n1]) );
//        display("\tlength %g\n", V3DistanceBetween2Points( &(ovec[n]), &(ovec[n1]) ) );
      n = n1;
    }

  return distance;
}

// 弦長パラメータ化補正関数
// パラメータの移動量の平均距離を返す
//  type: DIFF_GEODIS (測地線) or DIFF_CHORD (弦長)
//  dir: XPARAM or YPARAM
double tfc_uv_isoparam_diff( TFace* tfc,
			     int vn, 
			     Vec2d* ouv,
			     Vec* ovec,
			     PLArray* pla,
			     int type )
{
  Splp* lp = tfc->clp;

//    int vn = (div + 1) * (div + 1);
  // 補正分
  Vec2d* duv  = (Vec2d*) malloc( vn * sizeof(Vec2d) );
  Vec*   dvec = (Vec*)   malloc( vn * sizeof(Vec) );
  // 初期化
  int k;
  for ( k = 0; k < vn; ++k )
    {
      duv[k].x = ouv[k].x;
      duv[k].y = ouv[k].y;
      dvec[k].x = ovec[k].x;
      dvec[k].y = ovec[k].y;
      dvec[k].z = ovec[k].z;
    }
  
  // パラメータ空間上での境界上の始点と終点の格納
//    for ( int i = 1; i < div; ++i )
//      {
  for ( int i = 0; i < pla->n_line; ++i )
    {
      ParamLine* line = &(pla->line[i]);
      if ( line->type == PL_BOUNDARY ) continue; // 内部パラメータ線のみ処理
      
      Vec2d uvstart;
      int p = line->vt_id[0];
//        int p = ( dir == XPARAM ) ? (i * (div + 1)) : (i);
      V2Equal( &(uvstart), &(ouv[p]) );

#if 0      
      // uvstart.x, uvstart.y の入っている境界エッジを見つける
      Sped *edS = remesh_find_boundaryedge( tfc, lp, uvstart.x, uvstart.y );
      assert( edS != NULL );

      // 境界エッジに隣接する面を探す
      Spfc *fcS = NULL;
      if ( edS->lhe != NULL )
	{
	  if ( edS->lhe->bpfc->tile_id == tfc->no ) fcS = edS->lhe->bpfc;
	}
	
      if ( edS->rhe != NULL )
	{
	  if ( edS->rhe->bpfc->tile_id == tfc->no ) fcS = edS->rhe->bpfc;
	}
      assert( fcS != NULL );
#endif
      Sped* edS = line->ed;
      Spfc* fcS = line->fc;
      
      //パラメータ線分 uvstart, uvend 間の測地線距離の計算
      double geo;
      if ( type == DIFF_GEODIS )
	{
//  	  geo = tfc_uv_geodesic_distance( tfc, edS, fcS, div, i, ouv, dir );
	  geo = tfc_uv_geodesic_distance( line, tfc, ouv );
	}
      else // type == DIFF_CHORD
	{
//  	  geo = uv_chord_length( div, i, ovec, dir );
	  geo = tfc_uv_chord_length( line, ovec );
	}

      display("geo %g\n", geo );
      // 交点のパラメータ座標
      Vec2d uv; V2Equal( &uv, &(uvstart) );
      // 交点の三次元座標
      Vec pos;
      remesh_uv_to_coord( tfc, fcS, &uv, &pos );

      // 確実に交点をだすために end 方向へすこし延ばす

//        int k = 0;
//        int q  = ( dir == XPARAM ) ? (i * (div + 1) + k) : (k * (div + 1) + i);
//        ++k;
//        int q1 = ( dir == XPARAM ) ? (i * (div + 1) + k) : (k * (div + 1) + i);
      int k = 0;
      int q  = line->vt_id[k]; // 境界頂点の番号
      ++k;
      int q1 = line->vt_id[k]; // 次の頂点の番号
      Vec2d startL, endL;
      startL.x = ouv[q].x;
      startL.y = ouv[q].y;
      endL.x   = ouv[q].x + MUL * (ouv[q1].x - ouv[q].x);
      endL.y   = ouv[q].y + MUL * (ouv[q1].y - ouv[q].y);
      
      Sped* ed = edS;
      Spfc* fc = fcS;
      for ( int j = 1; j < line->n_vt-1; ++j )
	{

	  // ステップ長さ
	  double len3 = geo / (double) (line->n_vt-1);

	  double tmpdis = 0.0;
	  
	  while ( TRUE )
	    {
	      Vec2d intsec2;
	      Sped* ned = remesh_nextedge( tfc, fc, ed, &startL, &endL, &intsec2 );
	      assert( ned != NULL );

	      // パラメータ上のもとの点と交点との長さ
	      double pdis  = V2DistanceBetween2Points( &uv, &intsec2 );
	      // パラメータ上のもとの点と終点との長さ
	      double pdis0 = V2DistanceBetween2Points( &uv, &(ouv[q1]) );

	      if ( pdis0 < pdis ) // param update //
		{
		  // geom update かどうかを調べる
		  Vec intsec;
		  remesh_uv_to_coord( tfc, fc, &(ouv[q1]), &intsec );
		  double edis = V3DistanceBetween2Points( &pos, &intsec );
		  
		  if ( len3 < edis ) // param update の前に geom update が訪れる //
		    {
		      // 面上のパラメータ点の計算
		      double den3 = len3 / edis;
//  		      int n = ( dir == XPARAM ) ? (i * (div + 1) + j) : (j * (div + 1) + i);
		      int n = line->vt_id[j];
		      duv[n].x = uv.x + den3 * (ouv[q1].x - uv.x);
		      duv[n].y = uv.y + den3 * (ouv[q1].y - uv.y);
		      // 面上の点の計算
		      remesh_uv_to_coord( tfc, fc, &(duv[n]), &(dvec[n]) );
		      
		      // 座標のアップデート
		      V2Equal( &uv, &(duv[n]) );

#if defined(_DEBUG)
		      //TEMP
		      Vec pos1;
		      remesh_uv_to_coord( tfc, fc, &(uv), &pos1 );
		      tmpdis += V3DistanceBetween2Points( &pos, &pos1 );
		      V3Equal( &pos, &pos1 );
		      //TEMP
#endif	      
		      remesh_uv_to_coord( tfc, fc, &(uv), &pos );
		      
		      break; // while 文を抜ける
		    }
		  else // 本当に param update //
		    {
		      // 走査パラメータの更新
//  		      q  = q1; ++k;
//  		      q1 = ( dir == XPARAM )
//  			? (i * (div + 1) + k) : (k * (div + 1) + i);
		      q  = q1; ++k;
		      q1 = line->vt_id[k];
		      V2Equal( &startL, &(ouv[q]) );
		      endL.x   = ouv[q].x + MUL * (ouv[q1].x - ouv[q].x);
		      endL.y   = ouv[q].y + MUL * (ouv[q1].y - ouv[q].y);
		      
		      // 長さのアップデート
		      len3 -= edis;
		      assert( len3 > 0 );
		      tmpdis += edis;

		      // 座標のアップデート
		      V2Equal( &uv, &(ouv[q]) );
		      V3Equal( &pos, &intsec );

		      ed = NULL;
			      
		    }
		}
	      else // continue //
		{
		  // geom update かどうかを調べる //
		  Vec intsec;
		  remesh_uv_to_coord( tfc, fc, &intsec2, &intsec );
		  double edis = V3DistanceBetween2Points( &pos, &intsec );

		  if ( len3 < edis ) // continue の前に geom update が訪れる
		    {
		      // 面上のパラメータ点の計算
		      double den3 = len3 / edis;
//  		      int n = ( dir == XPARAM )
//  			? (i * (div + 1) + j) : (j * (div + 1) + i);
		      int n = line->vt_id[j];
		      duv[n].x = uv.x + den3 * (intsec2.x - uv.x);
		      duv[n].y = uv.y + den3 * (intsec2.y - uv.y);
		      // 面上の点の計算
		      remesh_uv_to_coord( tfc, fc, &(duv[n]), &(dvec[n]) );

		      // 座標のアップデート
		      V2Equal( &uv, &(duv[n]) );
#if defined(_DEBUG)
		      //TEMP
		      Vec pos1;
		      remesh_uv_to_coord( tfc, fc, &(uv), &pos1 );
		      tmpdis += V3DistanceBetween2Points( &pos, &pos1 );
		      V3Equal( &pos, &pos1 );
		      //TEMP
#endif	      
		      remesh_uv_to_coord( tfc, fc, &(uv), &pos );
		      
		      
		      break; // while 文を抜ける
		    }
		  else // 本当に continue
		    {
		      // 長さのアップデート
		      len3 -= edis;
		      assert( len3 > 0 );
		      tmpdis += edis;
		      
		      // 座標のアップデート
		      V2Equal( &uv, &intsec2 );
		      V3Equal( &pos, &intsec );
		  
		      // エッジが境界エッジのときにはループを抜ける
		      if ( ned->sp_type == SP_EDGE_BOUNDARY ) break;
		      
		      // fc の反対側の面
		      fc = ppdedge_another_fc( ned, fc );
		      ed = ned;
		    }
		}
	      
	    }
	  
	}
      
    }
  // アップデート
  double val = 0.0;
  for ( k = 0; k < vn; ++k )
    {
      double vx = duv[k].x - ouv[k].x;
      double vy = duv[k].y - ouv[k].y;
      val += sqrt( vx * vx + vy * vy );
      ouv[k].x = duv[k].x;
      ouv[k].y = duv[k].y;
      ovec[k].x = dvec[k].x;
      ovec[k].y = dvec[k].y;
      ovec[k].z = dvec[k].z;
    }
  val /= (double) vn;

  // お役ごめん
  free( duv );
  free( dvec );

  return val;
}

#if 0
//
// ppd のパラメータの再パラメータ化
// newppd のパラメータが，vt->uvw から ouv に変形したときの ppd のパラメータ
// の変分を求める
//
void tfc_remL_reparam( Sppd* ppd, Sppd* refppd, Vec2d* ouv )
{
  for ( Spvt* vt = ppd->spvt; vt != NULL; vt = vt->nxt )
    {
      if ( vt->isBoundary == TRUE ) continue;

      // refppd の面 rfc を見つける
      Spfc* rfc;
      if ( (rfc = remL_find_ppdface( refppd, &(vt->uvw) )) == NULL )
	{
	  fprintf(stderr, "okasii!!\n");
	  continue;
	}

      double a1, a2, a3;
      ppdface_find_barycentric_coordinate( rfc, vt->uvw.x, vt->uvw.y,
					   &a1, &a2, &a3 );

      // 新しい座標
      Vec2d *v1, *v2, *v3;
      v1 = &(ouv[rfc->sphe->vt->tvec_id]);
      v2 = &(ouv[rfc->sphe->nxt->vt->tvec_id]);
      v3 = &(ouv[rfc->sphe->nxt->nxt->vt->tvec_id]);
      vt->uvw.x = a1 * v1->x + a2 * v2->x + a3 * v3->x;
      vt->uvw.y = a1 * v1->y + a2 * v2->y + a3 * v3->y;
      
    }
      
}
#endif

//
// radial shape mesh for tfc
//
Sppd *toradialmesh( TFace *tfc, int div )
{
  if ( tfc == NULL ) return NULL;

  // ppd, ppdsolid, ppdpart
  Sppd *newppd = create_ppd();
  Spso *newso = create_ppdsolid( newppd );
  Sppt *newpt = create_ppdpart( newppd );

  // ppdvertex
  int vn = 1 + div * div * tfc->n_tedge;
  Spvt **pvt  = (Spvt **) malloc( vn * sizeof(Spvt *) );

  int i;
  for ( i = 0; i < vn; ++i ) {
    pvt[i] = create_ppdvertex( newppd );
    pvt[i]->vec.x = 0.0;
    pvt[i]->vec.y = 0.0;
    pvt[i]->vec.z = 0.0;
    pvt[i]->bpso = newso;
  }

  // ppdface
  int fid = 0;
  int fn = div * tfc->n_tedge * ( 2 * div - 1 );
  Spfc **pfc  = (Spfc **) malloc( fn * sizeof(Spfc *) );
  for ( i = 0; i < fn; ++i ) {
    pfc[i] = create_ppdface( newppd );
    pfc[i]->bpso = newso;
    pfc[i]->bppt = newpt;
  }

  // vertex-face connectivity
  // center vertex
  Spvt *cv = pvt[0];
  cv->vec.x = tfc->cv->vec.x;
  cv->vec.y = tfc->cv->vec.y;
  cv->vec.z = tfc->cv->vec.z;
  cv->uvw.x = tfc->cv->uvw.x;
  cv->uvw.y = tfc->cv->uvw.y;
    
  for ( i = 0; i < tfc->n_tedge * div; ++i ) {

    // 最も内側の頂点
    Spvt *v1, *v2, *v3, *v4;
    Spfc *f1, *f2;
    Sphe *he;
    v1 = pvt[ 1 + div * i ];
    v2 = ( i != (tfc->n_tedge * div - 1) ) ? pvt[ 1 + div * (i+1) ] : pvt[1];
    f1 = pfc[fid]; ++fid;
    he = create_ppdhalfedge(f1); he->vt = cv;
    he = create_ppdhalfedge(f1); he->vt = v1;
    he = create_ppdhalfedge(f1); he->vt = v2;

    for ( int j = 0; j < div - 1; ++j ) {
      v1 = pvt[ 1 + div * i + j];
      v2 = ( i != (tfc->n_tedge * div - 1) )
	? pvt[ 1 + div * (i+1) + j] : pvt[1+j];
      v3 = pvt[ 1 + div * i + j + 1];
      v4 = ( i != (tfc->n_tedge * div - 1) )
	? pvt[ 1 + div * (i+1) + j+1] : pvt[1+j+1];
      f1 = pfc[fid]; ++fid;
      f2 = pfc[fid]; ++fid;
      he = create_ppdhalfedge(f1); he->vt = v1;
      he = create_ppdhalfedge(f1); he->vt = v3;
      he = create_ppdhalfedge(f1); he->vt = v4;
      he = create_ppdhalfedge(f2); he->vt = v1;
      he = create_ppdhalfedge(f2); he->vt = v4;
      he = create_ppdhalfedge(f2); he->vt = v2;
    }
  }
  //display("fn %d fid %d\n", fn, fid );

  //
  // decide vertex's 3D coordinates
  //

  // 初期化
  // 頂点への tvector の番号の埋め込み
  TVector *tvec;
  for ( i = 0; i < tfc->n_tvector; ++i ) {
    tvec = &(tfc->tvec[i]);
    Spvt *vt = tvec->vt;
    // 頂点への tvector の番号の埋め込み
    vt->tvec_id = i;
  }

  Splp *lp = tfc->clp;

  //
  // 角の点の座標値の決定
  //
  int *nn = (int *) malloc( tfc->n_tedge * sizeof(int) );
  for ( i = 0; i < tfc->n_tedge; ++i ) {
    nn[i] = 1 + i * div * div + div - 1;
  }
  
  // 角のパラメータ値を格納
  Vec2d *uvcorner = (Vec2d *) malloc( tfc->n_tedge * sizeof(Vec2d) );
  int cnt = -1;
  for ( Splv *lv = lp->splv; lv != NULL; lv = lv->nxt ) {
    if ( lv->vt->sp_type == SP_VERTEX_TVERTEX ) {
      ++cnt;
      Spvt *vt = lv->vt;
      int id = vt->tvec_id;
      
      TVector *tvec = &(tfc->tvec[id]);
      uvcorner[cnt].x = tvec->uv.x;
      uvcorner[cnt].y = tvec->uv.y;

      // 座標値の決定
      pvt[nn[cnt]]->vec.x = vt->vec.x;
      pvt[nn[cnt]]->vec.y = vt->vec.y;
      pvt[nn[cnt]]->vec.z = vt->vec.z;
      pvt[nn[cnt]]->uvw.x = tvec->uv.x;
      pvt[nn[cnt]]->uvw.y = tvec->uv.y;
    }
  }

  double ddiv = (double) div;

  //
  // 境界上の点（角をのぞく）の座標値の決定
  //

  int bn = div * tfc->n_tedge;
  int bid = 0;
  Vec2d *uvboundary = (Vec2d *) malloc( bn * sizeof(Vec2d) );
  for ( i = 0; i < tfc->n_tedge; ++i ) {

    Vec2d *s, *e, sub;
    s = &(uvcorner[i]);
    e = ( i != tfc->n_tedge - 1 ) ? &(uvcorner[i+1]) : &(uvcorner[0]);
    sub.x = e->x - s->x;
    sub.y = e->y - s->y;
    uvboundary[bid].x = uvcorner[i].x;
    uvboundary[bid].y = uvcorner[i].y;
//      display("uvboundary %d %g %g\n", bid, uvboundary[bid].x, uvboundary[bid].y );
    ++bid;
    
    for ( int j = 1; j < div; ++j ) {

      int n = 1 + (i * div * div) + (div * j) + div - 1;
      
      double xparam = s->x + (double) j * sub.x / ddiv;
      double yparam = s->y + (double) j * sub.y / ddiv;
      uvboundary[bid].x = xparam;
      uvboundary[bid].y = yparam;
//        display("uvboundary %d %g %g\n", bid, uvboundary[bid].x, uvboundary[bid].y );
      ++bid;
      
      Sped *ed = remesh_find_boundaryedge( tfc, lp, xparam, yparam );
      assert( ed != NULL );
      Spfc *fc = NULL;
      if ( ed->lhe != NULL ) {
	if ( ed->lhe->bpfc->tile_id == tfc->no ) fc = ed->lhe->bpfc;
      }
      if ( ed->rhe != NULL ) {
	if ( ed->rhe->bpfc->tile_id == tfc->no ) fc = ed->rhe->bpfc;
      }
      assert( fc != NULL );
      Vec2d vec2;
      vec2.x = xparam; vec2.y = yparam;
      remesh_uv_to_coord( tfc, fc, &vec2, &(pvt[n]->vec) );
//        double a1, a2, a3;
//        remesh_find_barycentric_coordinate( tfc, fc, xparam, yparam,
//  						  &a1, &a2, &a3 );
//        ppdface_barycentric_coordinate( fc, a1, a2, a3, &(pvt[n]->vec) );

      // パラメータの格納
      pvt[n]->uvw.x = xparam;
      pvt[n]->uvw.y = yparam;
      
    }
  }

  //
  // 内部点（境界をのぞく）の座標値の決定
  //

//    display("cv %d\n", tfc->cv->no );
  Vec2d cuv;
//    cuv.x = pvt[0]->uvw.x;
//    cuv.y = pvt[0]->uvw.y;
  pvt[0]->uvw.x = cuv.x = tfc->tvec[tfc->cv->tvec_id].uv.x;
  pvt[0]->uvw.y = cuv.y = tfc->tvec[tfc->cv->tvec_id].uv.y;
//    pvt[0]->uvw.x = cuv.x = 0.0;
//    pvt[0]->uvw.y = cuv.y = 0.0;
  for ( i = 0; i < tfc->n_tedge * div; ++i ) {

    Vec sub;
//      sub.x = cuv.x - uvboundary[i].x;
//      sub.y = cuv.y - uvboundary[i].y;
    sub.x = uvboundary[i].x - cuv.x;
    sub.y = uvboundary[i].y - cuv.y;

    // 最初の面の探索
    Spfc *fc = remesh_find_intsecface( tfc, tfc->cv, &cuv, &(uvboundary[i]) );
    assert( fc != NULL );
    Sped *ed = NULL;
//      display("found. fc %d\n", fc->no );
    
//      Sped *ed = remesh_find_boundaryedge( tfc, lp, 
//  					 uvboundary[i].x, uvboundary[i].y );
//      assert( ed != NULL );
//      Spfc *fc = NULL;
//      if ( ed->lhe != NULL ) {
//        if ( ed->lhe->bpfc->tile_id == tfc->no ) fc = ed->lhe->bpfc;
//      }
//      if ( ed->rhe != NULL ) {
//        if ( ed->rhe->bpfc->tile_id == tfc->no ) fc = ed->rhe->bpfc;
//      }
//      assert( fc != NULL );
    
    //for ( int j = div - 1; j >= 1; --j ) {
    for ( int j = 1; j < div; ++j ) {
      
      int n = 1 + (div * i) + (j - 1);
//        double xparam = uvboundary[i].x + (double) j * sub.x / ddiv;
//        double yparam = uvboundary[i].y + (double) j * sub.y / ddiv;
      double xparam = cuv.x + (double) j * sub.x / ddiv;
      double yparam = cuv.y + (double) j * sub.y / ddiv;

      while ( TRUE ) {
	
	// xparam, yparam が面 fc の中に入っている
	if ( remesh_isParamsInFace( tfc, fc, xparam, yparam ) == TRUE ) {
	  
	  // 頂点の座標値を計算
	  Vec2d vec2;
	  vec2.x = xparam; vec2.y = yparam;
	  remesh_uv_to_coord( tfc, fc, &vec2, &(pvt[n]->vec) );
      
//  	  double a1, a2, a3;
//  	  remesh_find_barycentric_coordinate( tfc, fc, xparam, yparam,
//  						      &a1, &a2, &a3 );
//  	  ppdface_barycentric_coordinate( fc, a1, a2, a3, &(pvt[n]->vec) );
//  	  display("ok. param %g %g fc %d\n", xparam, yparam, fc->no );
	  
	  // パラメータの格納
	  pvt[n]->uvw.x = xparam;
	  pvt[n]->uvw.y = yparam;
	  
	  break;
	  
	} else { // この面にはないので次の面へ
	  
	  // 面のエッジのうち，ed ではなく uvstart, uvend によって定義される
	  // 直線と交わるエッジを探す
	  if ( ed != NULL ) {
//  	    display("aa\n");
	    ed = remesh_nextedge( tfc, fc, ed, &(cuv), &(uvboundary[i]), NULL );
	  } else {
//  	    display("bb\n");
	    ed = remesh_find_intsecedge( tfc, fc, tfc->cv );
	  }
	  assert( ed != NULL );
//  	  display("ed %d\n", ed->no );
	  
	  // fc の反対側の面
	  fc = ppdedge_another_fc( ed, fc );
	  assert( fc != NULL );
//  	  display("next fc %d\n", fc->no );
	  
	}
	
      }
    }
  }
    
  free( uvboundary );
  free( uvcorner );
  free( nn );
  free( pvt );
  free( pfc );
  
  return newppd;
}

Spfc *remesh_find_intsecface( TFace *tfc, Spvt *cv, Vec2d *v0, Vec2d *v1 )
{
  if ( cv == NULL ) return NULL;

  Sphe *he = cv->sphe;
  do {
    Spfc *fc = he->bpfc;

    Sphe *hhe = ppdface_opposite_halfedge( fc, cv );
    Sped *ed = hhe->ed;
    Vec2d *sv = &(tfc->tvec[ ed->sv->tvec_id ].uv);
    Vec2d *ev = &(tfc->tvec[ ed->ev->tvec_id ].uv);
    if ( isLineSegmentCrossing( sv, ev, v0, v1 ) == TRUE ) {
      return fc;
    }
//      do {
//        Sped *ed = hhe->ed;
//        Vec2d *sv = &(tfc->tvec[ ed->sv->tvec_id ].uv);
//        Vec2d *ev = &(tfc->tvec[ ed->ev->tvec_id ].uv);
//        if ( isLineSegmentCrossing( sv, ev, v0, v1 ) == TRUE ) {
//  	return fc;
//        }
//      } while ( (hhe = hhe->nxt) != fc->sphe );
    he = ppdvertex_ccwnext_halfedge( he );
  } while (  (he != cv->sphe) && (he != NULL) );

  return NULL;
}

Sped *remesh_find_intsecedge( TFace *tfc, Spfc *fc, Spvt *root )
{
  Sphe *he = ppdface_opposite_halfedge( fc, root );
  return he->ed;
}

Sped *remesh_find_boundaryedge( TFace* tfc, Splp *lp, double xparam, double yparam )
{
  for ( Splv *lv = lp->splv; lv != NULL; lv = lv->nxt )
    {

      Spvt *vt0 = lv->vt;
      Spvt *vt1 = ( lv == lp->eplv ) ? lp->splv->vt : lv->nxt->vt;
//        double xval = ( xparam - vt0->uvw.x ) / ( vt1->uvw.x - vt0->uvw.x );
//        double yval = ( yparam - vt0->uvw.y ) / ( vt1->uvw.y - vt0->uvw.y );
//        if ( (0.0 <= xval) && (1.0 >= xval) && (0.0 <= yval) && (1.0 >= yval) ) {
      Vec2d v;
      v.x = xparam; v.y = yparam;
      
      Vec2d* vs = &(tfc->tvec[vt0->tvec_id].uv);
      Vec2d* ve = &(tfc->tvec[vt1->tvec_id].uv);
      
      if ( isParamsOnLine( &v, vs, ve ) == TRUE )
	{
	  double dis = ( V2DistanceBetween2Points( &v, vs )
			 / V2DistanceBetween2Points( vs, ve ) );
//    	  display("\tdis %g\n", dis );
	  if ( (0.0 < dis) && (1.0 > dis) )
	    {
	      Sped *ed = find_ppdedge_new( vt0, vt1 );
	      return ed;
	    }
	}
    }
  
  return NULL;
}

#if 0
Sped *remesh_find_boundaryedge( TFace *tfc, Splp *lp,
				double xparam, double yparam )
{
  for ( Splv *lv = lp->splv; lv != NULL; lv = lv->nxt ) {

    Spvt *vt0 = lv->vt;
    Spvt *vt1 = ( lv == lp->eplv ) ? lp->splv->vt : lv->nxt->vt;
    TVector *s = &(tfc->tvec[vt0->tvec_id]);
    TVector *e = &(tfc->tvec[vt1->tvec_id]);

    double xval = (xparam - s->uv.x) / ( e->uv.x - s->uv.x );
    double yval = (yparam - s->uv.y) / ( e->uv.y - s->uv.y );
//      display("xval %g yval %g\n", xval, yval );

    if ( (0.0 <= xval) && (1.0 >= xval) && (0.0 <= yval) && (1.0 >= yval) ) {
      Sped *ed = find_ppdedge_new( vt0, vt1 );
      return ed;
    }
  }
  return NULL;
}
#endif

#if 0
// 面のエッジのうち，ed ではなく uvstart, uvend によって定義される
// 直線と交わるエッジを探す (パラメータ交点なし)
Sped *remesh_nextedge( TFace *tfc, Spfc *fc, Sped *ed, Vec2d *v0, Vec2d *v1 )
{
  Sphe *he = fc->sphe;
  do {
    if ( he->ed == ed ) continue;

    Vec2d *sv = &(tfc->tvec[ he->ed->sv->tvec_id ].uv);
    Vec2d *ev = &(tfc->tvec[ he->ed->ev->tvec_id ].uv);
    
//      display("sv %g %g ev %g %g v0 %g %g v1 %g %g\n",
//  	    sv->x, sv->y, ev->x, ev->y, 
//  	    v0->x, v0->y, v1->x, v1->y );
    
    if ( isLineSegmentCrossing( sv, ev, v0, v1 ) == TRUE ) {
      
      return he->ed;
    }
    
  } while ( (he = he->nxt) != fc->sphe );

  return NULL;
}
#endif

// 面のエッジのうち，ed ではなく uvstart, uvend によって定義される
// 直線と交わるエッジを探す (パラメータ交点つき)
Sped *remesh_nextedge( TFace* tfc, Spfc *fc, Sped *ed,
		       Vec2d *v0, Vec2d *v1, Vec2d* intsec )
{
  Sphe *he = fc->sphe;
  do
    {
      if ( he->ed == ed )
	{
	  continue;
	}

      Vec2d *sv = &(tfc->tvec[ he->ed->sv->tvec_id ].uv);
      Vec2d *ev = &(tfc->tvec[ he->ed->ev->tvec_id ].uv);
//        display("sv %g %g ev %g %g v0 %g %g v1 %g %g\n",
//  	      sv->x, sv->y, ev->x, ev->y, 
//  	      v0->x, v0->y, v1->x, v1->y );
//        Vec2d *sv = &(he->ed->sv->uvw);
//        Vec2d *ev = &(he->ed->ev->uvw);

      if ( isLineSegmentCrossing( sv, ev, v0, v1 ) == TRUE )
	{
	  if ( intsec != NULL ) 
	    LineIntersectionPoint( sv, ev, v0, v1, intsec );
	  return he->ed;
	}
    }
  while ( (he = he->nxt) != fc->sphe );

  return NULL;
}

BOOL remesh_isParamsInFace( TFace *tfc, Spfc *fc, double xparam, double yparam )
{
  Vec2d *uvw0, *uvw1, uvw;
  Sphe *he = fc->sphe;
  do {
    uvw0 = &(tfc->tvec[ he->vt->tvec_id ].uv);
    uvw1 = &(tfc->tvec[ he->nxt->vt->tvec_id ].uv);
    uvw.x = xparam; uvw.y = yparam;
    if ( isLeftSide( uvw0, uvw1, &uvw ) == FALSE ) return FALSE;
  } while ( (he = he->nxt) != fc->sphe );
  
  return TRUE;
}

//
// パラメータ座標から座標値を求める
//
void remesh_uv_to_coord( TFace* tfc, Spfc* fc, Vec2d* uv, Vec* vec )
{
  // 頂点の座標値を計算
  double a1, a2, a3;
  remesh_find_barycentric_coordinate( tfc, fc, uv->x, uv->y, &a1, &a2, &a3 );
  ppdface_barycentric_coordinate( fc, a1, a2, a3, vec );
}


// find barycentric coordinates
void remesh_find_barycentric_coordinate( TFace *tfc, Spfc *fc,
					 double xparam, double yparam,
					 double *a1, double *a2, double *a3 )
{
  Vec2d vec;
  vec.x = xparam; vec.y = yparam;
  Vec2d *vec1 = &(tfc->tvec[ fc->sphe->vt->tvec_id].uv);
  Vec2d *vec2 = &(tfc->tvec[ fc->sphe->nxt->vt->tvec_id].uv);
  Vec2d *vec3 = &(tfc->tvec[ fc->sphe->nxt->nxt->vt->tvec_id].uv);

  double area = V2TriArea( vec1, vec2, vec3 );
  *a1 = V2TriArea( &vec, vec2, vec3 ) / area;
  *a2 = V2TriArea( &vec, vec3, vec1 ) / area;
  *a3 = V2TriArea( &vec, vec1, vec2 ) / area;
}

//////////////////////////////////////////////////////////////////////////////
//
// Parameter Line functions
//

PLArray* init_plarray( void )
{
  PLArray* pla = (PLArray*) malloc( sizeof(PLArray) );

  pla->n_line = 0;
  pla->line = NULL;

  return pla;
}

void init_paramline( ParamLine* l )
{
  l->type = PL_INTERNAL;
  l->n_vt = 0;
  l->vt = NULL;
  l->vt_id = NULL;
  l->ed = NULL;
  l->fc = NULL;
}
  
void free_plarray( PLArray* pla )
{
  for ( int i = 0; i < pla->n_line; ++i )
    {
      if ( pla->line[i].vt ) free( pla->line[i].vt );
      if ( pla->line[i].vt_id ) free( pla->line[i].vt_id );
    }

  free( pla->line );
  free( pla );
}
  
  

