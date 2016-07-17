//
// spm.cxx
// shape preserving mapping functions
// based on Floater's CAGD paper
//
// Copyright (c) 1999-2000 by Takashi Kanai; All rights reserved. 
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

#include "ppd.h"
#include "ppdlist.h"
#include "ppdvertex.h"
#include "ppdface.h"
#include "ppdedge.h"
#include "veclib.h"
#include "linbcg.h"

#include "uvwsigproc.h"
#include "floater.h"
#include "umbrella.h"
#include "remesh.h"

#include "cylspm.h"

#include "spm.h"
#include "ps.h"

#if !defined(_WINDOWS)

//#define INTERVAL 50
#define INTERVAL 10

int main( int argc, char *argv[] )
{
#if 1
  if ( argc != 5 )
    {
      fprintf( stderr, "usage: %s in.ppd out.ppd type num.\n", argv[0] );
      fprintf( stderr, "\ttype: remeshing type: 0: geodesic distance, 1: chord length. \n" );
      fprintf( stderr, "\tnum: the number of divisions. \n" );
      exit(1);
    }
#endif

  // open ppd
  Sppd *ppd = (Sppd *) NULL;
  if ( (ppd  = open_ppd( argv[1], TRUE )) == NULL )
  // if ( (ppd  = open_ppd( "tigerear.ppd", TRUE )) == NULL )
    {
      fprintf( stderr, "Error: can't open %s.\n", argv[1] );
      exit(1);
    }

  // shape preserving mapping
  if ( ppd->ln == 1 )
    {
      // shape preserving mapping
      ppdspm( ppd );
      
      write_ppd_uvw( "uvw.ppd", ppd );
      write_ppd_file( "paramtex.ppd", ppd, TRUE );
      
//        ppdparam( ppd, SP_MEMBRANE );
//        ppdparam( ppd, SP_EWEIGHT );
//        ppdparam( ppd, SP_FWEIGHT );
//        ppdparam( ppd, SP_TPWEIGHT );
//        ppdparam( ppd, SP_HMWEIGHT );

#if 0
      // relaxation by fairing
      double kpb = 0.1;
      double lambda = 0.7;
      double mu = 1.0 / ( kpb - (1.0 / lambda) );
      int nrepeat = 100;
      uvwsigproc( ppd, lambda, mu, nrepeat, SP_MEMBRANE );
//        uvwsigproc( ppd, lambda, mu, nrepeat, SP_EWEIGHT );
//        uvwsigproc( ppd, lambda, mu, nrepeat, SP_FWEIGHT );
//        uvwsigproc( ppd, lambda, mu, nrepeat, SP_TPWEIGHT );
//        uvwsigproc( ppd, lambda, mu, nrepeat, SP_HMWEIGHT );
  
      write_ppd_uvw( "remeshuvw1.ppd", ppd );
#endif
      
      // int div = INTERVAL;
      int div = atoi(argv[4]);
      int type = (atoi(argv[3]) == 0) ? DIFF_GEODIS : DIFF_CHORD;
      // int div = 12;
      // int type = DIFF_GEODIS;
      Sppd *newppd = tolatticeppd( ppd, div, type );
//        Sppd *newppd = toradialppd( ppd, div );

      // write ppd
      write_ppd_file( argv[2], newppd, FALSE );
      // write_ppd_file( "remesh.ppd", newppd, FALSE );
      write_ppd_file( "parammodtex.ppd", ppd, TRUE );
      write_ppd_uvw( "remeshuvw.ppd", newppd );
      write_ppd_uvw( "reparam.ppd", ppd );
      harmonicmap_to_ps( "reparam.ps", ppd );

      free_ppd( newppd );

    }
#if 0  
  else if ( ppd->ln == 2 )
    {
      cylspm( ppd );
      write_ppd_file( "tmp.ppd", ppd, FALSE );
    }
#endif  
      
  free_ppd( ppd );

  return TRUE;
}

#endif

////////////////////////////////////////////////////////////////////////
//
// parameterization using umbrella-operator
//
////////////////////////////////////////////////////////////////////////

void ppdparam( Sppd *ppd, int type )
{
  // 初期化
  ppdparam_initialize( ppd );

  // boundary mapping
  boundary_mapping( ppd );

  // mapping internal vertices
  internal_mapping( ppd, type );

  // finish
  //ppdspm_finish( ppd );
}  

void ppdparam_initialize( Sppd *ppd )
{
  // エッジの持たない頂点を削除
  free_ppdvertex_noedge( ppd );
  
  // 頂点の番号付け, uvw の初期化
  Spvt *vt;
  int i;
  for ( i = 0, vt = ppd->spvt; vt != NULL; vt = vt->nxt, ++i )
    {
      vt->sid = i;
      vt->uvw.x = vt->uvw.y = 0.0;
    }

  // エッジの番号付け，長さの計算
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

#define RADIUS 1.0
//
// boundary mapping (used in ppdspm)
//
void boundary_mapping( Sppd *ppd )
{
  // outer loop のエッジの長さの計算
  Splp *lp = ppd->splp;

  // ループのエッジ長さの総和
  Splv *lv;
  double sum = 0.0;
  for ( lv = lp->splv; lv != NULL; lv = lv->nxt )
    {
      Splv *nlv = (lv->nxt != NULL ) ? lv->nxt : lp->splv;
      Sped *ed = find_ppdedge( lv->vt, nlv->vt );
      assert( ed != NULL );
      sum += ed->length;
    }

  // lvn:  ループの頂点の数
  // lcvn: ループの角頂点の数
  // ループの一番最初の点 (lp->splv) は，角の点
  Vec2d *vtb = (Vec2d*) malloc( lp->lcvn * sizeof(Vec2d) );
  double *midsum = (double *) malloc( lp->lcvn * sizeof(double) );
  int i;
  for ( i = 0; i < lp->lcvn; ++i ) midsum[i] = 0.0;
  int cnt = -1;
  double angle = 0;
  for ( lv = lp->splv; lv != NULL; lv = lv->nxt )
    {

      if ( lv->type == LOOPVT_CORNER ) // lv が角の頂点の場合
	{
	  ++cnt;
	  vtb[cnt].x = lv->vt->uvw.x = RADIUS * cos(angle);
	  vtb[cnt].y = lv->vt->uvw.y = RADIUS * sin(angle);
	}
      
      Splv *nlv = (lv->nxt != NULL ) ? lv->nxt : lp->splv;
      Sped *ed = find_ppdedge( lv->vt, nlv->vt );
      assert( ed != NULL );
      midsum[cnt] += ed->length;
      angle += (2 * SMDPI * ed->length / sum);
    }

  cnt = -1;
  double a = 0.0;
  for ( lv = lp->splv; lv != NULL; lv = lv->nxt )
    {

      if ( lv->type == LOOPVT_CORNER )
	{
	  ++cnt;
	  a = 0.0;
	  continue;
	}

      Splv *nlv = (lv->nxt != NULL ) ? lv->nxt : lp->splv;
      Sped *ed = find_ppdedge( lv->vt, nlv->vt );
      assert( ed != NULL );
      a += ed->length;
      int p = cnt;
      int q = (cnt == lp->lcvn - 1) ? 0 : cnt+1;
      double b = midsum[p] - a;
      lv->vt->uvw.x = ( b * vtb[p].x + a * vtb[q].x ) / midsum[p];
      lv->vt->uvw.y = ( b * vtb[p].y + a * vtb[q].y ) / midsum[p];
    
    }
  
  free( vtb );
  free( midsum );
  
}

//
// internal vertices mapping
//
void internal_mapping( Sppd *ppd, int type )
{
  //int nrepeat = 100;
  //
  // k回のフェアリングステップ
  //
  double ans  = 0.0;
  int k = 0;
  while ( TRUE )
    {
//    for ( int k = 0; k < nrepeat; ++k )
//      {
      switch ( type )
	{
	case SP_MEMBRANE:
	  ans = param_membrane( ppd );
	  break;
	case SP_EWEIGHT:
	  ans = param_eweight( ppd );
	  break;
	case SP_FWEIGHT:
	  ans = param_fweight( ppd );
	  break;
	case SP_TPWEIGHT:
	  ans = param_tpweight( ppd );
	  break;
	case SP_HMWEIGHT:
	  ans = param_hmweight( ppd );
	  break;
	}

      //printf("k %d\n", k);
      if ( ans < 1.0e-04 ) return;
      
      ++k;
      //if ( k == ppd->vn ) return;
    }
}
  
//
// 膜の弾性エネルギー(membrane energy) の最小化による平滑化
//
// Kobbelt 論文 (SG98) の umbrella-algorithm を使って解いている
// Taubin の重心法 (最も簡単な離散ラプラシアン, SG95) と一致
//
double param_membrane( Sppd *ppd )
{
  Vec2d *update = (Vec2d *) malloc( ppd->vn * sizeof(Vec2d) );
  
  int i;
  Spvt *vt;
  for ( i = 0, vt = ppd->spvt; vt != NULL; vt = vt->nxt, ++i )
    {
      if ( vt->isBoundary != TRUE ) // 内部頂点に関する処理
	{
	  //
	  // umbrella-operator
	  Vec2d d;
	  uvw_umbrella_operator( vt, &d );
	  //uvw_sd_umbrella_operator( vt, &d );
	  
	  // update vertex coordinate
	  update[i].x = vt->uvw.x + d.x;
	  update[i].y = vt->uvw.y + d.y;
	}
      else  // 境界頂点に関する処理
	{
	  update[i].x = vt->uvw.x;
	  update[i].y = vt->uvw.y;
	}
    }
  // 値の更新
  double ans = 0.0;
  for ( i = 0, vt = ppd->spvt; vt != NULL; vt = vt->nxt, ++i )
    {
      ans += V2DistanceBetween2Points( &(vt->uvw), &(update[i]) );
      vt->uvw.x = update[i].x;
      vt->uvw.y = update[i].y;
    }
  ans /= (double) ppd->vn;

  free( update );

  return ans;
}

//
// umbrella-operator の重みの計算にエッジの長さを使っている
// Taubin 論文 (SG95) に登場
//
// Kobbelt 論文 (SG98) の umbrella-algorithm を使って解いている
//
double param_eweight( Sppd *ppd )
{
  Vec2d *update = (Vec2d *) malloc( ppd->vn * sizeof(Vec2d) );
  
  int i;
  Spvt *vt;
  for ( i = 0, vt = ppd->spvt; vt != NULL; vt = vt->nxt, ++i )
    {
      if ( vt->isBoundary != TRUE ) // 内部頂点に関する処理
	{
	  // vt の隣接頂点に対する処理
	  double sum = 0.0;
	  Vted *vted = NULL;
	  for ( vted = vt->svted; vted != NULL; vted = vted->nxt )
	    {
	      sum += ( vted->ed->length );
	    }
	  
	  Vec2d d; d.x = 0.0; d.y = 0.0;
	  for ( vted = vt->svted; vted != NULL; vted = vted->nxt )
	    {
	      double frac = ( vted->ed->length ) / sum;
	      Spvt *avt = another_vt( vted->ed, vt );
	      d.x += (frac * avt->uvw.x);
	      d.y += (frac * avt->uvw.y);
	    }
	  d.x -= vt->uvw.x;
	  d.y -= vt->uvw.y;
	  update[i].x = vt->uvw.x + d.x;
	  update[i].y = vt->uvw.y + d.y;
	}
      else  // 境界頂点に関する処理
	{
	  update[i].x = vt->uvw.x;
	  update[i].y = vt->uvw.y;
	}
    }
  // 値の更新
  double ans = 0.0;
  for ( i = 0, vt = ppd->spvt; vt != NULL; vt = vt->nxt, ++i )
    {
      ans += V2DistanceBetween2Points( &(vt->uvw), &(update[i]) );
      vt->uvw.x = update[i].x;
      vt->uvw.y = update[i].y;
    }
  ans /= (double) ppd->vn;
  
  free( update );
  
  return ans;
}

//
// umbrella-operator の重みの計算に隣接面の面積を使っている
// Taubin 論文 (SG95) に登場
//
// Kobbelt 論文 (SG98) の umbrella-algorithm を使って解いている
//
double param_fweight( Sppd *ppd )
{
  Vec2d *update = (Vec2d *) malloc( ppd->vn * sizeof(Vec2d) );
  
  int i;
  Spvt *vt;
  for ( i = 0, vt = ppd->spvt; vt != NULL; vt = vt->nxt, ++i )
    {
      if ( vt->isBoundary != TRUE ) // 内部頂点に関する処理
	{
	  // vt の隣接頂点に対する処理
	  double sum = 0.0;
	  Vted *vted = NULL;
	  for ( vted = vt->svted; vted != NULL; vted = vted->nxt )
	    {
	      double val = 0.0;
	      val += ( vted->ed->lhe->bpfc->area / 2.0 );
	      val += ( vted->ed->rhe->bpfc->area / 2.0 );
	      sum += val;
	    }
	  
	  Vec2d d; d.x = 0.0; d.y = 0.0;
	  for ( vted = vt->svted; vted != NULL; vted = vted->nxt )
	    {
	      double val = 0.0;
	      val += ( vted->ed->lhe->bpfc->area / 2.0 );
	      val += ( vted->ed->rhe->bpfc->area / 2.0 );
	      double frac = val / sum;
	      Spvt *avt = another_vt( vted->ed, vt );
	      d.x += (frac * avt->uvw.x);
	      d.y += (frac * avt->uvw.y);
	    }
	  d.x -= vt->uvw.x;
	  d.y -= vt->uvw.y;
	  update[i].x = vt->uvw.x + d.x;
	  update[i].y = vt->uvw.y + d.y;
	}
      else  // 境界頂点に関する処理
	{
	  update[i].x = vt->uvw.x;
	  update[i].y = vt->uvw.y;
	}
    }
  // 値の更新
  double ans = 0.0;
  for ( i = 0, vt = ppd->spvt; vt != NULL; vt = vt->nxt, ++i )
    {
      ans += V2DistanceBetween2Points( &(vt->uvw), &(update[i]) );
      vt->uvw.x = update[i].x;
      vt->uvw.y = update[i].y;
    }
  ans /= (double) ppd->vn;

  free( update );

  return ans;
}

//
// thin plate energy による離散ラプラシアンフェアリング
// ( in Kobbelt's SG98 paper )
//
// Kobbelt 論文 (SG98) の umbrella-algorithm を使って解いている
//
double param_tpweight( Sppd *ppd )
{
  Vec2d *update = (Vec2d *) malloc( ppd->vn * sizeof(Vec2d) );
  
  int i;
  Spvt *vt;
  for ( i = 0, vt = ppd->spvt; vt != NULL; vt = vt->nxt, ++i )
    {
      if ( vt->isBoundary != TRUE ) // 内部頂点に関する処理
	{
	  //
	  // (umbrella-operator)^2
	  //
	  double frac = 1.0 / (double) vt->ven;
	  // for calculating diagonal element
	  double fracj = 0.0; 
	  Vec2d dd; dd.x = dd.y = 0.0;
	  for ( Vted *vted = vt->svted; vted != NULL; vted = vted->nxt )
	    {
	      Spvt *avt = another_vt( vted->ed, vt );
	      Vec2d da;
	      uvw_umbrella_operator_bfree( avt, &da );
	      dd.x += (frac * da.x);
	      dd.y += (frac * da.y);
	      // for calculating diagonal element
	      fracj += (1.0 / (double) avt->ven);
	    }
	  Vec2d dp;
	  uvw_umbrella_operator_bfree( vt, &dp );
	  dd.x -= dp.x;
	  dd.y -= dp.y;

	  // diagonal element
	  double mu = 1.0 + frac * fracj;
	  // update vertex coordinate
	  update[i].x = vt->uvw.x - dd.x / mu;
	  update[i].y = vt->uvw.y - dd.y / mu;
	}
      else  // 境界頂点に関する処理
	{
	  update[i].x = vt->uvw.x;
	  update[i].y = vt->uvw.y;
	}
    }
  // 値の更新
  double ans = 0.0;
  for ( i = 0, vt = ppd->spvt; vt != NULL; vt = vt->nxt, ++i )
    {
      ans += V2DistanceBetween2Points( &(vt->uvw), &(update[i]) );
      vt->uvw.x = update[i].x;
      vt->uvw.y = update[i].y;
    }
  ans /= (double) ppd->vn;

  free( update );

  return ans;
}

//
// harmonic mapping の重みを使った離散ラプラシアンフェアリング
// Eck95 の係数
//
// Kobbelt 論文 (SG98) の umbrella-algorithm を使って解いている
//
double param_hmweight( Sppd *ppd )
{
  Vec2d *update = (Vec2d *) malloc( ppd->vn * sizeof(Vec2d) );
  
  int i;
  Spvt *vt;
  for ( i = 0, vt = ppd->spvt; vt != NULL; vt = vt->nxt, ++i )
    {
      if ( vt->isBoundary != TRUE ) // 内部頂点に関する処理
	{
	  // 隣接のエッジに対する重み
	  double *w = (double *) malloc( vt->ven * sizeof(double) );
	  
	  // vt の隣接頂点に対する処理
	  double sum = 0.0;
	  int j = 0;
	  Vted *vted = NULL;
	  for ( vted = vt->svted; vted != NULL; vted = vted->nxt )
	    {
	      w[j] = param_hmw( vted->ed );
	      //printf("w %g\n", w[j] );
	      sum += w[j];
	      ++j;
	    }
	  
	  Vec2d d; d.x = 0.0; d.y = 0.0;
	  j = 0;
	  for ( vted = vt->svted; vted != NULL; vted = vted->nxt )
	    {
	      double frac = w[j] / sum;
	      Spvt *avt = another_vt( vted->ed, vt );
	      d.x += (frac * avt->uvw.x);
	      d.y += (frac * avt->uvw.y);
	      ++j;
	    }
	  d.x -= vt->uvw.x;
	  d.y -= vt->uvw.y;
	  update[i].x = vt->uvw.x + d.x;
	  update[i].y = vt->uvw.y + d.y;

	  free( w );
	}
      else  // 境界頂点に関する処理
	{
	  update[i].x = vt->uvw.x;
	  update[i].y = vt->uvw.y;
	}
    }
  // 値の更新
  double ans = 0.0;
  for ( i = 0, vt = ppd->spvt; vt != NULL; vt = vt->nxt, ++i )
    {
      ans += V2DistanceBetween2Points( &(vt->uvw), &(update[i]) );
      vt->uvw.x = update[i].x;
      vt->uvw.y = update[i].y;
    }
  ans /= (double) ppd->vn;

  free( update );

  return ans;
}

// harmonic mapping weight
double param_hmw( Sped *ed )
{
  if ( ed == NULL ) return 0.0;

  double w = 0.0;
  for ( int i = 0; i < 2; ++i )
    {
      Sphe *he = ( i == 0 ) ? ed->lhe : ed->rhe;
      if ( he != NULL )
	{
	  Spfc *fc = he->bpfc;
	  Sped *e0 = NULL, *e1 = NULL, *e2 = NULL;
	  if ( ed == fc->sphe->ed )
	    {
	      e0 = fc->sphe->ed;
	      e1 = fc->sphe->nxt->ed;
	      e2 = fc->sphe->nxt->nxt->ed;
	    }
	  else if ( ed == fc->sphe->nxt->ed )
	    {
	      e0 = fc->sphe->nxt->ed;
	      e1 = fc->sphe->nxt->nxt->ed;
	      e2 = fc->sphe->ed;
	    }
	  else if ( ed == fc->sphe->nxt->nxt->ed )
	    {
	      e0 = fc->sphe->nxt->nxt->ed;
	      e1 = fc->sphe->ed;
	      e2 = fc->sphe->nxt->ed;
	    }
	  w += (
		( (e1->length * e1->length)
		  + (e2->length * e2->length)
		  - (e0->length * e0->length)
		  ) / fc->area 
		);
	}
    }

  return w;
      
}

////////////////////////////////////////////////////////////////////////
//
// write to file functions
//
////////////////////////////////////////////////////////////////////////

//
// 頂点を二次元平面内の座標におきかえた ppd を出力
//
void write_ppd_uvw( char *fname, Sppd *ppd )
{
  FILE *fp;
  Spso *s;
  Sppt *p;
  Spfc *f;
  Spvt *v;
  Spnm *nm;
  Sphe *he;
  Splp *l;
  Splv *lv;
  int  i, j, n;
  Id	 a, b;
  //	Id	 id;
  Vec  vec;
  //	Vec  vec1, vec2;
  int  solidnum, partnum, vertexnum, normalnum;
  int  facenum, loopnum, fnodenum;
  int  bfc, efc, bpt, ept, bvt, evt, bnm, enm;
  Id	 vtx[128], nrm[128];
	
  if ((fp = fopen(fname, "w")) == NULL) {
    return;
  }
	
  // header
  for (s = ppd->spso, solidnum = 0; s != (Spso *) NULL; s = s->nxt) {
    ++solidnum;
  }
  for (p = ppd->sppt, partnum = 0; p != (Sppt *) NULL; p = p->nxt) {
    ++partnum;
  }
  for (v = ppd->spvt, vertexnum = 0; v != (Spvt *) NULL; v = v->nxt) {
    if ( v->sp_type != SP_VERTEX_STEINER ) {
      ++vertexnum;
    }
  }
  for (l = ppd->splp, loopnum = 0; l != (Splp *) NULL; l = l->nxt) {
    if ( l->type != SHORTESTPATH ) ++loopnum;
  }
  for (nm = ppd->spnm, normalnum = 0; nm != (Spnm *) NULL; nm = nm->nxt) {
    ++normalnum;
  }
  for (f = ppd->spfc, facenum = fnodenum = 0; f != (Spfc *) NULL; f = f->nxt) {
    ++facenum;
    fnodenum += f->hen;
  }
	
  fprintf(fp, "header\n");
  if (solidnum)  fprintf(fp, "\tsolid\t%d\n",  solidnum);
  if (partnum)	 fprintf(fp, "\tpart\t%d\n",	 partnum);
  if (normalnum) fprintf(fp, "\tnormal\t%d\n", normalnum);
  if (vertexnum) fprintf(fp, "\tvertex\t%d\n", vertexnum);
  if (facenum)	 {
    fprintf(fp, "\tface\t%d\n",  facenum);
    fprintf(fp, "\tfnode\t%d\n", fnodenum);
  }
  fprintf(fp, "end\n");
	
  // solid
  if (solidnum) {
    bfc = efc  = 0;
    bpt = ept  = 0;
    bvt = evt  = 0;
    bnm = enm  = 0;
    fprintf(fp, "solid\n");
    i = 1;
    for (s = ppd->spso; s != (Spso *) NULL; s = s->nxt) {
      fprintf(fp, "\t%d\t", i); // print Id
      if (partnum) {
	bpt = ept + 1;
	for (p = ppd->sppt; p != (Sppt *) NULL; p = p->nxt) {
	  if (p->bpso == s) {
	    ++ept;
	  }
	  fprintf(fp, "/p %d %d ", bpt, ept);
	}
	if (vertexnum) {
	  bvt = evt + 1;
	  for (v = ppd->spvt; v != (Spvt *) NULL; v = v->nxt) {
	    if (v->bpso == s) {
	      if ( v->sp_type != SP_VERTEX_STEINER ) {
		++evt;
	      }
	    }
	  }
	  fprintf(fp, "/v %d %d ", bvt, evt);
	}
	if (normalnum) {
	  bnm = enm + 1;
	  for (nm = ppd->spnm; nm != (Spnm *) NULL; nm = nm->nxt) {
	    if (nm->bpso == s) {
	      ++enm;
	    }
	  }
	  fprintf(fp, "/n %d %d ", bnm, enm);
	}
	if (facenum) {
	  bfc = efc + 1;
	  for (p = ppd->sppt; p != (Sppt *) NULL; p = p->nxt) {
	    for (f = ppd->spfc; f != (Spfc *) NULL; f = f->nxt) {
	      if ((f->bpso == s) && (f->bppt == p)) {
		++efc;
	      }
	    }
	  }
	  fprintf(fp, "/f %d %d ", bfc, efc);
	}
	++i;
      }
      fprintf(fp, "\n");
    }
    fprintf(fp, "end\n");
  }
	
  // part
  if (partnum) {
    bfc = efc  = 0;
    fprintf(fp, "part\n");
    i = 1;
    for (p = ppd->sppt; p != (Sppt *) NULL; p = p->nxt) {
      fprintf(fp, "\t%d\t", i); // print Id
      if (facenum) {
	bfc = efc + 1;
	for (f = ppd->spfc; f != (Spfc *) NULL; f = f->nxt) {
	  if (f->bppt == p) {
	    ++efc;
	  }
	}
	fprintf(fp, "/f %d %d ", bfc, efc);
      }
      ++i;
      fprintf(fp, "\n");
    }
    fprintf(fp, "end\n");
  }
	
  // vertex
  if (vertexnum) {
    fprintf(fp, "vertex\n");
    i = 1;
    for (s = ppd->spso; s != (Spso *) NULL; s = s->nxt) {
      for (v = ppd->spvt; v != (Spvt *) NULL; v = v->nxt) {
	if (v->bpso == s) {
	  if ( v->sp_type != SP_VERTEX_STEINER ) {
	    v->sid = i;
	    vec.x = v->uvw.x;
	    vec.y = v->uvw.y;
	    vec.z = 0.0;
	    fprintf(fp, "\t%d\t%g %g %g\n", i, vec.x, vec.y, vec.z);
	    ++i;
	  }
	}
      }
    }
    fprintf(fp, "end\n");
  }
	
  // normal
  if (normalnum) {
    fprintf(fp, "normal\n");
    i = 1;
    for (s = ppd->spso; s != (Spso *) NULL; s = s->nxt) {
      for (nm = ppd->spnm; nm != (Spnm *) NULL; nm = nm->nxt) {
	if (nm->bpso == s) {
	  nm->sid = i;
	  fprintf(fp, "\t%d\t%g %g %g\n", i, nm->vec.x, nm->vec.y, nm->vec.z);
	  ++i;
	}
      }
    }
    fprintf(fp, "end\n");
  }
	
  // face
  if (facenum) {
    fprintf(fp, "face\n");
    i = 1;
    for (s = ppd->spso; s != (Spso *) NULL; s = s->nxt) {
      for (p = ppd->sppt; p != (Sppt *) NULL; p = p->nxt) {
	for (f = ppd->spfc; f != (Spfc *) NULL; f = f->nxt) {
	  if ((f->bpso == s) && (f->bppt == p)) {
	    he = f->sphe;
	    n = 0;
						
	    do {
	      if (vertexnum) vtx[n] = he->vt->sid;
	      if (normalnum) nrm[n] = he->nm->sid;
	      ++n;
	    } while ((he = he->nxt) != f->sphe);
						
	    fprintf(fp, "\t%d\t", i);
	    // vertex
	    for (j = 0; j < n; ++j) {
	      fprintf(fp, "%d ", vtx[j]);
	    }
	    // normal
	    if (normalnum) { 
	      fprintf(fp, "\\\n");
	      fprintf(fp, "\t/n\t");
	      for (j = 0; j < n; ++j)
		fprintf(fp, "%d ", nrm[j]);
	    }
	    fprintf(fp, "\n");
	    ++i;
	  }
	}
      }
    }
    fprintf(fp, "end\n");
  }

  //
  // loop
  //
  if ( loopnum ) {
    // header
    fprintf(fp, "header_mdl\nend\n");
    // solid
    fprintf(fp, "solid_mdl\n");
    fprintf(fp, "\t1\t/l 1 %d\n", loopnum);
    fprintf(fp, "end\n");
    fprintf(fp, "line_mdl\n");
    i = 1;
    for ( l = ppd->splp; l != (Splp *) NULL; l = l->nxt ) {
			
      if ( l->type != SHORTESTPATH ) {
	
	if ( l->type == CLOSEDLOOP ) a = i;
	else a = -i;
	
	fprintf(fp, "\t%d\t", a);
	for ( lv = l->splv; lv != NULL; lv = lv->nxt ) {
	  if ( lv->type == LOOPVT_ONBOUND ) {
	    b = - lv->vt->sid;
	  } else {
	    b = lv->vt->sid;
	  }
	  fprintf(fp, "%d ", b);
	}
	fprintf(fp, "\n");
	++i;
      }
			
    }
    fprintf(fp, "end\n");
  }
	
  fclose(fp);
}

//
// 一枚の tfc を ppdspm 用境界つき ppd として出力
//
void write_ppd_tfc( char *fname, TFace *tfc, Sppd *ppd )
{
//    display("write %s\n", fname );
  FILE *fp;
  if ( (fp = fopen(fname, "w")) == NULL )
    {
      return;
    }

  // header
  int solidnum = 1;
  int partnum = 1;
  int vertexnum = 0;
  Spvt *vt;
  for ( vt = ppd->spvt; vt != NULL; vt = vt->nxt )
    {
      if ( vt->tile_id != tfc->no ) continue;
      vt->sid = vertexnum+1;
      ++vertexnum;
    }
  int facenum = 0;
  int fnodenum = 0;
  Spfc *fc;
  for ( fc = ppd->spfc; fc != NULL; fc = fc->nxt )
    {
      if ( fc->tile_id != tfc->no ) continue;
      ++facenum;
      fnodenum += fc->hen;
    }

  // loop
  int loopnum = 0;
  if ( tfc->clp != NULL )
    {
      loopnum = 1;
    }
  else if ( tfc->cyl_lp[0] != NULL )
    {
      loopnum = 2;
    }

  fprintf(fp, "header\n");
  if (solidnum)  fprintf(fp, "\tsolid\t%d\n",  solidnum);
  if (partnum)	 fprintf(fp, "\tpart\t%d\n",   partnum);
  if (vertexnum) fprintf(fp, "\tvertex\t%d\n", vertexnum);
  if (facenum)
    {
      fprintf(fp, "\tface\t%d\n",  facenum);
      fprintf(fp, "\tfnode\t%d\n", fnodenum);
    }
  fprintf(fp, "end\n");
  
  // solid
  fprintf(fp, "solid\n");
  fprintf(fp, "\t1\t");
  fprintf(fp, "/p 1 1 /v 1 %d /f 1 %d\n", vertexnum, facenum );
  fprintf(fp, "end\n");

  // part
  fprintf(fp, "part\n");
  fprintf(fp, "\t1\t");
  fprintf(fp, "/f 1 %d\n", facenum );
  fprintf(fp, "end\n");

  // vertex
  if ( vertexnum ) {
    fprintf(fp, "vertex\n");
    int i = 1;
    for ( vt = ppd->spvt; vt != NULL; vt = vt->nxt ) {
      
      if ( vt->tile_id != tfc->no ) continue;
      
      Vec vec;
      vec.x = vt->vec.x;
      vec.y = vt->vec.y;
      vec.z = vt->vec.z;
      fprintf(fp, "\t%d\t%g %g %g\n", i, vec.x, vec.y, vec.z);
      ++i;
      
    }
    fprintf(fp, "end\n");
  }

  // face
  if ( facenum ) {
    fprintf(fp, "face\n");
    int i = 1;
    for ( fc = ppd->spfc; fc != NULL; fc = fc->nxt ) {

      if ( fc->tile_id != tfc->no ) continue;

      fprintf(fp, "\t%d\t", i);
      Sphe *he = fc->sphe;
      do {
	fprintf(fp, "%d ", he->vt->sid );
      } while ( (he = he->nxt) != fc->sphe );
      fprintf(fp, "\n");
      ++i;
      
    }
    fprintf(fp, "end\n");
  }

  // boundary loop
  if ( loopnum ) {

    // header
    fprintf(fp, "header_mdl\nend\n");
    // solid
    fprintf(fp, "solid_mdl\n");
    fprintf(fp, "\t1\t/l 1 %d\n", loopnum);
    fprintf(fp, "end\n");
    fprintf(fp, "line_mdl\n");

    Splp *lp;
    for ( int i = 0; i < loopnum; ++i )
      {

	if ( loopnum == 1 ) lp = tfc->clp;
	else lp = tfc->cyl_lp[i];
	  
	fprintf(fp, "\t%d\t", i+1);
	for ( Splv *lv = lp->splv; lv != NULL; lv = lv->nxt )
	  {
	    int id = SMDNULL;
	    vt = lv->vt;
	    if ( (vt->sp_type == SP_VERTEX_TEDGE) ||
		 (vt->sp_type == SP_VERTEX_TEDGE_BP) )
	      {
		id = -lv->vt->sid;
	      }
	    else if ( vt->sp_type == SP_VERTEX_TVERTEX )
	      {
		id = lv->vt->sid;
	      }
	  
	    fprintf(fp, "%d ", id);
	  }
	fprintf(fp, "\n");
      }
    
    fprintf(fp, "end\n");
  }
    
  fclose(fp);
  
//    display("end\n");
}

//
// 一枚の tfc を頂点を二次元平面内の座標におきかえた ppd として出力
//
void write_tfc_param_file( char *fname, TFace *tfc, Sppd *ppd )
{
  FILE *fp;
  if ((fp = fopen(fname, "w")) == NULL) {
    return;
  }

  // header
  int solidnum = 1;
  int partnum = 1;
  int vertexnum = 0;
  Spvt *vt;
  for ( vt = ppd->spvt; vt != NULL; vt = vt->nxt ) {
    if ( vt->tile_id != tfc->no ) continue;
    vt->sid = vertexnum+1;
    ++vertexnum;
  }
  int facenum = 0;
  int fnodenum = 0;
  Spfc *fc;
  for ( fc = ppd->spfc; fc != NULL; fc = fc->nxt ) {
    if ( fc->tile_id != tfc->no ) continue;
    ++facenum;
    fnodenum += fc->hen;
  }
  
  fprintf(fp, "header\n");
  if (solidnum)  fprintf(fp, "\tsolid\t%d\n",  solidnum);
  if (partnum)	 fprintf(fp, "\tpart\t%d\n",   partnum);
  if (vertexnum) fprintf(fp, "\tvertex\t%d\n", vertexnum);
  if (facenum)	 {
    fprintf(fp, "\tface\t%d\n",  facenum);
    fprintf(fp, "\tfnode\t%d\n", fnodenum);
  }
  fprintf(fp, "end\n");
  
  // solid
  fprintf(fp, "solid\n");
  fprintf(fp, "\t1\t");
  fprintf(fp, "/p 1 1 /v 1 %d /f 1 %d\n", vertexnum, facenum );
  fprintf(fp, "end\n");

  // part
  fprintf(fp, "part\n");
  fprintf(fp, "\t1\t");
  fprintf(fp, "/f 1 %d\n", facenum );
  fprintf(fp, "end\n");

  // vertex
  if (vertexnum) {
    fprintf(fp, "vertex\n");
    int i = 1;
    for ( vt = ppd->spvt; vt != NULL; vt = vt->nxt ) {
      
      if ( vt->tile_id != tfc->no ) continue;
      
      int id = vt->tvec_id;
      Vec vec;
      vec.x = tfc->tvec[id].uv.x;
      vec.y = tfc->tvec[id].uv.y;
      vec.z = 0.0;
      fprintf(fp, "\t%d\t%g %g %g\n", i, vec.x, vec.y, vec.z);
      ++i;
      
    }
    fprintf(fp, "end\n");
  }

  if (facenum) {
    fprintf(fp, "face\n");
    int i = 1;
    for ( fc = ppd->spfc; fc != NULL; fc = fc->nxt ) {

      if ( fc->tile_id != tfc->no ) continue;

      fprintf(fp, "\t%d\t", i);
      Sphe *he = fc->sphe;
      do {
	fprintf(fp, "%d ", he->vt->sid );
      } while ( (he = he->nxt) != fc->sphe );
      fprintf(fp, "\n");
      ++i;
      
    }
    fprintf(fp, "end\n");
  }
      
  fclose(fp);

}

//
// 一枚の tfc を頂点を二次元平面内の座標におきかえた ps として出力
//
void write_tfc_param_ps_file( char *fname, TFace *tfc, Sppd *ppd )
{
  FILE *fp;
  if ((fp = fopen(fname, "w")) == NULL) {
    return;
  }

  // header
  int solidnum = 1;
  int partnum = 1;
  int vertexnum = 0;
  Spvt *vt;
  for ( vt = ppd->spvt; vt != NULL; vt = vt->nxt ) {
    if ( vt->tile_id != tfc->no ) continue;
    vt->sid = vertexnum+1;
    ++vertexnum;
  }
  int facenum = 0;
  int fnodenum = 0;
  Spfc *fc;
  for ( fc = ppd->spfc; fc != NULL; fc = fc->nxt ) {
    if ( fc->tile_id != tfc->no ) continue;
    ++facenum;
    fnodenum += fc->hen;
  }
  
  fprintf(fp, "header\n");
  if (solidnum)  fprintf(fp, "\tsolid\t%d\n",  solidnum);
  if (partnum)	 fprintf(fp, "\tpart\t%d\n",   partnum);
  if (vertexnum) fprintf(fp, "\tvertex\t%d\n", vertexnum);
  if (facenum)	 {
    fprintf(fp, "\tface\t%d\n",  facenum);
    fprintf(fp, "\tfnode\t%d\n", fnodenum);
  }
  fprintf(fp, "end\n");
  
  // solid
  fprintf(fp, "solid\n");
  fprintf(fp, "\t1\t");
  fprintf(fp, "/p 1 1 /v 1 %d /f 1 %d\n", vertexnum, facenum );
  fprintf(fp, "end\n");

  // part
  fprintf(fp, "part\n");
  fprintf(fp, "\t1\t");
  fprintf(fp, "/f 1 %d\n", facenum );
  fprintf(fp, "end\n");

  // vertex
  if (vertexnum) {
    fprintf(fp, "vertex\n");
    int i = 1;
    for ( vt = ppd->spvt; vt != NULL; vt = vt->nxt ) {
      
      if ( vt->tile_id != tfc->no ) continue;
      
      int id = vt->tvec_id;
      Vec vec;
      vec.x = tfc->tvec[id].uv.x;
      vec.y = tfc->tvec[id].uv.y;
      vec.z = 0.0;
      fprintf(fp, "\t%d\t%g %g %g\n", i, vec.x, vec.y, vec.z);
      ++i;
      
    }
    fprintf(fp, "end\n");
  }

  if (facenum) {
    fprintf(fp, "face\n");
    int i = 1;
    for ( fc = ppd->spfc; fc != NULL; fc = fc->nxt ) {

      if ( fc->tile_id != tfc->no ) continue;

      fprintf(fp, "\t%d\t", i);
      Sphe *he = fc->sphe;
      do {
	fprintf(fp, "%d ", he->vt->sid );
      } while ( (he = he->nxt) != fc->sphe );
      fprintf(fp, "\n");
      ++i;
      
    }
    fprintf(fp, "end\n");
  }
      
  fclose(fp);

}
