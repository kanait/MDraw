//
// edit.cpp
//
// Copyright (c) 1997-1999 Takashi Kanai
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
//

#include "stdafx.h"
#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include <stdio.h>
#include <math.h>
#include <assert.h>

#include "smd.h"
#include "screen.h"
#include "tile.h"
#include "ppdlist.h"
#include "ppdpart.h"
#include "ppdvertex.h"
#include "ppdedge.h"
#include "ppdface.h"
#include "ppdloop.h"
#include "esp.h"
#include "asp.h"
#include "linbcg.h"
#include "spm.h"
#include "floater.h"
#include "uvwsigproc.h"
#include "sgraph.h"

#include "sellist.h"
#include "edit.h"

//
// Esp GUI connected routine
//
BOOL esp_create_spath( ScreenAtr *screen )
{
  Sppd *ppd;
  Spvt *src, *dist;
  
  if ( screen->current_ppd == (Sppd *) NULL ) return FALSE;
  if ( screen->n_sellist != TWO_VERTICES ) {
    return FALSE;
  }

  ppd  = screen->current_ppd;
  src  = screen->sel_first->vt;
  dist = screen->sel_first->nxt->vt;

  if ( (src == NULL) || (dist == NULL) ) return FALSE;
  if ( src == dist ) return FALSE;

  if ( screen->elp != NULL ) {
    free_ppdvertex_splp( screen->elp, ppd );
    free_splp( screen->elp );
    screen->elp = NULL;
  }

  if ( (screen->elp = esp( ppd, src, dist )) == FALSE ) {
    return FALSE;
  }

  return TRUE;
}

//
// Asp GUI connected routine
//
BOOL asp_create_spath_old( ScreenAtr *screen )
{
  if ( screen->current_ppd == (Sppd *) NULL ) return FALSE;
  if ( screen->n_sellist != TWO_VERTICES ) {
    return FALSE;
  }

  Sppd *ppd  = screen->current_ppd;
  Spvt *src  = screen->sel_first->vt;
  Spvt *dist = screen->sel_first->nxt->vt;

  if ( (src == NULL) || (dist == NULL) ) return FALSE;
  if ( src == dist ) return FALSE;

  // previous sp
  if ( screen->alp != NULL ) {
    free_ppdvertex_splp( screen->alp, ppd );
    free_splp( screen->alp );
    screen->alp = NULL;
  }

  // sgraph initialize
  SGraph *sg = ppd->sg;
  if ( sg == NULL ) return FALSE;
  sg->src  = find_sgvt_from_spvt( sg, src );
  sg->dist = find_sgvt_from_spvt( sg, dist );
  if ( (sg->src == NULL) || (sg->dist == NULL) ) return FALSE;

  // approx.
  int midp = 1;
//   int midp = 3;
//   int midp = 5;
//   int midp = 7;
//   int midp = 10;
  if ( (screen->alp = asp( sg, ppd, midp )) == NULL ) return FALSE;

  return TRUE;
}

//
// Asp GUI connected routine (new)
//
BOOL asp_create_spath( ScreenAtr *screen )
{
  if ( screen->current_ppd == (Sppd *) NULL ) return FALSE;
  
  if ( screen->n_sellist < TWO_VERTICES ) {
    return FALSE;
  }

  Sppd *ppd  = screen->current_ppd;

  // previous sp
  if ( screen->alp != NULL ) {
    free_ppdvertex_splp( screen->alp, ppd );
    free_splp( screen->alp );
    screen->alp = NULL;
  }
  
  for ( SelList *sl = screen->sel_first; sl != NULL; sl = sl->nxt ) {

    if ( sl->nxt == NULL ) continue;
    
    Spvt *src = sl->vt;
    Spvt *dist = sl->nxt->vt;

    // sgraph initialize
    SGraph *sg = ppd->sg;
    if ( sg == NULL ) return FALSE;
    sg->src  = find_sgvt_from_spvt( sg, src );
    sg->dist = find_sgvt_from_spvt( sg, dist );
    if ( (sg->src == NULL) || (sg->dist == NULL) ) return FALSE;

    // approx.
    int midp = 1;

    Splp *newlp = NULL;
    if ( ( newlp = asp( sg, ppd, midp )) == NULL ) return FALSE;
    if ( screen->alp == NULL ) {
      screen->alp = create_splp();
      screen->alp->type  = SHORTESTPATH;
    }
    add_splp( screen->alp, newlp );
    free_splp( newlp );

  }

  return TRUE;
}

//
// create tile face (GUI connected routine)
//
BOOL create_tile_face( ScreenAtr *screen )
{
  
  // loop がつながっていないとだめ
  if ( loop_connect_check( screen ) == FALSE ) {
    return FALSE;
  }

  // tface の生成
  Tile *tile = screen->current_tile;
  TFace *tfc = create_tface( tile );
  tfc->n_tedge = screen->n_sellist;
  tfc->ed = (TEdge **) malloc( tfc->n_tedge * sizeof( TEdge *) );
  int i = 0;
  for ( SelList *sl = screen->sel_first; sl != NULL; sl = sl->nxt ) {
    tfc->ed[i] = sl->ted; ++i;
  }
  
  // outer closed loop の生成
  tfc->clp = create_closedloop( screen );
  
  Sppd *ppd = screen->current_ppd;
  // TEMP
  //{
  //  Spvt* tmp_vt = list_ppdvertex( ppd, 10323 );
  //  display("vt %d neighbor edges %d\n", tmp_vt->no, ppdvertex_num_edge( tmp_vt ) );
  //}
  
  // loop の通っている ppd face の分割

  tface_subdivide_ppdface( tfc, tile, ppd );

  display("-->4\n");
  // ppd のパートの生成
  Sppt *pt = create_ppdpart( ppd );
  pt->bpso = ppd->spso;
  tfc->pt = pt;
  
  // face のグループ化 + tvector の生成
  tface_grouping( tfc, ppd );

  display("-->5\n");
  
  // shape preserving mapping
  tfcspm( tfc );

  display("-->6\n");
#if 0
  // relaxation of internal vertices
  if ( swin->param_type != SP_CONVEX )
    {
      tfcparam( tfc, swin->param_type );
    }
#endif

  /////////////////////////////////////////////////////////////////
  //
  // ted->isRecalc = TRUE がついている ted の経路の再計算
  //
  for ( TEdge* ted = tile->sted; ted != NULL; ted = ted->nxt )
    {
      if ( ted->isRecalc == FALSE ) continue;

      // screen1 に情報を蓄える
      ScreenAtr screen1;
      // 必要な初期化
      screen1.current_tile = tile;
      screen1.current_ppd = ppd;
      screen1.alp = NULL;
      tedge_lp_to_screen( ted->lp, &screen1 );
      
      // ted->lp の消去
      delete_tedge_lp( ted->lp, ppd ); ted->lp = NULL;

      // 再計算
      assert( ppd->sg == NULL );
      ppd->sg = initialize_sgraph( ppd, ted->sp_gamma );
      asp_create_spath( &screen1 );
      ted->lp = screen1.alp;

      // spn を１つ追加
      for ( Splv *lv = ted->lp->splv; lv != NULL; lv = lv->nxt )
	{
	  ++(lv->vt->spn);
	  
	  if ( lv->type == LOOPVT_CORNER ) lv->vt->sp_type = SP_VERTEX_TEDGE_BP;
	  else lv->vt->sp_type = SP_VERTEX_TEDGE;

	}

      // free
      free_sgraph( ppd->sg ); ppd->sg = NULL;
      FreeSelectList( &screen1 );
      
    }
  //
  //
  /////////////////////////////////////////////////////////////////

      
#if 0  
  // TEMP
  {
    write_ppd_tfc( "tile.ppd", tfc, ppd );
    write_tfc_param_file("tileprm.ppd", tfc, ppd );
  }
#endif  
  
  return TRUE;
}

void tedge_lp_to_screen( Splp* lp, ScreenAtr* screen )
{
  for ( Splv* lv = lp->splv; lv != NULL; lv = lv->nxt )
    {
      if ( (lv->vt->sp_type == SP_VERTEX_TEDGE_BP) ||
	   (lv->vt->sp_type == SP_VERTEX_TVERTEX) )
	{
	  create_selectlist_vt( screen, lv->vt );
	}
    }
}
  
BOOL loop_connect_check( ScreenAtr *screen )
{
  for ( SelList *sl = screen->sel_first; sl != NULL; sl = sl->nxt ) {
    Splp *lp = sl->ted->lp;
    Splp *nlp = ( sl->nxt != NULL ) ? sl->nxt->ted->lp : screen->sel_first->ted->lp;
    assert( lp != NULL ); assert( lp != NULL );
    if ( ((lp->splv->vt) != (nlp->splv->vt)) &&
	 ((lp->eplv->vt) != (nlp->eplv->vt)) &&
	 ((lp->eplv->vt) != (nlp->splv->vt)) &&
	 ((lp->splv->vt) != (nlp->eplv->vt)) ) {
      return FALSE;
    }
  }
  return TRUE;
}

Splp *create_closedloop( ScreenAtr *screen )
{
  display("in create_closed_loop\n");
  
  Splp *clp = create_splp();
  clp->type = CLOSEDLOOP;

  int i;
  SelList *sl;
  for ( i = 0, sl = screen->sel_first; sl != NULL; sl = sl->nxt, ++i ) {

    Splp *lp = sl->ted->lp;
    Splp *nlp = ( sl->nxt != NULL ) ? sl->nxt->ted->lp : screen->sel_first->ted->lp;

    // decide lp's direction
    BOOL reverse = FALSE;
    if ( lp->splv->vt == nlp->splv->vt ) {
      reverse = TRUE;
    } else if ( lp->splv->vt == nlp->eplv->vt ) {
      reverse = TRUE;
    } else if ( lp->eplv->vt == nlp->splv->vt ) {
      reverse = FALSE;
    } else if ( lp->eplv->vt == nlp->eplv->vt ) {
      reverse = FALSE;
    }

    // make closed loop's vertices
    // loop vertex becomes ORIGINAL
    Splv *lv, *clv;
    if ( reverse == FALSE ) {
      for ( lv = lp->splv; lv != lp->eplv; lv = lv->nxt ) {
	clv = create_ppdloopvertex( clp );
	clv->vt = lv->vt;
	clv->type = ( lv != lp->splv ) ? LOOPVT_ONBOUND : LOOPVT_CORNER;
      }
    } else { // reverse == TRUE
      for ( lv = lp->eplv; lv != lp->splv; lv = lv->prv ) {
	clv = create_ppdloopvertex( clp );
	clv->vt = lv->vt;
	clv->type = ( lv != lp->eplv ) ? LOOPVT_ONBOUND : LOOPVT_CORNER;
      }
    }
    
    for ( Splv *b = lp->splv; b != NULL; b = b->nxt ) {
      display("(open) lv %d vt %d\n", b->no, b->vt->no );
    }
    display("\n");
  }

  for ( Splv *a = clp->splv; a != NULL; a = a->nxt ) {
    display("(closed) lv %d vt %d type %d\n", a->no, a->vt->no, a->type  );
  }
  
  return clp;
}

//
// create tile cylindrical_face (GUI connected routine)
//
BOOL create_tile_cylindrical_face( ScreenAtr *screen )
{
  
  // 2 つの loop がつながっていないとだめ
  int n; // 一つのループに対するエッジの数
  if ( two_loop_connect_check( screen, &n ) == FALSE ) {
    return FALSE;
  }

  // tface の生成
  Tile *tile = screen->current_tile;
  TFace *tfc = create_tface( tile );
  tfc->num_edges_per_loop = n;

  // tedge の生成
  tfc->n_tedge = screen->n_sellist;
  tfc->ed = (TEdge **) malloc( tfc->n_tedge * sizeof( TEdge *) );
  int i = 0;
  for ( SelList *sl = screen->sel_first; sl != NULL; sl = sl->nxt ) {
    tfc->ed[i] = sl->ted; ++i;
  }
  
  // 2 つの outer closed loop の生成
  create_two_closedloop( screen, tfc );

  // loop の通っている ppd face の分割
  Sppd *ppd = screen->current_ppd;
#if 1
  cyl_loop_subdivide_ppdface( tfc, ppd );
#endif

  // ppd のパートの生成
  Sppt *pt = create_ppdpart( ppd );
  pt->bpso = ppd->spso;
  tfc->pt = pt;
  
#if 1  
  // face のグループ化 + tvector の生成
  cyl_tface_grouping( tfc, ppd );

#endif
  
#if 0  
  // shape preserving mapping
  tfcspm( tfc );

  // relaxation of internal vertices
  if ( swin->param_type != SP_CONVEX )
    {
      tfcparam( tfc, swin->param_type );
    }
#endif
  
#if 0
  // TEMP
  {
    write_ppd_tfc( "tile.ppd", tfc, ppd );
    //write_tfc_param_file("tileprm.ppd", tfc, ppd );
  }
#endif  
  
  return TRUE;
}

//
// for cylindrical_face
//
BOOL two_loop_connect_check( ScreenAtr *screen, int* num_edges_per_loop )
{
  // 始点頂点の決定
  Spvt *first_vt = NULL;
  SelList *sl = screen->sel_first;
  SelList *nsl = sl->nxt;
  if ( nsl == NULL ) // エッジが一つしかない
    {
      return FALSE;
    }
  Splp *lp = sl->ted->lp;
  Splp *nlp = nsl->ted->lp;
  if ( (lp->splv->vt == nlp->splv->vt) || (lp->splv->vt == nlp->eplv->vt) )
    {
      first_vt = lp->eplv->vt;
    }
  if ( (lp->eplv->vt == nlp->splv->vt) || (lp->eplv->vt == nlp->eplv->vt) )
    {
      first_vt = lp->splv->vt;
    }
  if ( first_vt == NULL ) return FALSE;

  // 一つ目のループのチェック
  int l1_cnt = 0;
  while ( sl != NULL )
    {
      ++(l1_cnt);
      nsl = sl->nxt; if ( nsl == NULL ) return FALSE;
      lp = sl->ted->lp; nlp = nsl->ted->lp;
      assert( lp != NULL ); assert( nlp != NULL );
      if ( ((lp->splv->vt) != (nlp->splv->vt)) &&
	   ((lp->eplv->vt) != (nlp->eplv->vt)) &&
	   ((lp->eplv->vt) != (nlp->splv->vt)) &&
	   ((lp->splv->vt) != (nlp->eplv->vt)) )
	{
	  return FALSE;
	}
      if ( (first_vt == nlp->splv->vt) || (first_vt == nlp->eplv->vt) )
	{
	  sl = nsl;
	  ++(l1_cnt);
	  break; // 次のループへ
	}
      sl = nsl;
    }

  //
  // 次のループ
  //

  // 始点頂点の決定
  sl = sl->nxt;
  nsl = sl->nxt;
  if ( nsl == NULL ) // エッジが一つしかない
    {
      return FALSE;
    }
  lp = sl->ted->lp; nlp = nsl->ted->lp;
  if ( (lp->splv->vt == nlp->splv->vt) || (lp->splv->vt == nlp->eplv->vt) )
    {
      first_vt = lp->eplv->vt;
    }
  if ( (lp->eplv->vt == nlp->splv->vt) || (lp->eplv->vt == nlp->eplv->vt) )
    {
      first_vt = lp->splv->vt;
    }
  if ( first_vt == NULL ) return FALSE;

  // 二つ目のループのチェック
  int l2_cnt = 0;
  while ( sl != NULL )
    {
      ++(l2_cnt);
      nsl = sl->nxt; if ( nsl == NULL ) return FALSE;
      lp = sl->ted->lp; nlp = nsl->ted->lp;
      assert( lp != NULL ); assert( nlp != NULL );
      if ( ((lp->splv->vt) != (nlp->splv->vt)) &&
	   ((lp->eplv->vt) != (nlp->eplv->vt)) &&
	   ((lp->eplv->vt) != (nlp->splv->vt)) &&
	   ((lp->splv->vt) != (nlp->eplv->vt)) )
	{
	  return FALSE;
	}
      if ( (first_vt == nlp->splv->vt) || (first_vt == nlp->eplv->vt) )
	{
	  ++(l2_cnt);
	  break;
	}
      sl = nsl;
    }

  // エッジの数が違う
  if ( l1_cnt != l2_cnt ) return FALSE;
  *num_edges_per_loop = l1_cnt;

  // 成功！！
  return TRUE;
}

void create_two_closedloop( ScreenAtr *screen, TFace *tfc )
{
  display("in two create_closed_loop\n");

  SelList* sl = screen->sel_first;
  for ( int k = 0; k < 2; ++k )
    {
      // ループの生成
      Splp *clp = tfc->cyl_lp[k] = create_splp();
      clp->type = CLOSEDLOOP;

      // 最初のリスト
      SelList* first_sl = sl;
      
      // closed loop の生成
      for ( int i = 0; i < tfc->num_edges_per_loop; ++i )
	{

	  Splp *lp = sl->ted->lp;
	  Splp *nlp = ( i != tfc->num_edges_per_loop - 1 )
	    ? sl->nxt->ted->lp : first_sl->ted->lp;

	  //
	  // decide lp's direction
	  //
	  BOOL reverse = FALSE;
	  if ( lp->splv->vt == nlp->splv->vt )
	    {
	      reverse = TRUE;
	    }
	  else if ( lp->splv->vt == nlp->eplv->vt )
	    {
	      reverse = TRUE;
	    }
	  else if ( lp->eplv->vt == nlp->splv->vt )
	    {
	      reverse = FALSE;
	    }
	  else if ( lp->eplv->vt == nlp->eplv->vt )
	    {
	      reverse = FALSE;
	    }

	  //
	  // make closed loop's vertices
	  // loop vertex becomes ORIGINAL
	  //
	  Splv *lv, *clv;
	  if ( reverse == FALSE )
	    {
	      for ( lv = lp->splv; lv != lp->eplv; lv = lv->nxt )
		{
		  clv = create_ppdloopvertex( clp );
		  clv->vt = lv->vt;
		  clv->type = ( lv != lp->splv ) ? LOOPVT_ONBOUND : LOOPVT_CORNER;
		}
	    }
	  else // reverse == TRUE
	    { 
	      for ( lv = lp->eplv; lv != lp->splv; lv = lv->prv )
		{
		  clv = create_ppdloopvertex( clp );
		  clv->vt = lv->vt;
		  clv->type = ( lv != lp->eplv ) ? LOOPVT_ONBOUND : LOOPVT_CORNER;
		}
	    }
    
	  sl = sl->nxt;
	  
	  for ( Splv *b = lp->splv; b != NULL; b = b->nxt )
	    {
	      display("(open) lv %d vt %d\n", b->no, b->vt->no );
	    }
	  display("\n");

	}
      
    }

  Splv *a = NULL;
  for ( a = tfc->cyl_lp[0]->splv; a != NULL; a = a->nxt )
    {
      display("(closed 1) lv %d vt %d type %d\n", a->no, a->vt->no, a->type  );
    }
  for ( a = tfc->cyl_lp[1]->splv; a != NULL; a = a->nxt )
    {
      display("(closed 2) lv %d vt %d type %d\n", a->no, a->vt->no, a->type  );
    }
}
