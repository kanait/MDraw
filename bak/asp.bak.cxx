//
// asp.cxx
// Shortest path functions (SGraph version)
//
// Copyright (c) 1997-1999 Takashi Kanai; All rights reserved. 
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

#ifdef _WINDOWS  
#include "screen.h"
#include "Bitmap.h"
#endif

#include "ppdvertex.h"
#include "ppdedge.h"
#include "ppdloop.h"
#include "sgraph.h"
#include "sgprique.h"
#include "veclib.h"
#include "mytime.h"
#include "asp.h"

#define NUM_ITER 20

double eprm[20] = {
  1, 0.5, 0.25, 0.125, 0.0625, 
  0.03125, 0.015625, 0.0078125, 0.00390625, 0.00195312, 
  0.000976562, 0.000488281, 0.000244141, 0.00012207, 6.10352e-05, 
  3.05176e-05, 1.52588e-05, 7.62939e-06, 3.8147e-06, 1.90735e-06
}; 
  
//
// STEP0: 
// create initial sgraph from ppd
//
SGraph *initialize_sgraph( Sppd *ppd, double gamma )
{
  int    i;
  SGraph *sg;
  Spvt   *vt;
  Spfc   *fc;
  Sped   *ed;
  Sphe   *he;
  SGvt   *sgvt, **tmp_sgvt, *sv, *ev;
  SGfc   *sgfc, **tmp_sgfc;
  SGed   *sged;

  display("gamma %g\n", gamma );
  sg = create_sgraph();
  sg->ppd = ppd;
	
  // vertices
  // 頂点の格納
  tmp_sgvt = (SGvt **) malloc( ppd->vn * sizeof(SGvt *) );
  for ( i = 0, vt = ppd->spvt; vt != (Spvt *) NULL; vt = vt->nxt, ++i ) {
    vt->sid = i;
    sgvt = create_sgvt( sg );
    sgvt->spvt = vt;
    
    // オリジナルの頂点
    sgvt->type = VERTEX_ORIGINAL;

    // 座標
    sgvt->vec.x = vt->vec.x;
    sgvt->vec.y = vt->vec.y;
    sgvt->vec.z = vt->vec.z;
    
    tmp_sgvt[i] = sgvt;
  }
	
  // faces
  // 面の格納
  tmp_sgfc = (SGfc **) malloc( ppd->fn * sizeof(SGfc *) );
  for ( i = 0, fc = ppd->spfc; fc != (Spfc *) NULL; fc = fc->nxt, ++i ) {
    fc->sid = i;
    sgfc = create_sgfc( sg );
    sgfc->spfc = fc;
    he = fc->sphe;
    do {
      vt   = he->vt;
      sgvt = tmp_sgvt[ vt->sid ];
      (void) create_sgfcvt( sgfc, sgvt );
//        if ( (sgfc->spfc->no == 7328) && (sgvt->no == 30) )
      if ( sgfc->no == 0 )
	display("1 sgvt %d inserted.\n", sgvt->no );
    } while ( (he = he->nxt) != fc->sphe );
    tmp_sgfc[i] = sgfc;
  }

  // エッジの長さを正規化する
  //	sg->sub = normalize_ppdedge( ppd );

  // edges
  // エッジの格納
  for ( ed = ppd->sped; ed != (Sped *) NULL; ed = ed->nxt ) {
    sv = tmp_sgvt[ ed->sv->sid ];
    ev = tmp_sgvt[ ed->ev->sid ];
    sged = CreateSGed( sg, sv, ev, ed );
    sged->type = EDGE_ORIGINAL;
    ed->length = V3DistanceBetween2Points( &(sged->sv->vec), &(sged->ev->vec) );
    sged->length = ed->length;
    sged->eprm = 1.0;
    //	sged->length = ed->length * sg->sub;
    if ( ed->lhe != NULL ) sged->lf = tmp_sgfc[ ed->lhe->bpfc->sid ];
    if ( ed->rhe != NULL ) sged->rf = tmp_sgfc[ ed->rhe->bpfc->sid ];
  }
	
  free( tmp_sgvt );
  free( tmp_sgfc );

  // 付加的なグラフの生成
  // 四番目の引数は ALGORITHM の中でのみ有効なので関係ない
  add_vertices_edges_sgraph( sg, PRIPROCESS, gamma, 0 );
  
	
  return sg;
}

//
// Approximate shortest path routine
//   
Splp *asp( SGraph *sg, Sppd *ppd, int midp )
{
  // time start
#ifdef _WINDOWS
  DWORD start = GetTickCount();
#else  
  time_start();
#endif
  
  int i = 0;
  int iter = NUM_ITER;
  //char string[BUFSIZ];
  SGraph *org_sg = sg, *new_sg = NULL;
  SGlp *org_lp = NULL, *tmp_lp = NULL;
  BOOL isCalculated = FALSE;

#ifdef _WINDOWS  
  // for animation
  ScreenAtr *screen;
  int j = 0;
#endif    

//  G^{0}: sg
//    while ( i < 1 ) {
  while ( (i < iter) && (isCalculated == FALSE) ) {

//     display("i %d\n", i );
    // STEP1: add vetices and edges in G^{i}
    if ( org_sg != sg ) add_vertices_edges_sgraph( org_sg, ALGORITHM, 0, midp );

#ifdef _WINDOWS
    char string[BUFSIZ];
    // drawwindow routine
//     if ( swin->dis3d.spath_anim == SMD_ON ) {
//       screen = &(swin->screenatr);
//       screen->view_sg   = org_sg;
//       screen->view_sglp = org_lp;
//       screen->wnd->RedrawWindow();
//       // for Figure
//       //sprintf( string, "tmp%02d.bmp", j ); ++j;
//       //DibSave( string, screen );
//       // for debugging
//       //AfxMessageBox("Step.");
//     }
#endif
    
    // STEP2: find path in G^{i}
    tmp_lp = ShortestPath_Dijkstra_SGraph( org_sg, org_sg->src, org_sg->dist );
    if ( tmp_lp == NULL ) {
      if ( org_sg != sg ) {
	free_sgraph( org_sg );
	org_sg = NULL;
      }
      break;
    }

#ifdef _WINDOWS
    // drawwindow routine
    if ( swin->dis3d.spath_anim == SMD_ON ) {
      screen = &(swin->screenatr);
      screen->view_sg   = org_sg;
      //screen->view_sglp = org_lp;
      screen->view_sglp = tmp_lp;
      screen->wnd->RedrawWindow();
      // for Figure
      sprintf( string, "tmp%02d.bmp", j ); ++j;
      DibSave( string, screen );
    }
#endif
    
    // STEP3: new graph (new_sg) extraction from the path (tmp_lp)
    new_sg = sglp_to_sgraph( tmp_lp, org_sg );
    display("(a) sgvtn %d\n", new_sg->lp->sglpvtn );

    if ( i > 4 ) {
      //if ( fabs( new_sg->lp->length - org_sg->lp->length ) < 1.0e-05 ) {
      if ( ( fabs( new_sg->lp->length - org_sg->lp->length ) /
	     org_sg->lp->length ) < 1.0e-05 ) {
	isCalculated = TRUE;
      }
    }
    
    
    free_sglp( tmp_lp );
    tmp_lp = NULL;

    // delete old graph and path
    if ( org_sg != sg ) {
      free_sgraph( org_sg );
      org_sg = NULL;
      org_lp = NULL;
    }

    // STEP4 update a graph
    org_sg = new_sg;
    org_lp = new_sg->lp;
    ++i;
    
#ifdef _WINDOWS
    // drawwindow routine
    if ( swin->dis3d.spath_anim == SMD_ON ) {
      screen = &(swin->screenatr);
      screen->view_sg   = org_sg;
      //screen->view_sglp = org_lp;
      screen->view_sglp = org_lp;
      screen->wnd->RedrawWindow();
      // for Figure
      sprintf( string, "tmp%02d.bmp", j ); ++j;
      DibSave( string, screen );
    }
#endif
    
  } // while

  // the number of iteration.
  int num_iter = i;
  
  if ( org_sg == NULL ) return NULL;
#ifdef _WINDOWS
  char string[BUFSIZ];
  // drawwindow routine
  if ( swin->dis3d.spath_anim == SMD_ON ) {
    screen = &(swin->screenatr);
    screen->view_sg   = org_sg;
    screen->view_sglp = org_lp;
    screen->wnd->RedrawWindow();
    // for Figure
    sprintf( string, "tmp%02d.bmp", j ); ++j;
    DibSave( string, screen );
  }
#endif
    
#ifdef _WINDOWS
  DWORD end = GetTickCount();
#else
  double realtime, usertime, systime;
  time_stop_value( &realtime, &usertime, &systime );
#endif  
  
  // create a new shortest-path loop
  display("sgvtn %d\n", org_sg->lp->sglpvtn );
  Splp *lp;
  if ( (lp = sglp_to_splp_new( org_sg->lp, ppd )) == NULL ) return NULL;
  display("lvn %d\n", lp->lvn );

#ifdef _WINDOWS
  lp->usertime = ( (double) ( end - start ) ) / 1000.0;
#else
  lp->realtime = realtime;
  lp->usertime = usertime;
  lp->systime  = systime;
  lp->num_iter = num_iter;
#endif  
  
  free_sgraph( org_sg );
  
#ifdef _WINDOWS
  if ( swin->dis3d.spath_anim == SMD_ON ) {
    screen->view_sg  = NULL;
    screen->view_sglp = NULL;
  }
#endif    
  
  return lp;
}

//
// STEP1:
// add vertices and edges in G
//

#define NUMMIDPOINT 1
//#define SUB_RATIO 0.5

// 付加的なグラフの生成
void add_vertices_edges_sgraph( SGraph *sg, int where, double gamma, int midp )
{
  int     i, j, n;
  int     vcnt, escnt, eocnt, edcnt;
  int     *num_midpoint, sum_midpoint;
  SGvt    *sgvt, *sv, *ev, *sgvt1, *sgvt2, **mid_sgvt;
  SGed    *sged, *next_sged, *sub_sged;
  SGfcvt  *sgfv1, *sgfv2;
  SGfc    *sgfc;
  Vec     *svec, *evec;
  double  s, t;
	
  eocnt = 0; // 条件 1 により加えられたエッジの数
  escnt = 0; // 条件 2 により加えられたエッジの数
  edcnt = 0; // 消去されたエッジの数
	
  // 追加する中点の数をかぞえる
  // ここを修正
  sum_midpoint = 0;
  num_midpoint = (int *) malloc( sg->sgedn * sizeof(int) );
  for ( j = 0, sged = sg->sged; sged != NULL; sged = sged->nxt, ++j ) {


    if ( sged->type == EDGE_ORIGINAL ) {
      
      // add vertices
      if ( where == ALGORITHM ) {

	if ( sged->isPathEdge != TRUE ) {
	  //num_midpoint[j] = NUMMIDPOINT;
	  //num_midpoint[j] = sged_num_midpoint( sged, sg );
	  //ここの数を変化
        	  num_midpoint[j] = midp;
//            	  num_midpoint[j] = 1;
//            	  num_midpoint[j] = 3;
//           	  num_midpoint[j] = 5;
//           	  num_midpoint[j] = 7;
//        	  num_midpoint[j] = 10;
	} else {
	  num_midpoint[j] = 0;
	}
	//display("edge %d num %d\n", j, num_midpoint[j]);
	
      } else {
	// where == PREPROCESS
	num_midpoint[j] = (int) ( sged->length / gamma );
      }
      sum_midpoint += num_midpoint[j];
      
      } else {

      num_midpoint[j] = 0;

    }
    
  }

  // Steiner Point の平均値
  sg->avrsp = (double) sum_midpoint / (double) sg->sgedn;
    
  //
  // STEP1-1:
  // add n vertices on the edge of M
  // 
  
  // オリジナルのエッジ (EDGE_ORIGINAL) に頂点を付加する
  vcnt = 0;
  mid_sgvt = (SGvt **) malloc( sum_midpoint * sizeof(SGvt *) );
  for ( j = 0, sged = sg->sged; sged != NULL; sged = sged->nxt, ++j ) {
		
    sged->sid = j;
    svec = &(sged->sv->vec);
    evec = &(sged->ev->vec);
		
    if ( sged->type == EDGE_ORIGINAL ) {

      sged->sgvt_id = vcnt;
      
      for ( i = 0; i < num_midpoint[j]; ++i ) {
				
	sgvt = create_sgvt( sg );

	// 新しく生成した頂点の座標値を求める
	s = (double) (i + 1) / (double) ( num_midpoint[j] + 1 );
	t = 1 - s;
	sgvt->vec.x = t * svec->x + s * evec->x;
	sgvt->vec.y = t * svec->y + s * evec->y;
	sgvt->vec.z = t * svec->z + s * evec->z;

	// 新しく加えた頂点であることのフラッグ
	sgvt->type = VERTEX_ADDED;

	// エッジへのポインタ
	sgvt->sped = sged->sped;
				
	mid_sgvt[ sged->sgvt_id + i ] = sgvt; 
	++vcnt;

	// 面から頂点へのポインタ
	if ( sged->lf != NULL ) {
	  create_sgfcvt( sged->lf, sgvt );
	  if ( sged->lf->no == 0 )
	    display("2 sgvt %d inserted.\n", sgvt->no );
	}
	  
	if ( sged->rf != NULL ) {
	  create_sgfcvt( sged->rf, sgvt );
	  if ( sged->rf->no == 0 )
	    display("3 sgvt %d inserted.\n", sgvt->no );
	}

	display("(in add_vertices_... #1) sgvt %d sped %d\n", sgvt->no,
		sgvt->sped->no );
	
      }
    }
  }
  
  //
  // STEP1-2: condition 2
  // 

  // オリジナルのエッジを中間点にしたがって分割
  n = sg->sgedn; sged = sg->sged;
  j = 0;
  while ( j < n ) {
		
    next_sged = sged->nxt;

    // 新しく加えられたエッジ (EDGE_ONFACE) に関しては処理を行わない
    if ( sged->type != EDGE_ORIGINAL ) {
      sged = next_sged; ++j;
      continue;
    }
		
    // 分割エッジの生成 開始

    // display("num_midpoint[%d] %d\n", j, num_midpoint[j] );
    
    if ( num_midpoint[j] ) {
      sv = sged->sv;
      for ( i = 0; i < num_midpoint[j]; ++i ) {
	ev = mid_sgvt[ sged->sgvt_id + i ];
			
	sub_sged = CreateSGed( sg, sv, ev, sged->sped ); ++escnt;
	sub_sged->type = EDGE_ORIGINAL;
	sub_sged->lf = sged->lf;
	sub_sged->rf = sged->rf;
	//sub_sged->length = sged->length / (double) (num_midpoint[j] + 1);
	sub_sged->eprm = sged->eprm / (double) (num_midpoint[j] + 1);
//  	display("(in add_vertices_... #1) sged %d type %d\n",
//  		sub_sged->no, sub_sged->type );
	
	
	sv = ev;
      }
      ev = sged->ev;
      sub_sged = CreateSGed( sg, sv, ev, sged->sped ); ++escnt;
      sub_sged->type = EDGE_ORIGINAL;
      sub_sged->lf = sged->lf;
      sub_sged->rf = sged->rf;
      // sub_sged->length = sged->length / (double) (num_midpoint[j] + 1);
      sub_sged->eprm = sged->eprm / (double) (num_midpoint[j] + 1);
//        display("(in add_vertices_... #2) sged %d type %d\n",
//  	      sub_sged->no, sub_sged->type );
    
      // 分割エッジの生成 終了
    
      // エッジが分割された場合にのみ，オリジナルのエッジは消去する
      FreeSGed( sged, sg );
      ++edcnt;
    }
		
    ++j;
    sged = next_sged;
		
  }
	
  //
  // STEP1-2: condition 1
  //

  // sgvt->sped != NULL の場合は面のエッジ上の中間点
  // sgvt->sped == NULL の場合は面の頂点
  
  for ( sgfc = sg->sgfc; sgfc != NULL; sgfc = sgfc->nxt ) {

//      if ( sgfc->no == 0 ) {
    display("(a) sgfc %d fcvtn %d\n", sgfc->no, sgfc->sgfcvtn );
    for ( sgfv1 = sgfc->sfcvt; sgfv1 != NULL; sgfv1 = sgfv1->nxt ) {
      display("\tsgvt %d\n", sgfv1->vt->no );
    }
      //      }
    for ( sgfv1 = sgfc->sfcvt; sgfv1 != NULL; sgfv1 = sgfv1->nxt ) {
      
      sgvt1 = sgfv1->vt;
      
      for ( sgfv2 = sgfv1->nxt; sgfv2 != NULL; sgfv2 = sgfv2->nxt ) {
	
	sgvt2 = sgfv2->vt;

	display("sgvt1 %d sgvt2 %d\n", sgvt1->no, sgvt2->no );
	
	//if ( (sgvt1->sped == NULL) && (sgvt2-> == NULL) ) continue;

//  	display("vt1 sped1 %d vt2 sped2 %d\n",
//  		( sgvt1->sped != NULL ) ? sgvt1->sped->no : SMDNULL,
//  		( sgvt2->sped != NULL ) ? sgvt2->sped->no : SMDNULL );

	if ( ( isOnDifferentEdges( sgvt1, sgvt2 ) == TRUE ) &&
	     ( find_sged( sgvt1, sgvt2 ) == NULL ) ) {
	  sged = CreateSGed( sg, sgvt1, sgvt2, NULL ); ++eocnt;
	  sged->type = EDGE_ONFACE;
	  sged->spfc = sgfc->spfc;
	  sged->length = V3DistanceBetween2Points( &(sged->sv->vec),
						   &(sged->ev->vec) );
	  display("(in add_vertices_... #3) sged %d sv %d ev %d type %d sgfc %d spfc %d\n",
		  sged->no, sged->sv->no, sged->ev->no, sged->type,
		  sgfc->no, sged->spfc->no );
	}
	
      }
      
    }
//      display("\n");
    
  }
	
  free( mid_sgvt );
  free( num_midpoint );
  
}

BOOL isOnDifferentEdges( SGvt *vt1, SGvt *vt2 )
  {
  Sped *ed;
  Spvt *vt;
	
  if ( (vt1->type == VERTEX_ADDED) &&
       (vt2->type == VERTEX_ADDED) ) {
    if ( vt1->sped != vt2->sped ) return TRUE;
  } else if ( (vt1->type == VERTEX_ADDED) &&
	      (vt2->type == VERTEX_ORIGINAL) ) {
    ed = vt1->sped;
    vt = vt2->spvt;
    if ( (ed->sv != vt) && (ed->ev != vt) ) return TRUE;
  } else if ( (vt1->type == VERTEX_ORIGINAL) &&
	      (vt2->type == VERTEX_ADDED) ) {
    ed = vt2->sped;
    vt = vt1->spvt;
    if ( (ed->sv != vt) && (ed->ev != vt) ) return TRUE;
  }
  return FALSE;
}

//
// EDGE_ORIGINALのエッジに対し，発生させる中間点の数を決定
//

int sged_num_midpoint( SGed *sged, SGraph *sg )
{
  SGvt *sv = sged->sv;
  SGvt *ev = sged->ev;

  if ( (sv->path == VERTEX_SPATH) && (ev->path == VERTEX_SPATH) ) {
    // 両端の頂点が経路の頂点のときは分割しない
    
    return 0;
    
  } else if ( (sv->path != VERTEX_SPATH) && (ev->path != VERTEX_SPATH) ) {
    // 両端とも経路の頂点でないときも分割しない
    
    return 0;
    
  } else {
    // どちらか一方が経路の頂点のとき,
    // num = 最低で minsub 個,　最大で maxsub 個 分割する
    
    double minsub = 0.0;
    double maxsub = 5.0;
    
    // 経路の頂点
    SGvt *vt  = ( sv->path == VERTEX_SPATH ) ? sv : ev;

    // 経路
    SGlp *lp = sg->lp;

    //
    // 経路の頂点に隣接する経路のエッジの平均ベクトルと，
    // 対象とするエッジの角度を調べる
    //

    // 経路のエッジの平均ベクトル
    Vec vec, edvec;
    vec.x = vec.y = vec.z = 0.0;
    SGlpvt *lv = find_sglv( lp, vt );
    SGlpvt *plv;
    if ( (lv != NULL) && (lv != lp->slpvt) ) {
      plv = lv->prv;
      vec.x += ( vt->vec.x - plv->vt->vec.x );
      vec.y += ( vt->vec.y - plv->vt->vec.y );
      vec.z += ( vt->vec.z - plv->vt->vec.z );
    }
    if ( (lv != NULL) && (lv != lp->elpvt) ) {
      plv = lv->nxt;
      vec.x += ( plv->vt->vec.x - vt->vec.x );
      vec.y += ( plv->vt->vec.y - vt->vec.y );
      vec.z += ( plv->vt->vec.z - vt->vec.z );
    }
    V3Normalize( &vec );

    // エッジの反対側の頂点
    SGvt *pvt = ( sv->path == VERTEX_SPATH ) ? ev : sv;
    
    // エッジのベクトル
    edvec.x = pvt->vec.x - vt->vec.x;
    edvec.y = pvt->vec.y - vt->vec.y;
    edvec.z = pvt->vec.z - vt->vec.z;
    V3Normalize( &edvec );

    double val = 1.0 - fabs( V3Cosign( &vec, &edvec ) );
    int num = (int) ( val * (maxsub - minsub) );
    return num;
  }
}
		      
// 
// STEP2:
// calculating a shortest-path using Dijkstra's algorithm
//
SGlp *ShortestPath_Dijkstra_SGraph( SGraph *sg, SGvt *src, SGvt *dist )
{
  int      i;
  int      num1, num2;
  SGvt     *vt, *avt, *min_tvt;
  SGlp     *lp;
  SGPQHeap *pqh;
  SGPQCont *pqc;
  double   min_dis, newlen;
  SGvted   *vted;
  SGVtList *vtlist;
	
  // Spvt initialization
	
  for ( i = 0, vt = sg->sgvt; vt != (SGvt *) NULL; vt = vt->nxt, ++i ) {
    vt->sid = i;
    vt->pqc = (SGPQCont *) NULL;
    vt->pq_type = EMPTY;
    if ( vt == src )  vt->pq_type = DELETED;
  }
	
  // initialize priority queue
  pqh    = create_sgpqheap( sg->sgvtn );
  vtlist = (SGVtList *) malloc( sg->sgvtn * sizeof(SGVtList) );
  for ( i = 0; i < sg->sgvtn; ++i ) {
    vtlist[i].prv = NULL;
  }
  vtlist[src->sid].prv = src;
  vtlist[src->sid].dis = 0.0;
  
  for ( vted = src->svted; vted != (SGvted *) NULL; vted = vted->nxt ) {
    avt = another_sgvt( vted->ed, src );
    insert_sgpqcont( sged_length( vted->ed ), pqh, avt, src );
  }

  //
  // vtlist: S から 1 つのエッジで辿れるものしか値が入らない
  //
  for ( i = 0; i < sg->sgvtn - 1; ++i ) {

    // min_tvt S に加える
    min_dis = pqh->pqcont[0].distance;
    min_tvt = pqh->pqcont[0].tgt_vt;
    vtlist[ min_tvt->sid ].prv = pqh->pqcont[0].prv_vt;
    vtlist[ min_tvt->sid ].dis = min_dis;
		
    // min_tvt をヒープから消去
    deletemin_sgpqcont( pqh );
    
    for ( vted = min_tvt->svted; vted != (SGvted *) NULL; vted = vted->nxt ) {
			
      avt    = another_sgvt( vted->ed, min_tvt );
      newlen = min_dis + sged_length( vted->ed );
      pqc    = avt->pqc;
			
      if ( (pqc == (SGPQCont *) NULL) && (avt->pq_type == EMPTY) ) {
				
	insert_sgpqcont( newlen, pqh, avt, min_tvt );
				
      } else if ( pqc != (SGPQCont *) NULL ) {
				
	// updated. 97/08/28. modified from original dijkstra's algorithm.
	if ( fabs( newlen - pqc->distance ) < SMDZEROEPS ) {
	  
	  num1 = sg_stepnumber_to_src( (SGVtList *) vtlist, pqc->prv_vt, src ); 
	  num2 = sg_stepnumber_to_src( (SGVtList *) vtlist, min_tvt, src ); 
					
	  if ( num2 < num1 ) {
						
	    pqc->distance = newlen;
	    pqc->prv_vt   = min_tvt;
						
	    adjust_sgpqheap( pqc->id, pqh );
	  }
					
	} else {
					
	  if ( newlen < pqc->distance ) {
						
	    pqc->distance = newlen;
	    pqc->prv_vt   = min_tvt;
						
	    adjust_sgpqheap( pqc->id, pqh );
						
	  }
	}
				
      }
    }
  }
	
  // VtList to Loop
  lp = sgvtlist_to_sglp( (SGVtList *) vtlist, src, dist );
	
  free_sgpqheap( pqh );
  free( vtlist );
	
  return lp;
}

//
// type によってエッジの長さを場合分け
double sged_length( SGed *ed )
{
  if ( ed->type == EDGE_ORIGINAL ) {
    return ed->eprm * ed->sped->length;
  } else {
    return ed->length;
  }
}
  
		    
int sg_stepnumber_to_src( SGVtList *vtlist, SGvt *start, SGvt *goal )
{
  int cnt = 0;
  SGvt *vt;
	
  vt = start; 
	
  while ( vt != goal ) {
    ++cnt;
    vt = vtlist[vt->sid].prv;
  }
    
  return cnt;
}

//
// vtlist to sglp
//
SGlp *sgvtlist_to_sglp( SGVtList *vtlist, SGvt *src, SGvt *dist )
{
  SGvt *sgvt, *sv, *ev;
  SGlpvt *lv;
  SGlp *lp;
  SGed *ed;
	
  if ( vtlist == (SGVtList *) NULL ) return (SGlp *) NULL;
	
  lp = create_sglp();
	
  // create loop vertices
  // lp includes src and dist.
  sgvt = dist;
  while ( sgvt != src ) {
    lv = create_sglpvt( lp, sgvt );
    //fprintf(stdout, "sgvt %d\n", sgvt->sid );
    // 付け焼刃．
    sgvt = vtlist[ sgvt->sid ].prv;
    if ( sgvt == NULL ) {
      free_sglp( lp );
      return NULL;
    }
  }
	
  // include a last vertex(src)
  lv = create_sglpvt( lp, sgvt );
	
  // create loop edges
  lp->length = 0.0;
  for ( lv = lp->slpvt; lv->nxt != (SGlpvt *) NULL; lv = lv->nxt ) {
    sv = lv->vt;
    ev = lv->nxt->vt;
//     display("sv %d ev %d\n", sv->no, ev->no );
    // 付け焼刃．
    if ( (ed = find_sged( sv, ev )) == NULL ) {
      free_sglp( lp );
      return NULL;
    }
//     if ( ed == NULL ) {
//       SGed *sged = CreateSGed( sg, sv, ev, NULL );
//       sged->type = EDGE_ONFACE;
//     }
    // エッジの生成
    //(void) create_sglped( lp, ed );
    
    ed->isPathEdge = TRUE;
    display("\tsged %d type %d ", ed->no, ed->type );
    if ( ed->type == EDGE_ONFACE )
      display("spfc %d ", ed->spfc->no );
    display("\n");
    lp->length += sged_length( ed );
  }
	
  return lp;
}

//
// STEP3:
// create graph G' from SGlp
// 
// 最短経路からの必要な部分グラフの復元
//
SGraph *sglp_to_sgraph( SGlp *lp, SGraph *sg )
{
  int     i, n;
  SGlpvt  *lv;
  SGraph  *nsg;
  SGvt    *sgvt, *egvt, *new_sv, *new_ev, *sv, *ev;
  SGed    *sged, *new_sged;
  SGfc    *sgfc;
  SGvted  *ve;
  SGlp    *nlp;
  BOOL *sgvtsearch, *sgedsearch, *sgfcsearch;
  SGvt    **new_sgvt;
  SGfc    **new_sgfc;
	
  //initialize search functions
  new_sgvt   = (SGvt **) malloc( sg->sgvtn * sizeof(SGvt *) );
  sgvtsearch = (BOOL *) malloc( sg->sgvtn * sizeof(BOOL) );
  for ( i = 0, sgvt = sg->sgvt; i < sg->sgvtn; ++i, sgvt = sgvt->nxt ) {
    sgvt->sid = i;
    sgvtsearch[i] = FALSE;
    new_sgvt[i] = NULL;
  }
  sgedsearch = (BOOL *) malloc( sg->sgedn * sizeof(BOOL) );
  for ( i = 0, sged = sg->sged; i < sg->sgedn; ++i, sged = sged->nxt ) {
    sged->sid = i;
    sgedsearch[i] = FALSE;
  }
  sgfcsearch = (BOOL *) malloc( sg->sgfcn * sizeof(BOOL) );
  new_sgfc   = (SGfc **) malloc( sg->sgfcn * sizeof(SGfc *) );
  for ( i = 0, sgfc = sg->sgfc; i < sg->sgfcn; ++i, sgfc = sgfc->nxt ) {
    sgfc->sid = i;
    sgfcsearch[i] = FALSE;
    new_sgfc[i] = NULL;
  }
	
  nsg = create_sgraph();
  nsg->ppd = sg->ppd;
  
  nlp = create_sglp();

  //
  // STEP3-1:
  // add vertices, faces to G^{i+1}
  // 

  // 経路の頂点，エッジを新しいグラフに加える
//   prv_sgvt = NULL;
  for ( lv = lp->slpvt; lv != (SGlpvt *) NULL; lv = lv->nxt ) {

    // 経路の頂点を新しいグラフに加える
    sgvt  = lv->vt;
    new_sv = CreateSGvtfromSGvt( nsg, sgvt );

    // added by T.Kanai 99.06.22
    new_sv->path = VERTEX_SPATH;
    
    sgvtsearch[ sgvt->sid ] = TRUE;
    new_sgvt[ sgvt->sid ] = new_sv;
    (void) create_sglpvt( nlp, new_sv );

    // 経路のエッジを新しいグラフに加える
    // added 99.07.08 by T.Kanai
//     if ( prv_sgvt != NULL ) {
//       sged = find_sged( prv_sgvt, sgvt );
//       new_sged = CreateSGedfromSGed( nsg,
// 				     new_sgvt[ prv_sgvt->sid ],
// 				     new_sv,
// 				     sged );
				     
  }
//     prv_sgvt = sgvt;
// }

  // create neighbor vertices
  // 経路の各頂点の1-近傍の頂点をグラフに加える
  for ( lv = lp->slpvt; lv != (SGlpvt *) NULL; lv = lv->nxt )
    {

      // create sgvt from loop vertices
      sgvt  = lv->vt;
      new_sv = new_sgvt[ sgvt->sid ];
		
      for ( ve = sgvt->svted; ve != NULL; ve = ve->nxt )
	{
      
	  sged = ve->ed;
      
	  // 経路の各頂点の1-近傍の頂点を，エッジが EDGE_ORIGINAL
	  // ならばグラフに加える
	  if ( sged->type == EDGE_ORIGINAL ) {
	    egvt = another_sgvt( sged, sgvt );
	    if ( sgvtsearch[ egvt->sid ] == FALSE ) {
	      new_ev = CreateSGvtfromSGvt( nsg, egvt );
	      sgvtsearch[ egvt->sid ] = TRUE;
	      new_sgvt[ egvt->sid ]   = new_ev;
	    } else {
	      new_ev = new_sgvt[ egvt->sid ];
	    }

	// エッジの左端の面をグラフに加える
	if ( sged->lf != (SGfc *) NULL ) {
	  if ( sgfcsearch[ sged->lf->sid ] == FALSE ) {
	    sgfc = create_sgfc( nsg );
	    sgfc->spfc = sged->lf->spfc;
	    sgfcsearch[ sged->lf->sid ] = TRUE;
	    new_sgfc[ sged->lf->sid ] = sgfc;
	  } else {
	    sgfc = new_sgfc[ sged->lf->sid ];
	  }
	  // 面から頂点へのポインタ
	  if ( find_sgvt_from_sgfc( sgfc, new_sv ) == NULL ) {
	    create_sgfcvt( sgfc, new_sv );
	  }
	  if ( find_sgvt_from_sgfc( sgfc, new_ev ) == NULL ) {
	    create_sgfcvt( sgfc, new_ev );
	  }
	}
				
	// エッジの右端の面をグラフに加える
	if ( sged->rf != (SGfc *) NULL ) {
	  if ( sgfcsearch[ sged->rf->sid ] == FALSE ) {
	    sgfc = create_sgfc( nsg );
	    sgfc->spfc = sged->rf->spfc;
	    sgfcsearch[ sged->rf->sid ] = TRUE;
	    new_sgfc[ sged->rf->sid ] = sgfc;
	  } else {
	    sgfc = new_sgfc[ sged->rf->sid ];
	  }
	  if ( find_sgvt_from_sgfc( sgfc, new_sv ) == NULL ) {
	    create_sgfcvt( sgfc, new_sv );
	  }
	    
	  if ( find_sgvt_from_sgfc( sgfc, new_ev ) == NULL ) {
	    create_sgfcvt( sgfc, new_ev );
	  }
	}
      }
    }
  }

  //
  // STEP 3-2:
  // add edges to G^{i+1}
  //
  // エッジを新しいグラフに加える
  // G^{i+1} の頂点の二つのペア <va, vb> のうち，<va, vb> \in G^{i} と
  // なるものを加える

  sgvt = sg->sgvt; n = sg->sgvtn; i = 0;
  while ( i < n ) {
		
    if ( sgvtsearch[ sgvt->sid ] == TRUE ) {

      // create neighbor vertices
      for ( ve = sgvt->svted; ve != (SGvted *) NULL; ve = ve->nxt ) {
	sged = ve->ed;
	sv = sged->sv;
	ev = sged->ev;
	if ( (sgvtsearch[ sv->sid ] == TRUE) &&
	     (sgvtsearch[ ev->sid ] == TRUE) &&
	     (sgedsearch[ sged->sid ] == FALSE) ) {
	  new_sv = new_sgvt[ sv->sid ];
	  new_ev = new_sgvt[ ev->sid ];
	  new_sged = CreateSGedfromSGed( nsg, new_sv, new_ev, sged );
	  new_sged->length = sged_length( sged );
	  // copy type
	  new_sged->type = sged->type;
	  new_sged->isPathEdge = sged->isPathEdge;
	  new_sged->sped = sged->sped;
	  new_sged->spfc = sged->spfc;
	  new_sged->eprm = sged->eprm;
	  if ( sged->lf != NULL ) new_sged->lf = new_sgfc[ sged->lf->sid ];
	  if ( sged->rf != NULL ) new_sged->rf = new_sgfc[ sged->rf->sid ];
	}
	sgedsearch[ sged->sid ] = TRUE;
      }
    }
    sgvt = sgvt->nxt; ++i; 
  }

#if 0  
  //
  // STEP3-3:
  // delete vertices and edges in G'
  //
  sgvt = nsg->sgvt; n = nsg->sgvtn;
  i = 0;
  while ( i < n ) {
    next_sgvt = sgvt->nxt;
		
    if ( sgvt->sgvtedn < 2 ) {
      if ( sgvt->svted != NULL ) {
	sged = sgvt->svted->ed;
	FreeSGed( sged, nsg );
      }
      free_sgvt( sgvt, nsg );
    }
		
    sgvt = next_sgvt;
    ++i;
  }
#endif  

  // test
  SGfc *tmp_sgfc;
  for ( tmp_sgfc = nsg->sgfc; tmp_sgfc != NULL; tmp_sgfc = tmp_sgfc->nxt )
    {
      if ( tmp_sgfc->no == 0 ) {
	display("(b) sgfc %d\n", tmp_sgfc->no );
	for ( SGfcvt *sgfv1 = tmp_sgfc->sfcvt; sgfv1 != NULL; sgfv1 = sgfv1->nxt ) {
	  display("\tsgvt %d\n", sgfv1->vt->no );
	}
      }
    }
	
  free( sgvtsearch );
  free( sgedsearch );
  free( sgfcsearch );
  free( new_sgvt );
  free( new_sgfc );
	
  nlp->length = lp->length;
  nsg->lp = nlp;
  nsg->src  = nlp->elpvt->vt;
  nsg->dist = nlp->slpvt->vt;
	
  return nsg;
}

//
// sgraph loop to ppd loop
//
Splp *sglp_to_splp( SGlp *sglp, Sppd *ppd )
{
  if ( sglp == NULL ) return NULL;

//    display("aa\n");
  Splp *lp = create_splp();
  lp->type  = SHORTESTPATH;

  // start sgvt
  SGlpvt *sglv = sglp->elpvt;
  Splv *newppdlv = create_ppdloopvertex( lp );
  newppdlv->vt = sglv->vt->spvt;
  //++( sglv->vt->spvt->spn );
  
  while ( TRUE ) {
    
    // sged: 最後のものを抽出
    SGlpvt *eglv = sglv->prv;
    SGed *sged = find_sged( sglv->vt, eglv->vt );
    SGvt *sgsv = sglv->vt;
    SGvt *sgev = eglv->vt;

    // create ppd vertex (esgvt)
    Spvt *ppdsv = sgsv->spvt;
    Spvt *ppdev = NULL;
    if ( (ppdev = sgev->spvt) == NULL ) {
      ppdev = create_ppdvertex( ppd );
      ppdev->vec.x = sgev->vec.x;
      ppdev->vec.y = sgev->vec.y;
      ppdev->vec.z = sgev->vec.z;
      ppdev->type = VERTEX_ADDED;
      ppdev->bpso = ppd->spso;
      ppdev->sp_type = SP_VERTEX_STEINER;
      // if sgvt has spvt, sgvt always has sged.
      ppdev->sp_ed  = sgev->sped;
      ppdev->sp_val =
	( V3DistanceBetween2Points( &(ppdev->vec), &(ppdev->sp_ed->sv->vec) )
	  / ppdev->sp_ed->length );
      sgev->spvt = ppdev;
    }

    // create ppd edge
    Sped *ppded = NULL;
    if ( (ppded = find_ppdedge(ppdsv, ppdev)) == NULL ) {
      ppded = create_ppdedge( ppd );
      ppded->sv = ppdsv;
      ppded->ev = ppdev;
      ppded->type = EDGE_ONFACE;
      ppded->sp_fc = sged->spfc;
      ppded->length = V3DistanceBetween2Points( &(ppdsv->vec), &(ppdev->vec) );
      (void) create_vtxed( ppdsv, ppded );
      (void) create_vtxed( ppdev, ppded );
    }
    ppded->sp_type = SP_EDGE_BOUNDARY;
//     display("ed %d sv %d ev %d type %d ",
// 	    ppded->no, ppded->sv->no, ppded->ev->no,
// 	    ppded->type );
//     if ( ppded->type == EDGE_ONFACE ) {
//       display("fc %d\n", ppded->fc->no );
//     } else {
//       display("\n");
//     }
    
    // create ppd loop vertex
    newppdlv = create_ppdloopvertex( lp );
    newppdlv->vt = ppdev;
    //++( ppdev->spn );
    
    if ( eglv == sglp->slpvt ) break;
    sglv = eglv;
  }
//   display("\n");
  lp->length = sglp->length;

//    display("lp vn %d\n", lp->lvn );
  return lp;
}

// loopedge は作らない
Splp *sglp_to_splp_new( SGlp *sglp, Sppd *ppd )
{
  if ( sglp == NULL ) return NULL;
	
  Splp *lp = create_splp();
  lp->type  = SHORTESTPATH;

  //
  // starts from distination vertex
  //
  SGvt *sgsrc  = sglp->elpvt->vt;
  SGvt *sgdist = sglp->slpvt->vt;
  SGvt *sgvt = sgsrc;
  SGed *sged = NULL;
  SGed *next_sged = NULL;
  SGlpvt *lv = sglp->elpvt; 
  SGlpvt *next_lv = NULL;
  // for creating ppd edge
  Spvt *ppdsv = NULL; Spvt *ppdev = NULL;
  Spvt *ppdvt = NULL;
  Splv *ppdlv = NULL;
  Sped *ppded = NULL;
  BOOL isVertexDeleted = FALSE;
  while ( next_lv != sglp->slpvt ) {
    
    isVertexDeleted = FALSE;
    next_lv   = lv->prv;
    next_sged = find_sged( lv->vt, next_lv->vt );

    //　一つの ORIGINAL のエッジが二つに分割されている場合
    //  には一つにまとめる (中間の頂点を生成しない)
    if ( sged != NULL ) {
      if ( ( sged->sped != NULL ) && ( next_sged->sped != NULL ) &&
	   ( sged->sped == next_sged->sped ) ) {
	isVertexDeleted = TRUE;
      }
    }
		
    if ( isVertexDeleted == FALSE ) {
      //
      // ppdloopvertex is created.
      // If sgvt doesn't have a ppdvertex, ppdvertex is also created.
      //
      sgvt = lv->vt;
      if ( sgvt->spvt == NULL ) {
	
	ppdvt = create_ppdvertex( ppd );
	ppdvt->vec.x = sgvt->vec.x;
	ppdvt->vec.y = sgvt->vec.y;
	ppdvt->vec.z = sgvt->vec.z;
	ppdvt->type = VERTEX_ADDED;
	ppdvt->bpso = ppd->spso;
	ppdvt->sp_type = SP_VERTEX_STEINER;
	// if sgvt has spvt, sgvt always has sged.
	ppdvt->sp_ed  = sgvt->sped;
	ppdvt->sp_val = ( V3DistanceBetween2Points( &(ppdvt->vec),
						    &(ppdvt->sp_ed->sv->vec) )
			  / ppdvt->sp_ed->length );
				
      } else {
	
	ppdvt = sgvt->spvt;
	
      }
      
      // create loopvertex
      ppdlv = create_ppdloopvertex( lp );
      ppdlv->vt = ppdvt;
      ppdlv->type = LOOPVT_ONBOUND;
      //++( ppdvt->spn );

      ppdev = ppdvt;
      // create ppdedge
      if ( lv != sglp->elpvt ) {
	if ( (ppded = find_ppdedge( ppdsv, ppdev )) == NULL ) {
//  	  display("(a) create.\n");
	  ppded = create_ppdedge( ppd );
	  ppded->sv = ppdsv;
	  ppded->ev = ppdev;
	  ppded->type = EDGE_ONFACE;
	  ppded->sp_fc = sged->spfc;
	  ppded->length = V3DistanceBetween2Points( &(ppdsv->vec), &(ppdev->vec) );
	  //ppded->fc = next_sged->spfc;
	  (void) create_vtxed( ppdsv, ppded );
	  (void) create_vtxed( ppdev, ppded );
	}
	ppded->sp_type = SP_EDGE_BOUNDARY;
//  	display("ed %d sv %d ev %d type %d ",
//  		ppded->no, ppded->sv->no, ppded->ev->no,
//  		ppded->type );
//  	if ( ppded->type == EDGE_ONFACE ) {
//  	  display("fc %d\n", ppded->fc->no );
//  	} else {
	//  	  display("\n");
//  	}
      }
      ppdsv = ppdev;
      
    }
		
    lv = next_lv;
    sged = next_sged;
		
  }
	
  // include a last vertex (src)
  Spvt  *ppddist = sgdist->spvt;
  ppdvt = ppddist;
  ppdlv = create_ppdloopvertex( lp );
  ppdlv->vt = ppdvt;
  //++( ppdvt->spn );
  //ppdlv->type = LOOPVT_ONBOUND;

  // 始点・終点に属性
  lp->splv->type = LOOPVT_CORNER;
  lp->eplv->type = LOOPVT_CORNER;

  //
  // エッジのないところにエッジを作る
  // 
  // include a last ppdedge
  ppdev = sgdist->spvt;
  if ( (ppded = find_ppdedge( ppdsv, ppdev )) == NULL ) {
//      display("(b)create.\n");
    ppded = create_ppdedge( ppd );
    ppded->sv = ppdsv;
    ppded->ev = ppdev;
    ppded->type = EDGE_ONFACE;
    ppded->sp_fc = sged->spfc;
    ppded->length = V3DistanceBetween2Points( &(ppdsv->vec), &(ppdev->vec) );
    (void) create_vtxed( ppdsv, ppded );
    (void) create_vtxed( ppdev, ppded );
  }
  ppded->sp_type = SP_EDGE_BOUNDARY;

//    display("ed %d sv %d ev %d type %d ",
//  	  ppded->no, ppded->sv->no, ppded->ev->no,
//  	  ppded->type );
//    if ( ppded->type == EDGE_ONFACE ) {
//      display("fc %d\n", ppded->fc->no );
//    } else {
//      display("\n");
//    }
//    display("\n");

  // create ppdedge
  ppdlv = lp->splv;
  while ( ppdlv != lp->eplv ) {
		
    ppdsv = ppdlv->vt;
    ppdev = ppdlv->nxt->vt;
		
    //
    // If there is no ppdedge between two sgvts, ppdedge is also created.
    //
    if ( (ppded = find_ppdedge( ppdsv, ppdev )) == (Sped *) NULL ) {
      //display("(c)create.\n");
      ppded = create_ppdedge( ppd );
      ppded->sv = ppdsv;
      ppded->ev = ppdev;
      ppded->type = EDGE_ONFACE;
      ppded->length = V3DistanceBetween2Points( &(ppdsv->vec), &(ppdev->vec) );
      (void) create_vtxed( ppdsv, ppded );
      (void) create_vtxed( ppdev, ppded );
    }
    ppded->sp_type = SP_EDGE_BOUNDARY;
		
    ppdlv = ppdlv->nxt;
  }
	
  lp->length = sglp->length;

  Sple *ppdle;
  for ( ppdlv = lp->splv; ppdlv != NULL; ppdlv = ppdlv->nxt )
    {
      // create loop edge
      if ( ppdlv->nxt == NULL ) continue;
      
      ppded = find_ppdedge( ppdlv->vt, ppdlv->nxt->vt );
      assert( ppded != NULL );
      ppdle = create_ppdloopedge( lp );
      ppdle->ed = ppded;
    }

  // TEMP
//    for ( ppdle = lp->sple; ppdle != NULL; ppdle = ppdle->nxt ) {
//      ppded = ppdle->ed;
//      display("ed %d sv %d ev %d type %d ",
//  	    ppded->no, ppded->sv->no, ppded->ev->no, ppded->type );
//      if ( ppded->type == EDGE_ORIGINAL ) {
//        display("ed %d\n",
//  	      (ppded->org_ed != NULL) ? ppded->org_ed->no : -1 );
//      } else {
//        display("fc %d\n",
//  	      (ppded->fc != NULL) ? ppded->fc->no : -1 );
//      }
//    }
  return lp;
}

Splp *ShortestPath_Boundary( Sppd *ppd, Spvt *src, Spvt *dist )
{
  Sped *ed;
  Spvt *pvt;
  Splp *newlp, *olp, *newnewlp;
  Splv *lv, *newlv, *slv, *tlv;
  
  if ( (olp = find_ppdloop( ppd, dist )) == (Splp *) NULL )
    return (Splp *) NULL;
  
  if ( (slv = find_ppdloopvertex( olp, dist )) == (Splv *) NULL )
    return (Splp *) NULL;

  // next direction
  lv = slv;
  lv = lv->nxt;
  while (1) {
    if ( lv == (Splv *) NULL ) lv = olp->splv;
    if ( (lv->vt->sp_type == SP_VERTEX_TVERTEX) && (lv->vt != src) )
      break;
    if ( lv->vt == src ) {

      // find
      newlp = create_splp();
      newlp->type  = SHORTESTPATH;
      
      tlv = slv;
      newlv = create_ppdloopvertex( newlp );
      newlv->vt = tlv->vt;
      //++( newlv->vt->spn );
      
      pvt = tlv->vt; tlv = tlv->nxt;
      
      while ( TRUE ) {
	if ( tlv == (Splv *) NULL ) tlv = olp->splv;
	
	newlv = create_ppdloopvertex( newlp );
	newlv->vt = tlv->vt;
	//++( newlv->vt->spn );

	ed = (Sped *) NULL;
	if ( (ed = find_ppdedge( pvt, tlv->vt )) != (Sped *) NULL ) {
	  ed->sp_type = SP_EDGE_BOUNDARY;
	}
	
	if ( tlv->vt == src ) {

	  // a shortest-path is created
	  if ( newlp->splv->vt == src ) {
	    return newlp;
	  } else {
	    newnewlp = reverse_ppdloop( newlp );
	    free_splp( newlp );
	    return newnewlp;
	  }
	  
	} else {
//  	  newlv->vt->sp_type = SP_VERTEX_BOUNDARY;
	}
	
	pvt = tlv->vt; tlv = tlv->nxt;
      }
    }
    lv = lv->nxt;
  }

  // prev direction
  lv = slv;
  lv = lv->prv;
  while (1) {
    if ( lv == (Splv *) NULL ) lv = olp->eplv;
    if ( (lv->vt->sp_type == SP_VERTEX_TVERTEX) && (lv->vt != src) )
      return (Splp *) NULL;
    if ( lv->vt == src ) {

      // find
      newlp = create_splp();
      newlp->type  = SHORTESTPATH;
      
      tlv = slv;
      newlv = create_ppdloopvertex( newlp );
      newlv->vt = tlv->vt;
      //++( newlv->vt->spn );

      pvt = tlv->vt; tlv = tlv->prv; 

      while ( TRUE ) {
	
	if ( tlv == (Splv *) NULL ) tlv = olp->eplv;
	
	newlv = create_ppdloopvertex( newlp );
	newlv->vt = tlv->vt;
	//++( newlv->vt->spn );
	
	ed = (Sped *) NULL;
	if ( (ed = find_ppdedge( pvt, tlv->vt )) != (Sped *) NULL ) {
	  ed->sp_type = SP_EDGE_BOUNDARY;
	}
	
	if ( tlv->vt == src ) {
	  
	  // a shortest-path is created
	  if ( newlp->splv->vt == src ) {
	    return newlp;
	  } else {
	    newnewlp = reverse_ppdloop( newlp );
	    free_splp( newlp );
	    return newnewlp;
	  }
	  
	} else {
//  	  newlv->vt->sp_type = SP_VERTEX_BOUNDARY;
	}
	
	pvt = tlv->vt; tlv = tlv->prv; 
      }
    }
    lv = lv->prv;
  }
  // return (Splp *) NULL;
}	


