//
// mdw.cxx
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

#ifdef WIN32
#include <string.h>
#endif

#include <math.h>
#include <assert.h>
#include "smd.h"
#include "ppd.h"
#include "ppdsolid.h"
#include "ppdpart.h"
#include "ppdvertex.h"
#include "ppdface.h"
#include "ppdedge.h"
#include "ppdnormal.h"
#include "ppdloop.h"
#include "oppd.h"
#include "tile.h"
#include "mdw.h"
#include "file.h"
#include "veclib.h"

////////////////////////////////////////////////
//
// Write
//
////////////////////////////////////////////////

void write_mdw_v10( char *fname, Sppd *ppd, Tile *tile )
{
  FILE *fp;
  if ( (fp = fopen(fname, "w")) == NULL ) {
    return;
  }

  //
  // header
  //

  // ppd
  Spso *so;
  int solidnum = 0;
  for ( so = ppd->spso; so != NULL; so = so->nxt ) {
    ++solidnum;
  }
  Sppt *pt;
  int partnum = 0;
  for ( pt = ppd->sppt; pt != NULL; pt = pt->nxt) {
    ++partnum;
  }
  Spvt *vt;
  int vertexnum = 0;
  for ( vt = ppd->spvt; vt != NULL; vt = vt->nxt ) {
    if ( vt->sp_type == SP_VERTEX_STEINER ) continue;
    ++vertexnum;
  }
  Spfc *fc;
  int facenum = 0; int fnodenum = 0;
  for ( fc = ppd->spfc; fc != NULL; fc = fc->nxt ) {
    ++facenum;
    fnodenum += fc->hen;
  }
  Splp *lp;
  int loopnum = 0;
  for ( lp = ppd->splp; lp != NULL; lp = lp->nxt ) {
    if ( lp->type == SHORTESTPATH ) continue;
    ++loopnum;
  }

  // Tile
  TVertex *tvt;
  int tvtnum = 0;
  for ( tvt = tile->stvt; tvt != NULL; tvt = tvt->nxt ) {
    ++tvtnum;
  }
	
  TFace *tfc;
  int tfcnum = 0; int tfnodenum = 0;
  for ( tfc = tile->stfc; tfc != NULL; tfc = tfc->nxt ) {
    ++tfcnum;
    tfnodenum += tfc->n_tedge;
  }

  TEdge *ted;
  int tednum = 0;
  for ( ted = tile->sted; ted != NULL; ted = ted->nxt ) {
    ++tednum;
  }
	
  fprintf(fp, "#MDW V1.0\n");
  fprintf(fp, "header\n");
  if ( solidnum ) fprintf(fp, "\tsolid\t%d\n",  solidnum);
  if ( partnum ) fprintf(fp, "\tpart\t%d\n",   partnum);
  if ( vertexnum ) fprintf(fp, "\tvertex\t%d\n", vertexnum);
  if ( facenum ) {
    fprintf(fp, "\tface\t%d\n",  facenum);
    fprintf(fp, "\tfnode\t%d\n", fnodenum);
  }
  if ( tvtnum ) fprintf(fp, "\ttvertex\t%d\n", tvtnum );
  if ( tednum ) fprintf(fp, "\ttedge\t%d\n", tednum );
  if ( tfcnum ) {
    fprintf(fp, "\ttface\t%d\n", tfcnum );
    fprintf(fp, "\ttfnode\t%d\n", tfnodenum );
  }
  fprintf(fp, "end\n");

  // solid
  if (solidnum) {
    int bfc = 0; int efc = 0;
    int bpt = 0; int ept = 0;
    int bvt = 0; int evt = 0;
    fprintf(fp, "solid\n");
    int i = 1;
    for ( so = ppd->spso; so != NULL; so = so->nxt ) {
      fprintf(fp, "\t%d\t", i);  // print Id
      if (partnum) {
	bpt = ept + 1;
	for ( pt = ppd->sppt; pt != NULL; pt = pt->nxt ) {
	  if ( pt->bpso == so ) {
	    ++ept;
	  }
	  
	}
	fprintf(fp, "/p %d %d ", bpt, ept);
      }
      if (vertexnum) {
	bvt = evt + 1;
	for ( vt = ppd->spvt; vt != NULL; vt = vt->nxt ) {
	  if ( vt->bpso == so ) {
	    if ( vt->sp_type != SP_VERTEX_STEINER ) {
	      ++evt;
	    }
	  }
	}
	fprintf(fp, "/v %d %d ", bvt, evt);
      }
      if (facenum) {
	bfc = efc + 1;
	for ( pt = ppd->sppt; pt != NULL; pt = pt->nxt ) {
	  for ( fc = ppd->spfc; fc != NULL; fc = fc->nxt ) {
	    if ( (fc->bpso == so) && (fc->bppt == pt) ) {
	      ++efc;
	    }
	  }
	}
	fprintf(fp, "/f %d %d ", bfc, efc);
      }
      fprintf(fp, "\n");
      ++i;
    }
    fprintf(fp, "end\n");
  }

  // part
  if (partnum) {
    int bfc = 0; int efc  = 0;
    fprintf(fp, "part\n");
    int i = 1;
    for ( pt = ppd->sppt; pt != NULL; pt = pt->nxt ) {
      pt->sid = i;
      fprintf(fp, "\t%d\t", i); // print Id
      if (facenum) {
	bfc = efc + 1;
	for ( fc = ppd->spfc; fc != NULL; fc = fc->nxt ) {
	  if ( fc->bppt == pt ) {
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
  if ( vertexnum ) {
    fprintf(fp, "vertex\n");
    int i = 1;
    for ( so = ppd->spso; so != NULL; so = so->nxt ) {
      for ( vt = ppd->spvt; vt != NULL; vt = vt->nxt ) {
	if ( vt->bpso == so ) {
	  
	  if ( vt->sp_type == SP_VERTEX_STEINER ) continue;
	  
	  vt->sid = i;
	  Vec vec;
	  if ( fabs(vt->vec.x) > SMDZEROEPS ) vec.x = vt->vec.x; else vec.x = 0;
	  if ( fabs(vt->vec.y) > SMDZEROEPS ) vec.y = vt->vec.y; else vec.y = 0;
	  if ( fabs(vt->vec.z) > SMDZEROEPS ) vec.z = vt->vec.z; else vec.z = 0;

	  fprintf(fp, "\t%d\t%g %g %g\n", i, vec.x, vec.y, vec.z);
	  ++i;

	}
      }
    }
    fprintf(fp, "end\n");
  }

  // face
  if ( facenum ) {
    fprintf(fp, "face\n");
    int i = 1;
    for ( so = ppd->spso; so != NULL; so = so->nxt ) {
      for ( pt = ppd->sppt; pt != NULL; pt = pt->nxt ) {
	for ( fc = ppd->spfc; fc != NULL; fc = fc->nxt ) {
	  if ( (fc->bpso == so) && (fc->bppt == pt) ) {

	    Sphe *he = fc->sphe;
	    int n = 0;

	    int vtx[128];
	    do {
	      if (vertexnum) vtx[n] = he->vt->sid;
	      ++n;
	    } while ((he = he->nxt) != fc->sphe);
						
	    fprintf(fp, "\t%d\t", i);

	    // vertex
	    for ( int j = 0; j < n; ++j ) {
	      fprintf(fp, "%d ", vtx[j]);
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
  // tiles
  //
  
  if ( tvtnum ) {
    fprintf(fp, "tvertex\n");
    int i = 1;
    for ( tvt = tile->stvt; tvt != NULL; tvt = tvt->nxt ) {
      fprintf( fp, "\t%d\t/v %d\n", i, tvt->vt->sid ); // print Id
      tvt->sid = i;
      ++i;
    }
    fprintf(fp, "end\n");
  }
      
  if ( tednum ) {
    fprintf(fp, "tedge\n");
    int i = 1;
    for ( ted = tile->sted; ted != NULL; ted = ted->nxt ) {

      ted->sid = i;
      Splp *lp = ted->lp;
      fprintf( fp, "\t%d\t/v %d %d /l %d /g %g\n",
	       i,
	       ted->sv->sid,
	       ted->ev->sid,
	       lp->lvn, // ループ頂点の数
	       ted->sp_gamma // 経路を求めたときの gamma
	       );

      // loop vertex
      int j = 1;
      for ( Splv *lv = lp->splv; lv != NULL; lv = lv->nxt ) {
	vt = lv->vt;
	fprintf( fp, "\t\t%d\t/v %d ",
		 j, // (ループの中の)頂点番号
		 vt->sid // ppd の頂点の番号
		 );
	fprintf( fp, "/s %d ", lv->type ); // loopvertex の type
	fprintf( fp, "/t %d ", vt->type ); // vertex の type
	if ( (vt->type == VERTEX_ADDED) && (vt->sp_ed != NULL) ) {
	  // sp_type == SP_VERTEX_STEINER のときの
	  // sp_ed の始点，終点
	  fprintf(fp, "/e %d %d ", vt->sp_ed->sv->sid, vt->sp_ed->ev->sid );
	}
	fprintf(fp, "\n");
	++j;
      }
      ++i;
    }
    fprintf(fp, "end\n");
  }
      
  if ( tfcnum ) {
    fprintf(fp, "tface\n");
    int i = 1;
    for ( tfc = tile->stfc; tfc != NULL; tfc = tfc->nxt ) {
      // 対応する part の番号
      int pid = ( tfc->pt != NULL ) ? tfc->pt->sid : SMDNULL;
      fprintf( fp, "\t%d\t/p %d ", i, pid );
      // tvector の数
      if ( tfc->isParameterized == TRUE ) {
	fprintf( fp, "/t %d ", tfc->n_tvector );
      }
      // face loop を構成する一番最初の頂点
      // (あとは tedge の情報を使う)
      int clid = ( tfc->clp != NULL ) ? tfc->clp->splv->vt->sid : SMDNULL;
      fprintf( fp, "/s %d ", clid );

      int j;
      // tedge loop
      fprintf( fp, "/e ");
      for ( j = 0; j < tfc->n_tedge; ++j ) {
	fprintf( fp, "%d ", tfc->ed[j]->sid );
      }
      fprintf( fp, "\n");

      if ( tfc->isParameterized == TRUE ) {
	// tvector
	for ( j = 0; j < tfc->n_tvector; ++j ) {
	  fprintf(fp, "\t\t%d\t/v %d /p %g %g\n",
		  j+1,
		  tfc->tvec[j].vt->sid,
		  tfc->tvec[j].uv.x,
		  tfc->tvec[j].uv.y );
	}
      }
      ++i;
    }
    fprintf(fp, "end\n");
  }
  fclose(fp);
}

Tile *open_mdw( char *fname, BOOL isNormalize )
{
  FILE *fp;
  if ( (fp = fopen(fname, "r")) == NULL ) {
    return NULL;
  }

  int mode = PPD_NONE;
	
  Sppd *ppd  = create_ppd();
  Tile *tile = create_tile();
  tile->ppd = ppd;
	
  //sprintf( ppd->filename, "%s", fname );
	
  Oppd *oppd = create_oppd();
  
  int num, id;
  Oso *oso; Opt *opt; Ovt *ovt; Ofc *ofc;
  Spso *so; Sppt *pt; Spvt *vt; Spfc *fc; Sped *ed;
  TVertex *tvt; TEdge *ted; TFace *tfc;
  char buf[BUFSIZ], key[BUFSIZ], val[4][BUFSIZ];
  while ( fgets(buf, BUFSIZ, fp) ) {

    sscanf( buf, "%s", key );
    
    if ( comment(buf[0]) ) {
      
      continue;
      
    } else if ( !strcmp(key, "end") ) {

      if ( mode == PPD_FACE ) {

	// isolated vertices check.
	ppdvertex_noedge_check( ppd );
  
	//
	// needed for calculating shortest paths
	if ( isNormalize == TRUE ) normalize_ppd( ppd );
	
	//
	// set boundary attributes
	calc_ppd_boundary( ppd );
	
	//
	// re-attaching ppdvertex to halfedge pointer.
	// This function must be executed with ``calc_ppd_boundary''.
	reattach_ppdvertex_halfedge( ppd );
  
	//
	// calc edge length (for shortest path)
	edge_length( ppd );
      }
      
      mode = PPD_NONE;
      
    } else if ( mode == PPD_HEADER ) {	// read "header"
      
      sscanf( buf, "%s%s", key, val[0] );
      num = atoi(val[0]);
      if ( !strcmp(key, "solid") ) {
	oso = create_osolid( num );
	for ( int i = 0; i < num; ++i ) {
	  so = create_ppdsolid( ppd );
	  oso[i].so = so;
	}
      } else if ( !strcmp(key, "part") ) {
	opt = create_opart( num );
	for ( int i = 0; i < num; ++i ) {
	  pt = create_ppdpart( ppd );
	  opt[i].pt = pt;
	}
      } else if ( !strcmp(key, "face") ) {
	ofc = create_oface( num );
	for ( int i = 0; i < num; ++i ) {
	  fc = create_ppdface( ppd );
	  ofc[i].fc = fc;
	}
      } else if ( !strcmp(key, "vertex") ) {
	// used for esp
	ppd->nvt_org = num;
	ovt = create_overtex( num );
	for ( int i = 0; i < num; ++i ) {
	  vt = create_ppdvertex( ppd );
	  ovt[i].vt = vt;
	}
      }
      
    } else if ( mode == PPD_SOLID ) {
      
      char *bpt = buf;
      char stc[BUFSIZ];
      bpt = cut( bpt, stc );
      char fcmode = '\0';
      so = oso[atoi(stc)-1].so;
      BOOL done = FALSE;
      while ( done == FALSE ) {
	bpt = cut(bpt, stc);
	if (stc[0] == '/' && (stc[1] == 'f' || stc[1] == 'n'
			      || stc[1] == 'v' || stc[1] == 'p')) {
	  // /f or /p or /v
	  fcmode = stc[1];
	  bpt = cut(bpt, val[0]); int bn = atoi(val[0])-1;
	  bpt = cut(bpt, val[1]); int en = atoi(val[1])-1;
	  if ( fcmode == 'f' ) {
	    for ( int i = bn; i <= en; ++i ) {
	      fc = ofc[i].fc;
	      fc->bpso = so;
	    }
	  } else if ( fcmode == 'v' ) {
	    for ( int i = bn; i <= en; ++i ) {
	      vt = ovt[i].vt;
	      vt->bpso = so;
	    }
	  } else if ( fcmode == 'p' ) {
	    for ( int i = bn; i <= en; ++i ) {
	      pt = opt[i].pt;
	      pt->bpso = so;
	    }
	  }
	}
	if (!bpt)  done = TRUE;
      }
      
    } else if ( mode == PPD_PART ) {
      
      char *bpt = buf;
      char stc[BUFSIZ];
      bpt = cut( bpt, stc );
      char fcmode = '\0';
      pt = opt[atoi(stc)-1].pt;
      BOOL done = FALSE;
      while ( done == FALSE ) {
	if ( bpt ) {
	  bpt = cut( bpt, stc );
	  if (stc[0] == '/' && (stc[1] == 'f' || stc[1] == 'b')) {
	    // /f or /b
	    fcmode = stc[1];
	    bpt = cut(bpt, val[0]); int bn = atoi(val[0]) - 1;
	    bpt = cut(bpt, val[1]); int en = atoi(val[1]) - 1;
	    for ( int i = bn; i <= en; ++i ) {
	      fc = ofc[i].fc;
	      fc->bppt = pt;
	    }
	  }
	} else done = TRUE;
      }
      
    } else if ( mode == PPD_VERTEX ) {
      
      sscanf(buf, "%s%s%s%s", val[0], val[1], val[2], val[3]);
      vt = ovt[id].vt;
      ++id;
      vt->vec.x = atof(val[1]);
      vt->vec.y = atof(val[2]);
      vt->vec.z = atof(val[3]);
      
    } else if ( mode == PPD_FACE ) {
			
      char *bpt = buf;
      char stc[BUFSIZ];
      bpt = cut(bpt, stc);
      char fcmode = '\0';
      if ( (stc[0] == '/') && (stc[1] == 'n' || stc[1] == 'u'
			       || stc[1] == 'c' || stc[1] == 'h') ) {
				// /n or /u or /c or /h
				
	fcmode = stc[1];
      }
      int n = 0;
      
      int ibuf[BUFSIZ];
      BOOL done = FALSE;
      while ( done == FALSE ) {
	bpt = cut(bpt, stc);
	if ( !bpt ) {
	  if ( strcmp(stc, "\\") ) {
	    ibuf[n++] = atoi(stc);
	  }
	  done = TRUE;
	} else {
	  ibuf[n++] = atoi(stc);
	}
      }		

      if ( fcmode == '\0' ) {
				
	if ( n != TRIANGLE ) {
	  display("Warning: vertex number of Face No.%d is not a triangle. \n",
		  id + 1 );
	}
	fc = ofc[id].fc; ++id;
	Sphe *he = NULL;
	for ( int i = 0; i < n; ++i ) {
	  he = create_ppdhalfedge( fc );
	  he->vt = ovt[ibuf[i] - 1].vt;
	}
				
	// BEGIN: make Edge 
	he = fc->sphe;
	do {
					
	  Spvt *sv = he->vt;
	  Spvt *ev = he->nxt->vt;
	  
	  // vertex -> halfedge
	  // あとで境界頂点のすべてのハーフエッジを簡単にアクセスできるように
	  // 再配列する
	  if ( sv->sphe == NULL ) {
	    sv->sphe = he;
	  }

	  if ( (ed = find_ppdedge(sv, ev)) == NULL ) {
	    // create an edge
	    ed = create_ppdedge( ppd );
	    // edge - vertex
	    ed->sv = sv;
	    ed->ev = ev;
	    //ed->length = V3DistanceBetween2Points( &(sv->vec), &(ev->vec) );
	    // vertex - edge
	    (void) create_vtxed( sv, ed );
	    (void) create_vtxed( ev, ed );
	    // edge - halfedge
	    he->ed = ed;
	    ed->lhe = he;

	  } else {
	    // an edge have already existed
	    he->ed = ed;
	    if ( (ed->sv == sv) && (ed->ev == ev) ) {
	      if ( ed->lhe != NULL ) {
		display("Warning. Edge %d has more than three halfedges.\n",
			ed->no );
	      }
	      ed->lhe  = he;
	      // set mate
	      if ( ed->rhe != NULL ) {
		ed->rhe->mate = he;
		he->mate = ed->rhe;
	      }
	      
	    } else {
							
	      if ( ed->rhe != NULL ) {
		display("Warning. Edge %d has more than three halfedges. \n",
			ed->no);
	      }
	      ed->rhe  = he;
	      // set mate
	      if ( ed->lhe != NULL ) {
		ed->lhe->mate = he;
		he->mate = ed->lhe;
	      }
	    }
						
	  }
					
	} while ( (he = he->nxt) != fc->sphe );
	
	// END: make Edge
				
	// normal
	calc_fnorm( fc );

      } else if (fcmode == 'n') {
      } else if (fcmode == 'u') {
      } else if (fcmode == 'c') {
      } else if (fcmode == 'h') {
      }

    } else if ( mode == PPD_TVERTEX ) {

      char *bpt = buf;
      char stc[BUFSIZ];
      bpt = cut( bpt, stc );
      BOOL done = FALSE;

      tvt = create_tvertex( tile );
      
      while ( done == FALSE ) {
	bpt = cut(bpt, stc);
	if ( (stc[0] == '/') && (stc[1] == 'v') ) {
	  bpt = cut(bpt, stc);
	  vt = ovt[atoi(stc)-1].vt;
	  // create tile vertex
	  tvt->vt = vt;
//  	  display("tvertex %d vt %d\n", tvt->no, tvt->vt->no );
	}
	if (!bpt) done = TRUE;
      }
      
    } else if ( mode == PPD_TEDGE ) {

      char *bpt = buf;
      char stc[BUFSIZ];
      bpt = cut( bpt, stc );

      ted = create_tedge( tile );

      int num;
      BOOL done = FALSE;
      while ( done == FALSE ) {
	bpt = cut( bpt, stc );
	if ( (stc[0] == '/') && (stc[1] == 'v') ) {
	  bpt = cut( bpt, stc );
	  TVertex *stvt = list_tvertex( tile, atoi(stc)-1 );
	  bpt = cut( bpt, stc );
	  TVertex *etvt = list_tvertex( tile, atoi(stc)-1 );
	  ted->sv = stvt;
	  ted->ev = etvt;
	} else if ( (stc[0] == '/') && (stc[1] == 'l') ) {
	  bpt = cut( bpt, stc );
	  num = atoi( stc );
	} else if ( (stc[0] == '/') && (stc[1] == 'g') ) {
	  bpt = cut( bpt, stc );
	  ted->sp_gamma = atof( stc );
	}
	  
	if ( !bpt ) done = TRUE;
      }
//        display("tedge %d sv %d ev %d\n",
//  	      ted->no, ted->sv->no, ted->ev->no );

      //
      // ループの生成
      //
      Splp *lp = ted->lp = create_splp();
      
      for ( int i = 0; i < num; ++i ) {

	// ループ頂点の生成
	Splv *lv = create_ppdloopvertex( lp );

	fgets( buf, BUFSIZ, fp );
	bpt = buf;
	bpt = cut( bpt, stc );
	
	Spvt *sv, *ev;
	BOOL done = FALSE;
	while ( done == FALSE ) {
	  bpt = cut(bpt, stc);
	  if ( (stc[0] == '/') && (stc[1] == 'v') ) {
	    bpt = cut( bpt, stc );
	    vt = ovt[atoi(stc)-1].vt;
	    lv->vt = vt;
	  } else if ( (stc[0] == '/') && (stc[1] == 's') ) {
	    bpt = cut( bpt, stc );
	    lv->type = atoi(stc);
	  } else if ( (stc[0] == '/') && (stc[1] == 't') ) {
	    bpt = cut( bpt, stc );
	    lv->vt->type = atoi(stc);
	  } else if ( (stc[0] == '/') && (stc[1] == 'e') ) {
	    bpt = cut( bpt, stc );
	    sv = ovt[atoi(stc)-1].vt;
	    bpt = cut( bpt, stc );
	    ev = ovt[atoi(stc)-1].vt;
	    ed = find_ppdedge( sv, ev );
	    assert( ed != NULL );
	    vt = lv->vt;
	    assert( vt->type == VERTEX_ADDED );
	    vt->sp_ed = ed;
	    vt->sp_val = ( V3DistanceBetween2Points( &(vt->vec), &(sv->vec) )
			   / ed->length );
	  }
	  if ( !bpt ) done = TRUE;
	}
//  	display("\tlv i %d vt %d type %d\n",
//  		i, lv->vt->no, lv->vt->type );

	++(lv->vt->spn);
	
	// loopvertex の属性をつける
	if ( lv->type == LOOPVT_CORNER ) {
	  lv->vt->sp_type = SP_VERTEX_TEDGE_BP;
	} else {
	  lv->vt->sp_type = SP_VERTEX_TEDGE;
	}
	//lv->vt->sp_type = SP_VERTEX_TEDGE;

	// ループエッジの生成（なければ）
	if ( !i ) continue;

	ev = lv->vt;
	sv = lv->prv->vt;
	if ( (ed = find_ppdedge( sv, ev )) == NULL ) {
	  ed = create_ppdedge( ppd );
	  ed->sv = sv;
	  ed->ev = ev;
	  ed->length = V3DistanceBetween2Points( &(sv->vec), &(ev->vec) );
	  (void) create_vtxed( sv, ed );
	  (void) create_vtxed( ev, ed );
	  // EDGE_ONFACE に違いない
	  ed->type = EDGE_ONFACE;
//  	  display("ed %d type %d sv %d (type %d) ev %d (type %d)\n",
//  		  ed->no, ed->type,
//  		  ed->sv->no, ed->sv->type,
//  		  ed->ev->no, ed->ev->type );
	  
	  // 下の面を見つける
	  ed->sp_fc = find_edgeonface( ed );
	  assert( ed->sp_fc != NULL );
	  if ( ed->sp_fc == NULL ) {
	    display("Warning: ed %d sv %d ev %d\n", ed->no, sv->no, ev->no );
	  }
	  
	  //ed->fc = sged->spfc;
	}
	
	ed->sp_type = SP_EDGE_BOUNDARY;
	
      }
      
      if ( lp->splv != NULL ) lp->splv->vt->sp_type = SP_VERTEX_TVERTEX;
      if ( lp->eplv != NULL ) lp->eplv->vt->sp_type = SP_VERTEX_TVERTEX;
      
    } else if ( mode == PPD_TFACE ) {

      tfc = create_tface( tile );
      
      char *bpt = buf;
      char stc[BUFSIZ];
      bpt = cut( bpt, stc );
      
      Spvt *start_vt;
      BOOL done = FALSE;
      while ( done == FALSE ) {
	bpt = cut(bpt, stc);
	if ( (stc[0] == '/') && (stc[1] == 'p') ) {

	  // part
	  bpt = cut( bpt, stc );
	  int n = atoi(stc)-1;
	  tfc->pt = list_ppdpart( ppd, n );
	  assert( tfc->pt );

	  // 同じ part の面に属性をつける
	  for ( fc = ppd->spfc; fc != NULL; fc = fc->nxt ) {
	    if ( fc->bppt == tfc->pt ) fc->tile_id = tfc->no;
	  }
	  

	} else if ( (stc[0] == '/') && (stc[1] == 't') ) {

	  // tvector
	  bpt = cut( bpt, stc );
	  tfc->n_tvector = atoi( stc );
	  tfc->tvec = (TVector *) malloc( tfc->n_tvector * sizeof(TVector) );
	  for ( int i = 0; i < tfc->n_tvector; ++i ) {
	    initialize_tvector( &(tfc->tvec[i]) );
	  }
	  tfc->isParameterized = TRUE;

	} else if ( (stc[0] == '/') && (stc[1] == 's') ) {

	  // start vertex
	  bpt = cut( bpt, stc );
	  int n = atoi(stc)-1;
	  start_vt = list_ppdvertex( ppd, n );

	} else if ( (stc[0] == '/') && (stc[1] == 'e') ) {

	  // 一番最後にエッジ列がくることを前提に書いている
	  BOOL bdone = FALSE;
	  int en = 0; int ibuf[BUFSIZ];
	  while ( bdone == FALSE ) {
	    bpt = cut( bpt, stc );
	    ibuf[en++] = atoi(stc)-1;
	    if ( !bpt ) bdone = TRUE;
	  }

	  tfc->n_tedge = en;
	  tfc->ed = (TEdge **) malloc( en * sizeof(TEdge *) );
	  for ( int i = 0; i < en; ++i ) {
	    tfc->ed[i] = list_tedge( tile, ibuf[i] );
	  }

	}

	if ( !bpt ) done = TRUE;
	
      }

      // closed loop
      assert( start_vt != NULL );
      tfc->clp = create_closedloop_mdw( tfc, start_vt );

      for ( int i = 0; i < tfc->n_tvector; ++i ) {

	TVector *tvec = &(tfc->tvec[i]);
	
	fgets( buf, BUFSIZ, fp );
	bpt = buf;
	bpt = cut( bpt, stc );
	
	BOOL done = FALSE;
	while ( done == FALSE ) {
	  bpt = cut(bpt, stc);
	  if ( (stc[0] == '/') && (stc[1] == 'v') ) {
	    bpt = cut( bpt, stc );
	    vt = ovt[atoi(stc)-1].vt;
	    tvec->vt = vt;
	  } else if ( (stc[0] == '/') && (stc[1] == 'p') ) {
	    bpt = cut( bpt, stc );
	    tvec->uv.x = atof( stc );
	    bpt = cut( bpt, stc );
	    tvec->uv.y = atof( stc );
	  }
	  if ( !bpt ) done = TRUE;
	}
//  	display("tvector i %d vt %d uv %g %g\n",
//  		i, tvec->vt->no, tvec->uv.x, tvec->uv.y );
      }
      
    } else if ( mode == PPD_NONE ) {
      
      if ( !strcmp(key, "header") ) {
	mode = PPD_HEADER;
      } else if ( !strcmp(key, "solid") ) {
	mode = PPD_SOLID;
	id = 0;
      } else if ( !strcmp(key, "part") ) {
	mode = PPD_PART;
	id = 0;
      } else if ( !strcmp(key, "face") ) {
	mode = PPD_FACE;
	id = 0;
      } else if ( !strcmp(key, "vertex") ) {
	mode = PPD_VERTEX;
	id = 0;
      } else if ( !strcmp(key, "tvertex") ) {
	mode = PPD_TVERTEX;
	id = 0;
      } else if ( !strcmp(key, "tedge") ) {
	mode = PPD_TEDGE;
	id = 0;
      } else if ( !strcmp(key, "tface") ) {
	mode = PPD_TFACE;
	id = 0;
      } else if ( !strcmp(key, "line_mdl") ) {
	mode = PPD_LOOP;
	id = 0;
      }
    }
  }
  
  fclose( fp );

  return tile;
}

//
// ed->type == EDGE_ONFACE のときの ed の下の面を見つける
// (sv->sp_ed or ev->sp_ed があることを前提としている)
//
Spfc *find_edgeonface( Sped *ed )
{
  if ( ed->type != EDGE_ONFACE ) return NULL;
  
  Spvt *sv = ed->sv;
  Spvt *ev = ed->ev;
  
  if ( (sv->type == VERTEX_ORIGINAL) && (ev->type == VERTEX_ORIGINAL) )
    { // これはありえない
    
      return NULL;
    
    }
  else if ( (sv->type == VERTEX_ADDED) && (ev->type == VERTEX_ORIGINAL) )
    {
      Sped *sp_ed = sv->sp_ed;
      assert( sp_ed != NULL );
    for ( int i = 0; i < 2; ++i ) {
      Sphe *he = (!i) ? sp_ed->lhe : sp_ed->rhe;
      if ( he == NULL ) continue;
      Spfc *fc = he->bpfc;
      if ( find_vertex_in_face( fc, ev ) == TRUE ) {
	return fc;
      }
    }
    
  } else if ( (sv->type == VERTEX_ORIGINAL) && (ev->type == VERTEX_ADDED) ) {

    Sped *sp_ed = ev->sp_ed;
    assert( sp_ed != NULL );
    for ( int i = 0; i < 2; ++i ) {
      Sphe *he = (!i) ? sp_ed->lhe : sp_ed->rhe;
      if ( he == NULL ) continue;
      Spfc *fc = he->bpfc;
//        display("\tfc %d\n", fc->no );
      if ( find_vertex_in_face( fc, sv ) == TRUE ) {
	return fc;
      }
    }
    
  } else if ( (sv->type == VERTEX_ADDED) && (ev->type == VERTEX_ADDED) ) {

    Sped *eda = sv->sp_ed;
    Sped *edb = ev->sp_ed;
    assert( eda != NULL ); assert( edb != NULL );
    for ( int i = 0; i < 2; ++i ) {
      Sphe *he = (!i) ? eda->lhe : eda->rhe;
      if ( he == NULL ) continue;
      Spfc *fc = he->bpfc;
      if ( edb->lhe != NULL ) {
	if ( fc == edb->lhe->bpfc ) return fc;
      }
      if ( edb->rhe != NULL ) {
	if ( fc == edb->rhe->bpfc ) return fc;
      }
    }
    
  }

  return NULL;
}

//
// fc の中に vt があるかどうかのチェック
//
BOOL find_vertex_in_face( Spfc *fc, Spvt *vt )
{
  if ( fc == NULL ) return FALSE;

  Sphe *he = fc->sphe;
  do {
    if ( he->vt == vt ) return TRUE;
  } while ( (he = he->nxt) != fc->sphe );

  return FALSE;
}

Splp *create_closedloop_mdw( TFace *tfc, Spvt *start_vt )
{
  Splp *clp = create_splp();
  clp->type = CLOSEDLOOP;

  for ( int i = 0; i < tfc->n_tedge; ++i ) {

    Splp *lp = tfc->ed[i]->lp;
    Splp *nlp = ( i != (tfc->n_tedge-1) ) ? tfc->ed[i+1]->lp : tfc->ed[0]->lp;

    // decide lp's direction
    BOOL reverse = FALSE;
    if ( (!i) && (start_vt != NULL) ) {
      
      if ( lp->splv->vt != start_vt ) reverse = TRUE;
      
    } else {
      
      if ( lp->splv->vt == nlp->splv->vt ) {
	reverse = TRUE;
      } else if ( lp->splv->vt == nlp->eplv->vt ) {
	reverse = TRUE;
      } else if ( lp->eplv->vt == nlp->splv->vt ) {
	reverse = FALSE;
      } else if ( lp->eplv->vt == nlp->eplv->vt ) {
	reverse = FALSE;
      }
      
    }

    // make closed loop's vertices
    Splv *lv, *clv;
    if ( reverse == FALSE ) {
      for ( lv = lp->splv; lv != lp->eplv; lv = lv->nxt ) {
	clv = create_ppdloopvertex( clp );
	clv->vt = lv->vt;
	clv->type = ( lv != lp->splv ) ? LOOPVT_ONBOUND : LOOPVT_CORNER;
	// loop vertex becomes ORIGINAL
      }
    } else { // reverse == TRUE
      for ( lv = lp->eplv; lv != lp->splv; lv = lv->prv ) {
	clv = create_ppdloopvertex( clp );
	clv->vt = lv->vt;
	clv->type = ( lv != lp->eplv ) ? LOOPVT_ONBOUND : LOOPVT_CORNER;
      }
    }
    
  }

//    for ( Splv *a = clp->splv; a != NULL; a = a->nxt ) {
//      display("(closed) lv %d vt %d type %d\n", a->no, a->vt->no, a->type  );
//    }
  return clp;
}
