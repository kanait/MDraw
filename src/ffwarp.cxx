//
// fwarp.cxx
// foldover-free mesh warping functions
//
// References:
//   Kikuo Fujimura and Mihail Makarov,
//   "Foldover-Free Image Warping",
//   Graphical Models and Image Processing, Vol.60, No.2, pp.100-111, 1998.
//
// Copyright (c) 2000 Takashi Kanai
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
#include <assert.h>
#include <stdlib.h>
#include "smd.h"
#include "screen.h"

#include "ppdvertex.h"
#include "ppdedge.h"
#include "ppdface.h"
#include "intsec.h"
#include "veclib.h"
#include "ps.h"
#include "linbcg.h"
#include "spm.h"

#include "spedprique.h"
#include "ffwarp.h"

//
// [ワーピングアルゴリズム]
//
// 0. 初期画像に対する与えられた特徴（点，線）を含む三角形化を行なう．
//    線特徴の場合，三角形のエッジがこの特徴と交差してはならない．
//    (ここでは togridppd によって生成されたメッシュを使う)
// 1. イベント (event) をキューに登録する．
//    イベントの検出は解析的に求まる．
// 2  キューの先頭を取り出し，その対象となるエッジの両端の三角形の接続性
//    を変化させる．具体的には，
//    四辺形 imjn のエッジ ij を消去し，nm を追加して再三角形化する
//    (Hoppe の edge swap に相当) (論文の Fig. 6 参照).
// 3. 変形をそのイベントが起こる時間まで進める．
// 4. このエッジの付近のキューに入っているイベントをアップデートする．
// 5. すべての特徴点が最終目的地に達したとき，アルゴリズムを終了．そうで
//    なければ，2. に戻る．

// TEMP
//static Sped *ed86 = NULL;
//static Spfc *fc75 = NULL;

#define NUM 138
#define FFMULTI 0.7
//
// Foldover-free Mesh Warping
//
// ppd: 変形対象のメッシュ
// mesh: 変形の参照メッシュ
//  - 変形の対象になるのは ppd の内部頂点のみ
void ppdffwarp( Sppd *ppd, Sppd *mesh )
{
  //
  // 0. ppd の頂点が mesh のどの面に含まれているかを計算する
  //
  
  PLoc *ploc = ppdpointlocation( ppd, mesh );
  
  // 1. イベント (event) をヒープに登録する．
  // 1.1 mesh の内部頂点の移動ベクトルを求める
  // 1.2 mesh の内部頂点が移動して最初に最初に交差する(このときにevent発生)
  //     mesh のエッジとの距離を求め，この距離をもとに meshheap を作成する
  SpedPQHeap *meshheap = initialize_spedpqheap( mesh );

  // ps 出力 (ベクトルが欲しいとき)
  FILE *fp = open_ps_ppd("spm_lattice.ps", ppd );
  //    pscolor( fp, 0.5, 0.5, 0.5 );
  //    psppdedge( fp, ppd, .5, 1 );
//    pscolor( fp, 1.0, 1.0, 0.0 );
//    pscolor( fp, 0.0, 0.0, 0.0 );
//    psppdedge( fp, mesh, 0.01, 1 );
//    pscolor( fp, 1.0, 0.0, 0.0 );
//    psppdvertex( fp, mesh, 1.0, 2 );

  // 2. キューの先頭を取り出し，その対象となるエッジを挟む四辺形
  //    imjn のエッジ ij を消去し，nm を追加して再三角形化する
  //   (Hoppe の edge swap に相当) (論文の Fig. 6 参照).
  //
  // warping begin
  //
  int i = 0;
  while ( meshheap->last > 0 ) {

    SpedPQCont *pqc = &(meshheap->pqcont[0]);

    display("i = %d vt %d ed %d length %g svec %g %g evec %g %g\n", i,
	    pqc->vt->no,
	    ( pqc->ed != NULL ) ? pqc->ed->no : -1, pqc->length,
	    pqc->vt->uvw.x, pqc->vt->uvw.y, pqc->evec.x, pqc->evec.y );
    
    // edge swap の エッジ
    Sped *ed = pqc->ed;
    Spvt *vt = pqc->vt;
    Spvt *sv = NULL;
    Spvt *ev = NULL;
    if ( ed != NULL ) { // isUpdate = TRUE
      sv = ed->sv;
      ev = ed->ev;
    }
//      Spvt *avt = NULL;
//      if ( ed != NULL ) {
//        avt = another_vt( ed, vt );
//      }
    // start vector, end vector
    Vec2d svec, evec;
    svec.x = vt->uvw.x;
    svec.y = vt->uvw.y;
    evec.x = pqc->evec.x;
    evec.y = pqc->evec.y; 

    // ps 
    {
      if ( i <= NUM ) {
	// 移動量
	pslinewidth( fp, 2.0 );
	pscolor( fp, 0.0, 0.0, 1.0 );
	// ps 出力 (頂点が進む方向のベクトルを書く)
	psdashline( fp, &(vt->uvw), &(pqc->evec) );
      }
    }

    // 今回が変形の最後かどうかを調べる
    // (meshheap のアップデート時に用いる)
    BOOL isUpdate;
    Vec2d intsec;
    if ( ed != NULL ) {
      isUpdate = TRUE; // まだ変形は続く
      // 交点を求める
      // パラメータ
      double param = pqc->length / V2DistanceBetween2Points( &(svec), &(evec) );
      intsec.x = svec.x + param * (evec.x - svec.x);
      intsec.y = svec.y + param * (evec.y - svec.y);
    } else {
      isUpdate = FALSE; // 今回で変形は完了
    }

    {
      // vt->uvw の更新 (交点まで持ってくる)
      if ( isUpdate == TRUE ) {
	vt->uvw.x = intsec.x;
	vt->uvw.y = intsec.y;
      } else {
	vt->uvw.x = evec.x;
	vt->uvw.y = evec.y;
      }
    }

//      display("aa\n");

    //
    // エッジの交換ルーチン, ploc の更新
    //
    {
      if ( isUpdate == TRUE ) {

	// 頂点 vt のまわりの面の面積がすべて non-zero になるまで
	// 続ける
	BOOL isAroundVertex = FALSE;
	while ( isAroundVertex != TRUE ) {

	  Sphe* he = vt->sphe;
	  BOOL isZeroAreaFound = FALSE;
	  do {
	    Spfc* fc = he->bpfc;
	    // 面積が 0 になる
	    if ( fabs( facearea2d( fc ) ) < SMDZEROEPS ) {
	      
	      Spvt *tvt = find_180_vertex( fc, vt );

	      assert( tvt != NULL );
	      
	      // このエッジが swap の対象となる
	      Sped *ted = ppdvertex_1ring_ppdedge( tvt, fc );
	      assert( ted != NULL );

	      Sped *newed = edge_swap( ted, mesh );
	      assert( newed != NULL );

//  	      display("swap ed %d newed %d\n", ted->no, newed->no );
	      //
	      // ploc の更新
	      //
	      // 古い面
	      Spfc *bfc0 = ted->lhe->bpfc;
	      Spfc *bfc1 = ted->rhe->bpfc;
	      // 新しい面
//  	      Spfc *nfc0 = newed->lhe->bpfc;
//  	      Spfc *nfc1 = newed->rhe->bpfc;
	    
	      //
	      // 古いエッジと面の消去
	      //
	      //  	display("\tface %d deleted.\n", bfc0->no );
	      free_ppdface( bfc0, mesh );
	      //  	display("\tface %d deleted.\n", bfc1->no );
	      free_ppdface( bfc1, mesh );
	      //  	display("\tedge %d deleted.\n", ed->no );
	      free_ppdedge( ted, mesh );

	      isZeroAreaFound = TRUE;
	      break;
	    }
	    he = ppdvertex_ccwnext_halfedge( he );

	  } while ( (he != vt->sphe) && (he != NULL) );

	  if ( isZeroAreaFound != TRUE ) isAroundVertex = TRUE;
	  
	}
	    
	//
	// エッジの交換ルーチン
	// - いらなくなった面とエッジは，この段階では消さない
	// - newed: 新しく作成したエッジ
	//
//  	Sped *newed = edge_swap( ed, mesh );
//  	assert( newed != NULL );
	
//  	//
//  	// ploc の更新
//  	//
//  	// 古い面
//  	Spfc *bfc0 = ed->lhe->bpfc;
//  	Spfc *bfc1 = ed->rhe->bpfc;
//  	// 新しい面
//  	Spfc *nfc0 = newed->lhe->bpfc;
//  	Spfc *nfc1 = newed->rhe->bpfc;

//  	//
//  	// 古いエッジと面の消去
//  	//
//  //  	display("\tface %d deleted.\n", bfc0->no );
//  	free_ppdface( bfc0, mesh );
//  //  	display("\tface %d deleted.\n", bfc1->no );
//  	free_ppdface( bfc1, mesh );
//  //  	display("\tedge %d deleted.\n", ed->no );
//  	free_ppdedge( ed, mesh );

	ed = NULL;
	pqc->ed = NULL;
	pqc->ivt = NULL;

      }
    }

//      display("bb\n");
//      {
//        // vt->uvw の更新 (交点まで持ってくる)
//        if ( isUpdate == TRUE ) {
//  	vt->uvw.x = intsec.x;
//  	vt->uvw.y = intsec.y;
//        } else {
//  	vt->uvw.x = evec.x;
//  	vt->uvw.y = evec.y;
//        }
//      }
    //
    // warping
    // (upcoming)
    //

    {
      // 更新されたメッシュ
      if ( i == NUM ) {
	write_ppd_uvw( "remeshuvw.ppd", mesh );
	pscolor( fp, 0.0, 0.0, 0.0 );
	psppdedge( fp, mesh, 1.0, 1 );
	pscolor( fp, 1.0, 0.0, 0.0 );
	psppdvertex( fp, mesh, 1.0, 1 );
	close_ps( fp );
      }
    }
    //
    // meshheap のアップデート
    //
    // 動かした頂点 vt の 1-ring 近傍の頂点の隣接面に関する heap をアップデート
    //
    {
      // vt->uvw と pqc->evec が同じならその頂点に関する変形は完了する
      // ので，deletemin をする
      if ( isUpdate == TRUE ) {
	// vt をアップデート
	update_spedpqcont( vt, meshheap );
	// ed が消される場合，ed が持つ頂点は二つともアップデート
	if ( sv != vt ) update_spedpqcont( sv, meshheap );
	if ( ev != vt ) update_spedpqcont( ev, meshheap );
	
	//  	display("\t(update) vt %d pqc id %d ed %d length %g\n", vt->no, vt->pqc->id,
	//  		(vt->pqc->ed != NULL) ? vt->pqc->ed->no: -1,
	//  		vt->pqc->length );
      } else { // FALSE
	deletemin_spedpqcont( meshheap );
      }
      
      
      Sped *e1 = ppdvertex_first_edge( vt );
      Sped *fe1 = e1;
      do {

	// v1 の heap のアップデート
	Spvt *v1 = another_vt( e1, vt );
//    	display("\t\tv1 %d\n", v1->no );
	update_spedpqcont( v1, meshheap );
	
	e1 = ppdvertex_next_edge( e1, vt );
	
      } while ( (e1 != NULL) && (e1 != fe1) );
    }

//      display("cc\n");
    // 更新されたメッシュ
    //      if ( i == NUM ) {
    //        SpedPQCont *tmppqc = &(meshheap->pqcont[0]);
    //        Sped *tmped = tmppqc->ed;
    //        if ( tmped != NULL ) {
    //  	display("tmped %d\n", tmped->no );
    //  	display("aaaaa\n");
    //  	pslinewidth( fp, 2.0 );
    //  	pscolor( fp, 1.0, 0.0, 0.0 );
    //  	psline( fp, &(tmped->sv->uvw), &(tmped->ev->uvw) );
    //        }
    //        close_ps( fp );
    //      }
    
    ++i;
  } // while

//    write_ppd_uvw( "remeshuvw.ppd", mesh );
//    pscolor( fp, 0.0, 0.0, 0.0 );
//    psppdedge( fp, mesh, 0.01, 1 );
//    pscolor( fp, 1.0, 0.0, 0.0 );
//    psppdvertex( fp, mesh, 0.01, 2 );
//    close_ps( fp );
  
  // ps 出力終了
  //close_ps( fp );

  free_spedpqheap( meshheap );
  free( ploc );
}

//
// point-location algorithm between two meshes
//
// 以下の論文の edge-edge intersection の一部を用いる
//
// This algorithm is based on:
// James R. Kent, Wayne E. Carlson and Richard E. Parent,
// "Shape Transformation for Polyhedral Objects", in SIGGRAPH'92
// - 5.2 The Merging Algorithm -
//
// See also:
// 金井崇，鈴木宏正，木村文彦:
// "円盤と位相同型な任意の三角形メッシュ間の合成法",
// 情報処理学会論文誌, Vol.40, No.11, pp.4106-4116, 1999.
// 
PLoc *ppdpointlocation( Sppd *ppd, Sppd *mesh )
{
  //
  // initialize
  //
  
  // 頂点の sid の初期化
  // PLoc の初期化
  int i = 0;
  PLoc *ploc = (PLoc *) malloc( ppd->vn * sizeof(PLoc) );
  Spvt *vt;
  for ( i = 0, vt = ppd->spvt; vt != NULL; ++i, vt = vt->nxt ) {

    // ナンバリング
    vt->sid = i;
    
    ploc[i].vt = vt;
    ploc[i].fc = NULL;
  }

  // エッジの sid の初期化
  // edge_used (そのエッジをすでに使用したかどうか)の判定の初期化
  BOOL *edge_used = (BOOL *) malloc( ppd->en * sizeof(BOOL) );
  Sped *ed;
  for ( i = 0, ed = ppd->sped; ed != NULL; ++i, ed = ed->nxt ) {

    // ナンバリング
    ed->sid = i;
    
    if ( ed->isBoundary == TRUE ) edge_used[i] = TRUE;
    else edge_used[i] = FALSE;
    
  }
  
  // 最初に内部頂点とそこに含まれている面を探す
  Spvt *v1 = NULL; // 内部頂点 (in ppd)
  Spfc *f2 = NULL; // 包含面 (in mesh)
  for ( v1 = ppd->spvt; v1 != NULL; v1 = v1->nxt ) {

    if ( v1->isBoundary == TRUE ) continue;

    if ( (f2 = FindInFace( mesh, &(v1->uvw) )) != NULL )
      break;
    
  }

  // vt の包含面が mfc
  assert( f2 );
  ploc[ v1->sid ].fc = f2;
  //display("vt %d fc %d\n", v1->no, f2->no );

  //
  // アルゴリズムのスタート
  //

  // 待ち行列 (FIFO) を二つ用意
  // edfifo:  ppd  のエッジ (e1) を格納
  // medfifo: mesh のエッジ (e2) を格納
  SpedFIFO *e1fifo = create_spedfifo();
  SpedFIFO *e2fifo = create_spedfifo();
  
  // 始点に接続しているエッジ (e1) を e1fifo にプッシュ，
  // それらのエッジのフラッグに USED をたてる;
  Sped *e1 = NULL;
  {
    e1 = ppdvertex_first_edge( v1 );
    Sped *fe1 = e1;
    do {
      if ( e1->isBoundary != TRUE ) {
	(void) create_spedfifonode( e1, e1fifo );
	//display("e1 %d\n", e1->no );
	edge_used[e1->sid] = TRUE;
      }
      e1 = ppdvertex_next_edge( e1, v1 );
    } while ( (e1 != NULL) && (e1 != fe1) );
  }

  // while ( e1fifo が空でない ) {
  SpedFIFONode *e1node = NULL;
  while ( (e1node = spedfifo_getstartnode( e1fifo )) != NULL ) {

    //display("in while e1 %d\n", e1->no );

    // e1 <- e1fifo の先頭のエッジ;
    e1 = e1node->ed;

    // (add) e1 のうち，両方ともに包含面の決まっている vt に関しては
    //       探索しない
    // (add) e1 のうち，包含面の決まっている vt を探す
    v1 = NULL; f2 = NULL;
    if ( (ploc[e1->sv->sid].fc != NULL) &&
	 (ploc[e1->ev->sid].fc != NULL) ) {
      spedfifo_deletefirstnode( e1fifo );
      continue;
    } else if ( (ploc[e1->sv->sid].fc != NULL) &&
		(ploc[e1->ev->sid].fc == NULL) ) {
      v1 = e1->sv;
      f2 = ploc[e1->sv->sid].fc;
    } else if ( (ploc[e1->sv->sid].fc == NULL) &&
		(ploc[e1->ev->sid].fc != NULL) ) {
      v1 = e1->ev;
      f2 = ploc[e1->ev->sid].fc;
    }
    assert( v1 != NULL );
    assert( f2 != NULL );

    // f2 を構成するエッジ (e2) を e2fifo にプッシュ;
    Sped *e2 = NULL;
    {
      Sphe *h2 = f2->sphe;
      do {
	e2 = h2->ed;
	if ( e2->isBoundary != TRUE ) {
	  create_spedfifonode( e2, e2fifo );
	}
      } while ( (h2 = h2->nxt) != f2->sphe );
    }
    
    // while ( e2fifo が空でない ) {
    SpedFIFONode *e2node = NULL;
    while ( (e2node = spedfifo_getstartnode( e2fifo )) != NULL ) {

      //display("\tin while e2 %d\n", e2->no );
    
      // e2 <- e2fifo の先頭のエッジ;
      e2 = e2node->ed;

      // if ( 交差判定( e1, e2 ) = TRUE ) {
      if ( isLineSegmentCrossing( &(e1->sv->uvw), &(e1->ev->uvw),
				  &(e2->sv->uvw), &(e2->ev->uvw) )
	   == TRUE ) {

	// 交点を求め，双方のエッジの交点リストに加える; (ここではいらない)
	
	// f2 <- e2 に対し f2 の反対側にある面;
	f2 = ppdedge_another_fc( e2, f2 );
	
	// f2 の持つエッジのうち e2 以外を e2fifo にプッシュ;
	{
	  Sphe *h2 = f2->sphe;
	  do {
	    ed = h2->ed;
	    if ( (ed->isBoundary != TRUE) && (ed != e2) ) {
	      create_spedfifonode( ed, e2fifo );
	    }
	  } while ( (h2 = h2->nxt) != f2->sphe );
	}
	
      }
      
      // e2fifo をポップ;
      spedfifo_deletefirstnode( e2fifo );

    }

    // e1fifo をポップ;
    spedfifo_deletefirstnode( e1fifo );
    //display("e1fifo en %d\n", e1fifo->en );

    // (add) e1 の終点
    v1 = another_vt( e1, v1 );

    // (add) f2 が v1 の包含面である
    ploc[v1->sid].fc = f2;

    //display("v1 %03d f2 %d\n", v1->no, f2->no );
    // e1fifo の終点に接続しているエッジ (e1) のうち，NOTUSED
    // のフラッグがたっているエッジ (e1) を e1fifo にプッシュ;
    // 格納したエッジ (e1) のフラッグに USED をたてる;
    {
      ed = ppdvertex_first_edge( v1 );
      Sped *fe1 = ed;
      do {
	if ( (ed->isBoundary != TRUE) && (edge_used[ed->sid] != TRUE) ) {
	  (void) create_spedfifonode( ed, e1fifo );
	  edge_used[ed->sid] = TRUE;
	}
	ed = ppdvertex_next_edge( ed, v1 );
      } while ( (ed != NULL) && (ed != fe1) );
      
    }
  }

  free_spedfifo( e1fifo );
  free_spedfifo( e2fifo );
  
  free( edge_used );

#if 0
  //
  // test
  //
  for ( i = 0; i < ppd->vn; ++i ) {
    vt = ploc[i].vt;
    Spfc *fc = ploc[i].fc;
    double a1, a2, a3;
    ppdface_calc_barycentric_coordinates( fc, &(vt->uvw), &a1, &a2, &a3 );
    //display("vt %03d fc %d bc %g %g %g\n", vt->no, fc->no, a1, a2, a3 );
  }
#endif
 
  return ploc;
  
}

//
// 最初の包含面を見つけるための関数
//
Spfc *FindInFace( Sppd *ppd, Vec2d *vec )
{
  for ( Spfc *fc = ppd->spfc; fc != NULL; fc = fc->nxt ) {
    if ( isParamsInFace( fc, vec->x, vec->y ) == TRUE ) {
      return fc;
    }
  }
  return NULL;
}

//
// イベント (event) をヒープに登録するための関数
// - mesh の内部頂点の移動ベクトルを求める
// - mesh の内部頂点が移動して最初に最初に交差する (このときに event 発生)
//   mesh のエッジとの距離を求め，この距離をもとに meshheap を作成する
//
SpedPQHeap *initialize_spedpqheap( Sppd *ppd )
{
  if ( ppd == NULL ) return NULL;
  
  // 頂点の数だけの heap (配列) を作成する
  SpedPQHeap *heap = create_spedpqheap( ppd->vn );

  srand( 1385499 );

  // 各頂点毎のイベントの検出
  for ( Spvt *vt = ppd->spvt; vt != NULL; vt = vt->nxt ) {

    // 境界頂点は排除
    if ( vt->isBoundary == TRUE ) continue;
    
    // ppd の内部頂点の移動ベクトルを求める
    Vec2d svec, evec;
    // 出発点 (元の座標)
    svec.x = vt->uvw.x;
    svec.y = vt->uvw.y;
    //
    //
    // 終点
    // (とりあえずランダム)
    double randmax = (double) RAND_MAX;
    double drandx = (double) rand() / randmax;
    double drand = (1.0 + 0.3 * drandx);
//      double drandy = (double) rand() / randmax;
    evec.x = vt->uvw.x * drand;
    evec.y = vt->uvw.y * drand;
//      evec.x = vt->uvw.x + 0.6 * ( drandx - 0.5 );
//      evec.y = vt->uvw.y + 0.6 * ( drandy - 0.5 );
    //      evec.x = vt->uvw.x;
    //      evec.y = vt->uvw.y;
    //      display("vt %d svec %g %g evec %g %g\n", vt->no, svec.x, svec.y,
    //  	    evec.x, evec.y );
    //display("vt %d drandx %g drandy %g\n", vt->no, drandx, drandy );
    //      if ( evec.x >= 1.0 ) evec.x = 0.95;
    //      if ( evec.x <= -1.0 ) evec.x = -0.95;
    //      if ( evec.y >= 1.0 ) evec.y = 0.95;
    //      if ( evec.y <= -1.0 ) evec.y = -0.95;

    //
    // ppd の内部頂点が移動して最初に交差する (このときに event 発生)
    // ppd のエッジとの距離を求め，この距離をもとに ppdheap を作成する
    //

#if 0
    // OLD OLD OLD OLD OLD OLD OLD OLD OLD OLD OLD OLD OLD OLD OLD OLD 
    // vt の 1-ring 近傍のエッジで，パラメータ空間上で交差する
    // ed を見つける．
    // つまり，ppd の内部頂点が移動して最初に交差する
    // (このときに event 発生) ppd のエッジを見つける
    // Sped *ed = find_cross_1ring_edge( vt, &(svec), &(evec) );

    double length;      
    if ( ed != NULL ) { // エッジが交差した
      // もとの頂点から，ed と交差する点までの距離を求める
      Vec2d intsec;
      LineIntersectionPoint( &(svec), &(evec),
			     &(ed->sv->uvw), &(ed->ev->uvw),
			     &(intsec) );
      length = V2DistanceBetween2Points( &(svec), &(intsec) );
      //        display("found ed %d svec %g %g evec %g %g length %g\n", ed->no,
      //  	      svec.x, svec.y, evec.x, evec.y, length );
    } else { // 交差しない
      //        display("found ed NULL\n");
      length = V2DistanceBetween2Points( &(svec), &(evec) );
      //        display("found ed NULL svec %g %g evec %g %g length %g\n", 
      //  	      svec.x, svec.y, evec.x, evec.y, length );
    }
#endif

    // NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW NEW
    // vt の 1-ring 近傍の面で，頂点 vt が動くときに面積が 0 になる
    // 時間 t を求め，その中で 0 < t < 1 かつ一番小さいものを求める
    double param;
    Spfc *fc = find_areazero_1ring_face( vt, &(svec), &(evec), &param );

    // length と param は違う
    double length = V2DistanceBetween2Points( &(svec), &(evec) );
    Spvt *ivt = NULL;
    Sped *ed = NULL;
    if ( fc != NULL ) { // 面が見つかった
      //
      // param だけ時計を進めたとき，fc の中で角度が 180°となる頂点を
      // ivt (new), その対角にあるエッジを ed とする
      //
      ivt = find_180_vertex_param( fc, vt, &(svec), &(evec), param );
      assert( ivt != NULL );
      // このエッジが swap の対象となる
      ed = ppdvertex_1ring_ppdedge( ivt, fc );
      assert( ed != NULL );
      length *= param;

      // 1. 境界エッジにあたった時
      // 2. 境界頂点にあたった時
      // 3. length が ε = SMDZEROEPS 以下の場合
      // ぶつからないように移動ベクトルを戻す
      //  (付け焼き刃的)
      if ( (ed->isBoundary == TRUE) || (ivt->isBoundary == TRUE) ||
	   (fabs(length - 0.0) < SMDZEROEPS) ) {
	
	length *= FFMULTI;
	ed = NULL;
	Vec2d vec;
	vec.x = svec.x + param * FFMULTI * (evec.x - svec.x);
	vec.y = svec.y + param * FFMULTI * (evec.y - svec.y);
	evec.x = vec.x;
	evec.y = vec.y;
      }
      
    } else { // vt が svec から evec に移動するまで面積がゼロとなる面がない

      ivt = NULL;
      ed = NULL;
    }

    // もとの関数を修正
    SpedPQCont *pqc = insert_spedpqcont( heap, length, vt );
    // 交差する頂点，エッジをも保存しておく (ない場合は NULL )
    pqc->ed = ed;
    pqc->ivt = ivt;
    // 終点の座標を保存しておく
    pqc->evec.x = evec.x;
    pqc->evec.y = evec.y;
    
    //      display("pqc id %d vt %d ed %d uvw(svec) %g %g evec %g %g length %g\n",
    //  	    pqc->id, pqc->vt->no,
    //  	    ( pqc->ed != NULL ) ? pqc->ed->no : -1,
    //  	    pqc->vt->uvw.x, pqc->vt->uvw.y,
    //  	    pqc->evec.x, pqc->evec.y,
    //  	    pqc->length );
	    
  }
  
  return heap;
}

//
// heap のアップデート
//
void update_spedpqcont( Spvt *vt, SpedPQHeap *heap )
{
  // 内部頂点のみ
  if ( vt->isBoundary == TRUE ) return;
  
  // もうすでに終了してしまった
  if ( vt->pq_type == DELETED ) return;

  SpedPQCont *pqc = vt->pqc;
  assert( pqc != NULL );

  // 登録されている始点，終点
  Vec2d svec, evec;
  svec.x = vt->uvw.x;
  svec.y = vt->uvw.y;
  evec.x = pqc->evec.x;
  evec.y = pqc->evec.y;

//    display("\t(before) vt %d pqc id %d ed %d length %g\n", vt->no, pqc->id,
//  	  (pqc->ed != NULL) ? pqc->ed->no: -1, pqc->length );

#if 0
  Sped *ed = find_cross_1ring_edge( vt, &(svec), &(evec) );
  pqc->ed = ed;
  double length;
  if ( ed != NULL ) { // エッジが交差した
    // もとの頂点から，ed と交差する点までの距離を求める
    Vec2d intsec;
    LineIntersectionPoint( &(svec), &(evec),
			   &(ed->sv->uvw), &(ed->ev->uvw),
			   &(intsec) );
    length = V2DistanceBetween2Points( &(svec), &(intsec) );
  } else { // 交差しないので，終点までの距離を求める
    length = V2DistanceBetween2Points( &(svec), &(evec) );
  }
  pqc->ed = ed;
  pqc->length = length;
#endif

  // エッジのアップデート
  // 長さのアップデート
  double param;
  Spfc *fc = find_areazero_1ring_face( vt, &(svec), &(evec), &param );

  // length と param は違う
  double length = V2DistanceBetween2Points( &(svec), &(evec) );
  Spvt *ivt = NULL;
  Sped *ed = NULL;
  if ( fc != NULL ) { // 面が見つかった
    //
    // param だけ時計を進めたとき，fc の中で角度が 180°となる頂点を
    // ivt (new), その対角にあるエッジを ed とする
    //
    //display("fc %d\n", fc->no );
//      if ( fc->no == 75 ) {
//        display("fc %d e0 %d e1 %d e2 %d v0 %d v1 %d v2 %d\n",
//  	      fc->no,
//  	      fc->sphe->ed->no,
//  	      fc->sphe->nxt->ed->no,
//  	      fc->sphe->nxt->nxt->ed->no,
//  	      fc->sphe->vt->no,
//  	      fc->sphe->nxt->vt->no,
//  	      fc->sphe->nxt->nxt->vt->no );
//      }
    
    ivt = find_180_vertex_param( fc, vt, &(svec), &(evec), param );
    assert( ivt != NULL );
    // このエッジが swap の対象となる
    ed = ppdvertex_1ring_ppdedge( ivt, fc );
    assert( ed != NULL );
    //      display("---1 ed %d\n", ed->no );
    length *= param;

    // 1. 境界エッジにあたった時
    // 2. 境界頂点にあたった時
    // 3. length が ε = SMDZEROEPS 以下の場合
    // ぶつからないように移動ベクトルを戻す
    //  (付け焼き刃的)
//      if ( (ed->isBoundary == TRUE) || (ivt->isBoundary == TRUE) ) {
    if ( (ed->isBoundary == TRUE) || (ivt->isBoundary == TRUE) ||
	 (fabs(length - 0.0) < SMDZEROEPS) ) {
      length *= FFMULTI;
      ed = NULL;
      Vec2d vec;
      vec.x = vt->uvw.x + param * FFMULTI * (pqc->evec.x - vt->uvw.x);
      vec.y = vt->uvw.y + param * FFMULTI * (pqc->evec.y - vt->uvw.y);
      pqc->evec.x = vec.x;
      pqc->evec.y = vec.y;
    }
    
  } else { // vt が svec から evec に移動するまで面積がゼロとなる面がない

    ivt = NULL;
    ed = NULL;
    
  }

  // アップデート
  pqc->ed = ed;
  pqc->ivt = ivt;
  pqc->length = length;

  //    if ( pqc->ed != NULL ) {
  //      display("---2 pqc->ed %d\n", pqc->ed->no );
  //    }

  // heap のアップデート
  int id = vt->pqc->id;
  (void) adjust_spedpqcont( id, heap );
  //    display("id before: %d after %d\n", id, newid );
  pqc = vt->pqc;
//    display("\t(update) vt %d pqc id %d ed %d ivt %d length %g\n", vt->no, pqc->id,
//  	  (pqc->ed != NULL) ? pqc->ed->no: -1,
//  	  (ivt != NULL) ? ivt->no: -1,
//  	  pqc->length );
}

// vt の 1-ring 近傍のパラメータ空間上で交差する ed を見つける
Sped *find_cross_1ring_edge( Spvt *vt, Vec2d *svec, Vec2d *evec )
{
  if ( vt == NULL ) return NULL;

  Sphe* he = vt->sphe;
  do {
    Spfc* fc = he->bpfc;
    Sped *ed = ppdvertex_1ring_ppdedge( vt, fc );
    
    if ( isLineSegmentCrossing( &(ed->sv->uvw), &(ed->ev->uvw),
				svec, evec ) == TRUE ) {
      return ed;
    }
    
    he = ppdvertex_ccwnext_halfedge( he );
  } while ( (he != vt->sphe) && (he != NULL) );
  
  return NULL;
}

//
// 面の二次元面積
//
double facearea2d( Spfc *fc )
{
  assert( fc != NULL );
  Vec2d *a = &(fc->sphe->vt->uvw);
  Vec2d *b = &(fc->sphe->nxt->vt->uvw);
  Vec2d *c = &(fc->sphe->nxt->nxt->vt->uvw);
  return V2TriArea( a, b, c );
}

// 
// vt の 1-ring 近傍の面で，頂点 vt が動くときに面積が 0 になる
// 面を見つける
//
Spfc *find_areazero_1ring_face( Spvt *vt, Vec2d *svec, Vec2d *evec, double *time )
{
  if ( vt == NULL ) return NULL;

  // 最初だから 1 より大きければいい
  *time = 3.0;
  Spfc *tfc = NULL;
  
  Sphe *he = vt->sphe;
  do {
    Spfc* fc = he->bpfc;

    // 三角形 fc の面積が 0 となる t を求める
    Spvt *vb, *vc;
    if ( vt == he->vt ) {
      vb = he->nxt->vt;
      vc = he->nxt->nxt->vt;
    } else if ( vt == he->nxt->vt ) {
      vb = he->nxt->nxt->vt;
      vc = he->vt;
    } else { //  vt == he->nxt->nxt->vt 
      vb = he->vt;
      vc = he->nxt->vt;
    }

    //
    // a: evec - svec, b: vb - svec, c: vc - svec
    //
    Vec2d a, b, c;
    V2Sub( evec, svec, &a );
    V2Sub( &(vb->uvw), svec, &b );
    V2Sub( &(vc->uvw), svec, &c );
    //
    // t を求める
    //
    double t = (c.x * b.y - b.x * c.y) /
      (a.x * b.y - a.y * b.x + a.y * c.x - a.x * c.y);
    // t の範囲を調べる ( 0 < t < 1 の間にある)

    //if ( (t > SMDZEROEPS) && (t < 1.0 - SMDZEROEPS) ) {
    if ( (t > 0.0) && (t < 1.0) ) {
      if ( t < *time ) {
	tfc = fc;
	*time = t;
      }
    }

    he = ppdvertex_ccwnext_halfedge( he );
  } while ( (he != vt->sphe) && (he != NULL) );

  return tfc;
}

//
// fc の中で角度が 180°となる頂点を見つける
//
Spvt *find_180_vertex( Spfc *fc, Spvt *vt )
{
  if ( vt == NULL ) return NULL;
  if ( fc == NULL ) return NULL;
  // TRIANGLE only
  if ( fc->hen != TRIANGLE ) return NULL;
  
  // 移動する頂点(vt) を v0 とする
  // v0: vec[0], v1: vec[1], v2: vec[2]
  Vec2d vec[3];
  Spvt *v0, *v1, *v2;
  vec[0].x = vt->uvw.x;
  vec[0].y = vt->uvw.y;
  Sphe *he = fc->sphe;
  v0 = vt;
  if ( vt == he->vt ) {
    vec[1].x = he->nxt->vt->uvw.x;
    vec[1].y = he->nxt->vt->uvw.y;
    vec[2].x = he->nxt->nxt->vt->uvw.x;
    vec[2].y = he->nxt->nxt->vt->uvw.y;
    v1 = he->nxt->vt;
    v2 = he->nxt->nxt->vt;
  } else if ( vt == he->nxt->vt ) {
    vec[1].x = he->nxt->nxt->vt->uvw.x;
    vec[1].y = he->nxt->nxt->vt->uvw.y;
    vec[2].x = he->vt->uvw.x;
    vec[2].y = he->vt->uvw.y;
    v1 = he->nxt->nxt->vt;
    v2 = he->vt;
  } else { //  vt == he->nxt->nxt->vt 
    vec[1].x = he->vt->uvw.x;
    vec[1].y = he->vt->uvw.y;
    vec[2].x = he->nxt->vt->uvw.x;
    vec[2].y = he->nxt->vt->uvw.y;
    v1 = he->vt;
    v2 = he->nxt->vt;
  }

  // (v1-v0)・(v2-v0) == -1 の頂点を返す
  Vec2d sub1, sub2;
  // v0 をチェック
  V2Sub( &(vec[1]), &(vec[0]), &sub1 ); V2Normalize( &sub1 );
  V2Sub( &(vec[2]), &(vec[0]), &sub2 ); V2Normalize( &sub2 );
  if ( (V2Dot( &sub1, &sub2 ) + 1.0) < SMDZEROEPS ) {
    return v0;
  }
  // v1 をチェック
  V2Sub( &(vec[2]), &(vec[1]), &sub1 ); V2Normalize( &sub1 );
  V2Sub( &(vec[0]), &(vec[1]), &sub2 ); V2Normalize( &sub2 );
  if ( (V2Dot( &sub1, &sub2 ) + 1.0) < SMDZEROEPS ) {
    return v1;
  }
  // v2 をチェック
  V2Sub( &(vec[0]), &(vec[2]), &sub1 ); V2Normalize( &sub1 );
  V2Sub( &(vec[1]), &(vec[2]), &sub2 ); V2Normalize( &sub2 );
  if ( (V2Dot( &sub1, &sub2 ) + 1.0) < SMDZEROEPS ) {
    return v2;
  }
  
  return NULL;
}

//
// param だけ時計を進めたとき，fc の中で角度が 180°となる頂点を見つける
//
Spvt *find_180_vertex_param( Spfc *fc, Spvt *vt, Vec2d *svec, Vec2d *evec,
			     double param )
{
  if ( vt == NULL ) return NULL;
  if ( fc == NULL ) return NULL;
  // TRIANGLE only
  if ( fc->hen != TRIANGLE ) return NULL;
  
  // 移動する頂点(vt) を v0 とする
  // v0: vec[0], v1: vec[1], v2: vec[2]
  Vec2d vec[3];
  Spvt *v0, *v1, *v2;
  // vt が param だけ進んだときのベクトル
  vec[0].x = svec->x + param * ( evec->x - svec->x );
  vec[0].y = svec->y + param * ( evec->y - svec->y );
  Sphe *he = fc->sphe;
  v0 = vt;
  if ( vt == he->vt ) {
    vec[1].x = he->nxt->vt->uvw.x;
    vec[1].y = he->nxt->vt->uvw.y;
    vec[2].x = he->nxt->nxt->vt->uvw.x;
    vec[2].y = he->nxt->nxt->vt->uvw.y;
    v1 = he->nxt->vt;
    v2 = he->nxt->nxt->vt;
  } else if ( vt == he->nxt->vt ) {
    vec[1].x = he->nxt->nxt->vt->uvw.x;
    vec[1].y = he->nxt->nxt->vt->uvw.y;
    vec[2].x = he->vt->uvw.x;
    vec[2].y = he->vt->uvw.y;
    v1 = he->nxt->nxt->vt;
    v2 = he->vt;
  } else { //  vt == he->nxt->nxt->vt 
    vec[1].x = he->vt->uvw.x;
    vec[1].y = he->vt->uvw.y;
    vec[2].x = he->nxt->vt->uvw.x;
    vec[2].y = he->nxt->vt->uvw.y;
    v1 = he->vt;
    v2 = he->nxt->vt;
  }

  // (v1-v0)・(v2-v0) == -1 の頂点を返す
  Vec2d sub1, sub2;
  // v0 をチェック
  V2Sub( &(vec[1]), &(vec[0]), &sub1 ); V2Normalize( &sub1 );
  V2Sub( &(vec[2]), &(vec[0]), &sub2 ); V2Normalize( &sub2 );
  if ( (V2Dot( &sub1, &sub2 ) + 1.0) < SMDZEROEPS ) {
    return v0;
  }
  // v1 をチェック
  V2Sub( &(vec[2]), &(vec[1]), &sub1 ); V2Normalize( &sub1 );
  V2Sub( &(vec[0]), &(vec[1]), &sub2 ); V2Normalize( &sub2 );
  if ( (V2Dot( &sub1, &sub2 ) + 1.0) < SMDZEROEPS ) {
    return v1;
  }
  // v2 をチェック
  V2Sub( &(vec[0]), &(vec[2]), &sub1 ); V2Normalize( &sub1 );
  V2Sub( &(vec[1]), &(vec[2]), &sub2 ); V2Normalize( &sub2 );
  if ( (V2Dot( &sub1, &sub2 ) + 1.0) < SMDZEROEPS ) {
    return v2;
  }
  
  return NULL;
}

//
// エッジ交換操作 (edge swap operation)
//
Sped *edge_swap( Sped *e, Sppd *ppd )
{
  //    display("aa\n");
  if ( e == NULL ) return NULL;
  //    display("bb\n");

  // エッジの両側に面があることが条件
  if ( (e->lhe == NULL) || (e->rhe == NULL) )
    return NULL;
  //    display("cc\n");
  
  Spvt *vi = e->sv;
  Spvt *vj = e->ev;

  int side = 2;
  Spfc *f[2];
  if ( e->lhe != NULL ) f[0] = e->lhe->bpfc;
  else f[0] = NULL;
  if ( e->rhe != NULL ) f[1] = e->rhe->bpfc;
  else f[1] = NULL;
  
  // まず隣接情報を獲得する
  // i = 0: left, i = 1: right
  int  i;
  Spvt *vk[2];
  Sped *ei[2], *ej[2];
  Sphe *hei[2], *hej[2];
  Spfc *fi[2], *fj[2];
  for ( i = 0; i < side; ++i ) {

    if ( f[i] == NULL ) continue;
    
    vk[i] = get_lm_ppdvertex( f[i], vi, vj );
    ei[i] = get_lm_ppdedge( f[i], vi, vk[i] );
    ej[i] = get_lm_ppdedge( f[i], vj, vk[i] );

    hei[i] = NULL;
    hei[i] = get_lm_ppdhalfedge_rev( f[i], ei[i] );
    if ( hei[i] != NULL ) {
      fi[i] = hei[i]->bpfc;
    } else {
      fi[i] = NULL;
    }

    hej[i] = NULL;
    hej[i] = get_lm_ppdhalfedge_rev( f[i], ej[i] );
    if ( hej[i] != NULL ) {
      fj[i] = hej[i]->bpfc;
    } else {
      fj[i] = NULL;
    }
  }

//    display("vi %d vj %d\n", vi->no, vj->no );
//    display("ed %d lf %d rf %d\n", e->no, e->lhe->bpfc->no, e->rhe->bpfc->no );
//    display("f[0] %d e0 %d e1 %d e2 %d v0 %d v1 %d v2 %d\n",
//  	  f[0]->no,
//  	  f[0]->sphe->ed->no,
//  	  f[0]->sphe->nxt->ed->no,
//  	  f[0]->sphe->nxt->nxt->ed->no,
//  	  f[0]->sphe->vt->no,
//  	  f[0]->sphe->nxt->vt->no,
//  	  f[0]->sphe->nxt->nxt->vt->no );
//    display("f[1] %d e0 %d e1 %d e2 %d v0 %d v1 %d v2 %d\n",
//  	  f[1]->no,
//  	  f[1]->sphe->ed->no,
//  	  f[1]->sphe->nxt->ed->no,
//  	  f[1]->sphe->nxt->nxt->ed->no,
//  	  f[1]->sphe->vt->no,
//  	  f[1]->sphe->nxt->vt->no,
//  	  f[1]->sphe->nxt->nxt->vt->no );
//    display("ei[0] %d ei[1] %d\n", ei[0]->no, ei[1]->no );
//    display("ej[0] %d ej[1] %d\n", ej[0]->no, ej[1]->no );
	  
  //
  // topological operations
  //
  // 1. vi->sphe, vk[i]->sphe をいったん切り離す
  // 2. vj の近傍面の he->vt，近傍エッジの ed->sv or ed->ev を
  //    vj から vi にする
  // 3. 取得した隣接情報 (lhe, rhe や mate など) をアップデート
  // 4. vi->sphe, vk[i]->sphe を再接続する

  //
  // 1. vi->sphe, vk[i]->sphe をいったん切り離す
  //
  vi->sphe = NULL;
  vj->sphe = NULL;
  vk[0]->sphe = NULL;
  vk[1]->sphe = NULL;

  //
  // 2. 新しいエッジ，面，ハーフエッジの生成
  //
  // 交換するエッジ
  Sped *ek = create_ppdedge( ppd );
  ek->sv = vk[0];
  ek->ev = vk[1];
  (void) create_vtxed( vk[0], ek );
  (void) create_vtxed( vk[1], ek );
//    display("edge %d created.\n", ek->no );
  // 上の面
  Spfc *fu = create_ppdface( ppd );
//    display("face %d created.\n", fu->no );
  fu->bpso = ppd->spso; fu->bppt = ppd->sppt; 
  Sphe *heu0 = create_ppdhalfedge( fu );
  heu0->vt = vk[1];
  heu0->ed = ek;
  Sphe *heu1 = create_ppdhalfedge( fu );
  heu1->vt = vk[0];
  heu1->ed = ei[0];
  Sphe *heu2 = create_ppdhalfedge( fu );
  heu2->vt = vi;
  heu2->ed = ei[1];
  // 下の面
  Spfc *fd = create_ppdface( ppd );
//    display("face %d created.\n", fd->no );
  // temp
  //if ( fd->no == 75 ) fc75 = fd;
  fd->bpso = ppd->spso; fd->bppt = ppd->sppt; 
  Sphe *hed0 = create_ppdhalfedge( fd );
  hed0->vt = vk[0];
  hed0->ed = ek;
  Sphe *hed1 = create_ppdhalfedge( fd );
  hed1->vt = vk[1];
  hed1->ed = ej[1];
  Sphe *hed2 = create_ppdhalfedge( fd );
  hed2->vt = vj;
  hed2->ed = ej[0];
//    display("fu %d e0 %d e1 %d e2 %d v0 %d v1 %d v2 %d\n",
//  	  fu->no,
//  	  fu->sphe->ed->no,
//  	  fu->sphe->nxt->ed->no,
//  	  fu->sphe->nxt->nxt->ed->no,
//  	  fu->sphe->vt->no,
//  	  fu->sphe->nxt->vt->no,
//  	  fu->sphe->nxt->nxt->vt->no );
//    display("fd %d e0 %d e1 %d e2 %d v0 %d v1 %d v2 %d\n",
//  	  fd->no,
//  	  fd->sphe->ed->no,
//  	  fd->sphe->nxt->ed->no,
//  	  fd->sphe->nxt->nxt->ed->no,
//  	  fd->sphe->vt->no,
//  	  fd->sphe->nxt->vt->no,
//  	  fd->sphe->nxt->nxt->vt->no );
    
  //
  // 3. 取得した隣接情報 (lhe, rhe や mate など) をアップデート
  //
  // [mate]
  // heu0 と hed0
  heu0->mate = hed0;
  hed0->mate = heu0;
  // heu1 と hei[0]
  heu1->mate = hei[0];
  if ( hei[0] != NULL ) hei[0]->mate = heu1;
  // heu2 と hei[1]
  heu2->mate = hei[1];
  if ( hei[1] != NULL ) hei[1]->mate = heu2;
  // hed1 と hej[1]
  hed1->mate = hej[1];
  if ( hej[1] != NULL ) hej[1]->mate = hed1;
  // hed2 と hej[0]
  hed2->mate = hej[0];
  if ( hej[0] != NULL ) hej[0]->mate = hed2;
  //
  // [lhe, rhe]
  // ek
  ek->lhe = hed0; // 必ずこうなる
  ek->rhe = heu0;
  // ei[0] と heu1
  //    display("(before) ei[0] %d lfc %d rfc %d\n",
  //  	  ei[0]->no,
  //  	  (ei[0]->lhe != NULL ) ? ei[0]->lhe->bpfc->no : -1,
  //  	  (ei[0]->rhe != NULL ) ? ei[0]->rhe->bpfc->no : -1 );
  if ( ei[0]->sv == heu1->vt ) ei[0]->lhe = heu1;
  else ei[0]->rhe = heu1;
  //    display("(after) ei[0] %d lfc %d rfc %d\n",
  //  	  ei[0]->no,
  //  	  (ei[0]->lhe != NULL ) ? ei[0]->lhe->bpfc->no : -1,
  //  	  (ei[0]->rhe != NULL ) ? ei[0]->rhe->bpfc->no : -1 );
//    display("ei[0] %d\n", ei[0]->no );
  // ei[1] と heu2
  if ( ei[1]->sv == heu2->vt ) ei[1]->lhe = heu2;
  else ei[1]->rhe = heu2;
//    display("ei[1] %d\n", ei[1]->no );
  // ej[0] と hed2
  if ( ej[0]->sv == hed2->vt ) ej[0]->lhe = hed2;
  else ej[0]->rhe = hed2;
//    display("ej[0] %d\n", ej[0]->no );
  // ej[1] と hed1
  if ( ej[1]->sv == hed1->vt ) ej[1]->lhe = hed1;
  else ej[1]->rhe = hed1;
//    display("ej[1] %d\n", ej[1]->no );

  //
  // 4. v->sphe の付替え，リアレンジ
  //
  vi->sphe = heu2;
  vj->sphe = hed2;
  vk[0]->sphe = hed0;
  vk[1]->sphe = hed1;

  // 頂点が境界頂点のときのみ実行
  reattach_ppdvertex_sphe( vi );
  reattach_ppdvertex_sphe( vj );
  reattach_ppdvertex_sphe( vk[0] );
  reattach_ppdvertex_sphe( vk[1] );

  //
  // エッジと面の消去
  //
  //free_ppdface( f[0], ppd );
  //free_ppdface( f[1], ppd );

  // 新しくできたエッジを返す
  return ek;
}

Spvt *get_lm_ppdvertex( Spfc *f, Spvt *v1, Spvt *v2 )
{
  if ( f == NULL ) return NULL;

  Sphe *he = f->sphe;
  do {
    Spvt *vt = he->vt;
    if ( ( vt != v1 ) && ( vt != v2 ) ) return vt;
  } while ( (he = he->nxt) != f->sphe );
  
  return (Spvt *) NULL;
}

Sped *get_lm_ppdedge( Spfc *f, Spvt *v1, Spvt *v2 )
{
  if ( f == NULL ) return NULL;

  Sphe *he = f->sphe;
  do {
    Sped *e = he->ed;
    if ( ((e->sv == v1) && (e->ev == v2)) ||
	 ((e->ev == v1) && (e->sv == v2)) )
      return e;
  } while ( (he = he->nxt) != f->sphe );
  
  return NULL;
}

Sphe *get_lm_ppdhalfedge_rev( Spfc *fc, Sped *ed )
{
  if ( ed == NULL ) return NULL;
  if ( fc == NULL ) return NULL;

  Sphe *he = fc->sphe;
  do {
    if ( he->ed == ed ) {
      if ( ed->lhe == he ) return ed->rhe;
      if ( ed->rhe == he ) return ed->lhe;
    }
  } while ( (he = he->nxt) != fc->sphe );

  return NULL;
}
      
//
// エッジを格納しておくための待ち行列関数
//

SpedFIFO *create_spedfifo( void )
{
  SpedFIFO *edfifo;

  edfifo = (SpedFIFO *) malloc( sizeof(SpedFIFO) );
  
  edfifo->en = 0;
  edfifo->senode = edfifo->eenode = NULL;

  return edfifo;
}

void free_spedfifo( SpedFIFO *edfifo )
{
  SpedFIFONode *nenode = NULL;
  for ( SpedFIFONode *enode = edfifo->senode; enode != NULL; enode = nenode ) {
    nenode = enode->nxt;
    free_spedfifonode( enode, edfifo );
  }
  free( edfifo );
}

void spedfifo_deletefirstnode( SpedFIFO *edfifo )
{
  free_spedfifonode( edfifo->senode, edfifo );
}

SpedFIFONode *spedfifo_getstartnode( SpedFIFO *edfifo )
{
  return edfifo->senode;
}

SpedFIFONode *create_spedfifonode( Sped *ed, SpedFIFO *edfifo )
{
  SpedFIFONode *enode;

  enode = (SpedFIFONode *) malloc(sizeof(SpedFIFONode));

  enode->nxt = NULL;
  if ( edfifo->senode == NULL ) {
    enode->prv   = NULL;
    edfifo->eenode = edfifo->senode = enode;
  } else {
    enode->prv   = edfifo->eenode;
    edfifo->eenode = edfifo->eenode->nxt = enode;
  }

  enode->ed = ed;
  
  ++( edfifo->en );

  return enode;
}
  
void free_spedfifonode( SpedFIFONode *enode, SpedFIFO *edfifo )
{
  if ( enode == NULL ) return;

  if ( edfifo->senode == enode ) {
    if ( (edfifo->senode = enode->nxt) != NULL )
      enode->nxt->prv = NULL;
    else {
      edfifo->eenode = NULL;
    }
  } else if ( edfifo->eenode == enode ) {
    enode->prv->nxt = NULL;
    edfifo->eenode = enode->prv;
  } else {
    enode->prv->nxt = enode->nxt;
    enode->nxt->prv = enode->prv;
  }

  free( enode );
  --( edfifo->en );
}
  

