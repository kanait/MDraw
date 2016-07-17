//
// smf.cxx
//
// Copyright (c) 2000 Takashi Kanai; All rights reserved. 
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
#include "file.h"
#include "veclib.h"

////////////////////////////////////////////////
//
// Write
//
////////////////////////////////////////////////

void write_smf( char *fname, Sppd *ppd )
{
  FILE *fp;
  if ( (fp = fopen(fname, "w")) == NULL ) {
    return;
  }
	
  // header
  fprintf(fp, "#$SMF 1.0\n");

  Spso *s;
  Sppt *p;
  Spfc *f;
  Spvt *v;
  Sphe *he;
  int  i, n;
  Id   vtx[128];
	
  // vertex
  if ( ppd->vn ) {
    i = 1;
    for ( s = ppd->spso; s != NULL; s = s->nxt ) {
      for ( v = ppd->spvt; v != NULL; v = v->nxt ) {
	if (v->bpso == s) {
	  if ( v->sp_type != SP_VERTEX_STEINER ) {
	    v->sid = i;
	    Vec vec;
	    if ( fabs(v->vec.x) > SMDZEROEPS ) vec.x = v->vec.x; else vec.x = 0;
	    if ( fabs(v->vec.y) > SMDZEROEPS ) vec.y = v->vec.y; else vec.y = 0;
	    if ( fabs(v->vec.z) > SMDZEROEPS ) vec.z = v->vec.z; else vec.z = 0;
	    fprintf(fp, "v %g %g %g\n", vec.x, vec.y, vec.z );
	    ++i;
	  }
	}
      }
    }
  }
	
  // face
  if ( ppd->fn ) {
    i = 1;
    for ( s = ppd->spso; s != NULL; s = s->nxt ) {
      for ( p = ppd->sppt; p != NULL; p = p->nxt ) {
	for ( f = ppd->spfc; f != NULL; f = f->nxt ) {
	  if ( (f->bpso == s) && (f->bppt == p) ) {

	    he = f->sphe;
	    n = 0;
	    do {
	      if ( ppd->vn ) vtx[n] = he->vt->sid;
	      ++n;
	    } while ((he = he->nxt) != f->sphe);
						
	    fprintf( fp, "f " );
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
  }
	
  fclose(fp);
}

Sppd *open_smf( char *fname )
{
  // file variables
  FILE *fp;
  if ( (fp = fopen(fname, "r")) == NULL ) {
    return NULL;
  }

  // 読み込み 1 回目
  //
  // open ppd elements
  //
  char buf[BUFSIZ], key[BUFSIZ];
  int nv = 0; int nf = 0;
  while ( fgets(buf, BUFSIZ, fp) ) {
    sscanf( buf, "%s", key );
    if (comment(buf[0])) continue;
    else if ( !strcmp(key, "v") ) ++nv;
    else if ( !strcmp(key, "f") ) ++nf;
  }
  fclose( fp );

  Sppd *ppd  = create_ppd();
  sprintf( ppd->filename, "%s", fname );
	
  Oppd *oppd = create_oppd();
  // num of solid and part is 1
  int i; int num = 1;
  Oso *oso = create_osolid( num );
  for ( i = 0; i < num; ++i ) {
    Spso *so = create_ppdsolid( ppd );
    oso[i].so = so;
  }
  Opt *opt = create_opart( num );
  for ( i = 0; i < num; ++i ) {
    Sppt *pt = create_ppdpart( ppd );
    opt[i].pt = pt;
  }
  Ovt *ovt = create_overtex( nv );
  for ( i = 0; i < nv; ++i ) {
    Spvt *vt = create_ppdvertex( ppd );
    ovt[i].vt = vt;
    vt->bpso = ppd->spso;
  }
  Ofc *ofc = create_oface( nf );
  for ( i = 0; i < nf; ++i ) {
    Spfc *fc = create_ppdface( ppd );
    ofc[i].fc = fc;
    fc->bpso = ppd->spso;
    fc->bppt = ppd->sppt;
  }
  
  // 二回目
  if ( (fp = fopen(fname, "r")) == NULL ) {
    return NULL;
  }
  //mode = PPD_NONE;

  nv = 0; nf = 0;
  while ( fgets(buf, BUFSIZ, fp) ) {
    
    sscanf(buf, "%s", key);
    if ( comment(buf[0]) ) {

      continue;
      
    } else if ( !strcmp(key, "v") ) {
      
      char val[4][BUFSIZ];
      sscanf(buf, "%s%s%s%s", val[0], val[1], val[2], val[3]);
      Spvt *vt = ovt[nv].vt; ++nv;
      vt->vec.x = atof(val[1]);
      vt->vec.y = atof(val[2]);
      vt->vec.z = atof(val[3]);
      
    } else if ( !strcmp(key, "f") ) {
			
      char *bpt = buf;
      char stc[BUFSIZ];
      bpt = cut( bpt, stc );
      display("%s\n", bpt );

      int n = 0;
      BOOL done = FALSE;
      int ibuf[BUFSIZ];
      while ( done == FALSE ) {
	bpt = cut( bpt, stc );
	if ( !bpt ) {
	  done = TRUE;
	}
	ibuf[n++] = atoi( stc );
      }

      if ( n != TRIANGLE ) {
	display("Warning: vertex number of Face No.%d is not a triangle. \n",
		nf + 1 );
      }
      Spfc *fc = ofc[nf].fc; ++nf;

      // halfedge の生成
      Sphe *he;
      for ( i = 0; i < n; ++i ) {
	he = create_ppdhalfedge( fc );
	he->vt = ovt[ibuf[i] - 1].vt;
      }
				
      //display("fc %d hen %d\n", fc->no, fc->hen );
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

	Sped *ed;
	if ( (ed = find_ppdedge(sv, ev)) == NULL ) {
	  // create an edge
	  ed = create_ppdedge( ppd );
	  // edge - vertex
	  ed->sv = sv;
	  ed->ev = ev;
	  // ここではあとで計算するのでいらない
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
	      display("Warning. Edge %d has more than three halfedges. \n",
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
      calc_fnorm(fc);
    }
  }
	
  fclose( fp );
	
  free_oppd( oppd );
	
  // isolated vertices check.
  ppdvertex_noedge_check( ppd );
  
  //
  // needed for calculating shortest paths 
  normalize_ppd( ppd );
	
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

  return ppd;
}


