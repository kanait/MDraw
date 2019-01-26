//
// ppdlist.cxx
//
// Copyright (c) 1997-1999 Takashi Kanai
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
#include "file.h"
#include "veclib.h"

////////////////////////////////////////////////
//
// Write
//
////////////////////////////////////////////////

void write_ppd_file( char *fname, Sppd *ppd, BOOL isTexCoord )
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
  int  solidnum, partnum, vertexnum, normalnum, uvwnum;
  int  facenum, loopnum, fnodenum;
  int  bfc, efc, bpt, ept, bvt, evt, bnm, enm;
  Id	 vtx[128], nrm[128];
	
  if ((fp = fopen(fname, "w")) == NULL) {
    return;
  }
	
  /* header */
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
    /*		 display("nm %g %g %g\n", nm->vec.x, nm->vec.y, nm->vec.z); */
    ++normalnum;
  }
  for (f = ppd->spfc, facenum = fnodenum = 0; f != (Spfc *) NULL; f = f->nxt) {
    ++facenum;
    fnodenum += f->hen;
  }
  if ( isTexCoord == TRUE )
    {
      uvwnum = vertexnum;
    }
  else
    {
      uvwnum = 0;
    }
	
  fprintf(fp, "header\n");
  if (solidnum)  fprintf(fp, "\tsolid\t%d\n",  solidnum);
  if (partnum)	 fprintf(fp, "\tpart\t%d\n",	 partnum);
  if (vertexnum) fprintf(fp, "\tvertex\t%d\n", vertexnum);
  if (normalnum) fprintf(fp, "\tnormal\t%d\n", normalnum);
  if (uvwnum) fprintf(fp, "\tuvw\t%d\n", uvwnum);
  if (facenum)	 {
    fprintf(fp, "\tface\t%d\n",  facenum);
    fprintf(fp, "\tfnode\t%d\n", fnodenum);
  }
  fprintf(fp, "end\n");
	
  /* solid */
	
  if (solidnum) {
    bfc = efc  = 0;
    bpt = ept  = 0;
    bvt = evt  = 0;
    bnm = enm  = 0;
    fprintf(fp, "solid\n");
    i = 1;
    for (s = ppd->spso; s != (Spso *) NULL; s = s->nxt) {
      fprintf(fp, "\t%d\t", i); 			/* print Id */
      if (partnum) {
	bpt = ept + 1;
	for (p = ppd->sppt; p != (Sppt *) NULL; p = p->nxt) {
	  if ( p->bpso == s ) {
	    ++ept;
	  }
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
	if ( uvwnum ) fprintf(fp, "/u %d %d ", bvt, evt);
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
      fprintf(fp, "\n");
    }
    fprintf(fp, "end\n");
  }
	
  /* part */
  if (partnum) {
    bfc = efc  = 0;
    fprintf(fp, "part\n");
    i = 1;
    for (p = ppd->sppt; p != (Sppt *) NULL; p = p->nxt) {
      fprintf(fp, "\t%d\t", i); 			/* print Id */
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
	
  /* vertex */
  if (vertexnum) {
    fprintf(fp, "vertex\n");
    i = 1;
    for (s = ppd->spso; s != (Spso *) NULL; s = s->nxt) {
      for (v = ppd->spvt; v != (Spvt *) NULL; v = v->nxt) {
	if (v->bpso == s) {
	  if ( v->sp_type != SP_VERTEX_STEINER ) {
	    v->sid = i;
	    if ( fabs(v->vec.x) > SMDZEROEPS ) vec.x = v->vec.x; else vec.x = 0;
	    if ( fabs(v->vec.y) > SMDZEROEPS ) vec.y = v->vec.y; else vec.y = 0;
	    if ( fabs(v->vec.z) > SMDZEROEPS ) vec.z = v->vec.z; else vec.z = 0;
	    /*			display("v no %d sid %d %g %g %g\n", v->no, v->sid, vec.x, vec.y, vec.z); */
	    fprintf(fp, "\t%d\t%g %g %g\n", i, vec.x, vec.y, vec.z);
	    ++i;
	  }
	}
      }
    }
    fprintf(fp, "end\n");
  }
	
  /* normal */
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
	
  /* uvw */
  if ( uvwnum ) {
    fprintf(fp, "uvw\n");
    i = 1;
    for (s = ppd->spso; s != (Spso *) NULL; s = s->nxt) {
      for (v = ppd->spvt; v != (Spvt *) NULL; v = v->nxt) {
	if (v->bpso == s) {
	  if ( v->sp_type != SP_VERTEX_STEINER ) {
//  	    v->sid = i;
	    fprintf(fp, "\t%d\t%g %g 0\n", i, v->uvw.x, v->uvw.y );
	    ++i;
	  }
	}
      }
    }
    fprintf(fp, "end\n");
  }
	
  /* face */
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
	    for (j = 0; j < n; ++j)
	      {
		fprintf(fp, "%d ", vtx[j]);
	      }
	    // normal
	    if (normalnum)
	      {
		//  	      fprintf(fp, "\\\n");
		fprintf(fp, "/n ");
		for (j = 0; j < n; ++j)
		  {
		    fprintf(fp, "%d ", nrm[j]);
		  }
	      }
	    if ( uvwnum )
	      { 
		fprintf(fp, "/u ");
		for (j = 0; j < n; ++j)
		  {
		    fprintf(fp, "%d ", vtx[j]);
		  }
		
	      }
	    fprintf(fp, "\n");
	    
	    ++i;
	  }
	}
      }
    }
    fprintf(fp, "end\n");
  }
	
  if ( loopnum ) {
    // loop 
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
	if (l->type == CLOSEDLOOP) a = i;
	else											 a = -i;
	fprintf(fp, "\t%d\t", a);
	for (lv = l->splv; lv != (Splv *) NULL; lv = lv->nxt) {
	  if (lv->type == LOOPVT_ONBOUND) {
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

Sppd *open_ppd( char *fname, BOOL isNormalize )
{
  // file variables
  FILE *fp;
  int i, mode, n, done, ibuf[BUFSIZ], readmode = SMD_OFF;
  Id		num, bn, en, id = 0;
  Id		a;
  char	*bpt;
  char	fcmode, stc[BUFSIZ], buf[BUFSIZ];
  char	key[BUFSIZ], val[7][BUFSIZ];
  // ppd elements
  Sppd *ppd;
  Spso *so;
  Sppt *pt;
  Spfc *fc = NULL;
  Spvt *vt, *sv, *ev;
  Spnm *nm;
  Sphe *he;
  Splp *lp = NULL;
  Sped *ed;
  //	Vted *ve = NULL;
  Splv *lv;
  //Vec  *mvec1 = NULL, *mvec2 = NULL;
  //
  // open ppd elements
  //
  Oppd *oppd;
  Oso  *oso = NULL;
  Opt  *opt = NULL;
  Ofc  *ofc = NULL;
  Ovt  *ovt = NULL;
  Onm  *onm = NULL;
	
  if ((fp = fopen(fname, "r")) == NULL) {
    return (Sppd *) NULL;
  }
  mode = PPD_NONE;
	
  ppd  = create_ppd();
	
  sprintf( ppd->filename, "%s", fname );
	
  oppd = create_oppd();
	
  while (fgets(buf, BUFSIZ, fp)) {
    
    sscanf(buf, "%s", key);
    if (comment(buf[0])) continue;
    
    else if (!strcmp(key, "end")) {
      mode = PPD_NONE;
    }
    else if (mode == PPD_HEADER) {	// read "header"
      sscanf(buf, "%s%s", key, val[0]);
      num = atoi(val[0]);
      if (!strcmp(key, "solid")) {
	oso = create_osolid( num );
	for (i = 0; i < num; ++i) {
	  so = create_ppdsolid(ppd);
	  oso[i].so = so;
	}
      } else if (!strcmp(key, "part")) {
	opt = create_opart( num );
	for (i = 0; i < num; ++i) {
	  pt = create_ppdpart(ppd);
	  opt[i].pt = pt;
	}
      } else if (!strcmp(key, "face")) {
	ofc = create_oface( num );
	for (i = 0; i < num; ++i) {
	  fc = create_ppdface(ppd);
	  ofc[i].fc = fc;
	}
      } else if (!strcmp(key, "vertex")) {
	ppd->nvt_org = num;
	ovt = create_overtex( num );
	for (i = 0; i < num; ++i) {
	  vt = create_ppdvertex(ppd);
	  ovt[i].vt = vt;
	}
      } else if (!strcmp(key, "normal"))
	{
	  onm = create_onormal( num );
	  for (i = 0; i < num; ++i) {
	    nm = create_ppdnormal(ppd);
	    onm[i].nm = nm;
	  }
      } else if (!strcmp(key, "uvw"))
	{
	  assert( num == ppd->vn );
	  ppd->texcoord = TRUE;
	}      
    } else if (mode == PPD_SOLID) {
      bpt = buf;
      bpt = cut(bpt, stc);
      fcmode = '\0';
      so = oso[atoi(stc) - 1].so;
      done = SMD_OFF;
      while (!done) {
	bpt = cut(bpt, stc);
	if (stc[0] == '/' && (stc[1] == 'f' || stc[1] == 'n'
			      || stc[1] == 'v' || stc[1] == 'p')) {
	  /* /f or /n or /v */
	  fcmode = stc[1];
	  bpt = cut(bpt, val[0]); bn = atoi(val[0]) - 1;
	  bpt = cut(bpt, val[1]); en = atoi(val[1]) - 1;
	  if (fcmode == 'f') {
	    for (i = bn; i <= en; ++i) {
	      fc = ofc[i].fc;
	      fc->bpso = so;
	    }
	  } else if (fcmode == 'n') {
	    for (i = bn; i <= en; ++i) {
	      nm = onm[i].nm;
	      nm->bpso = so;
	    }
	  } else if (fcmode == 'v') {
	    for (i = bn; i <= en; ++i) {
	      vt = ovt[i].vt;
	      vt->bpso = so;
	    }
	  } else if (fcmode == 'p') {
	    for (i = bn; i <= en; ++i) {
	      pt = opt[i].pt;
	      pt->bpso = so;
	    }
	  }
	}
	if (!bpt)  done = SMD_ON;
      }
    } else if (mode == PPD_PART) {
      bpt = buf;
      bpt = cut(bpt, stc);
      fcmode = '\0';
      pt = opt[atoi(stc) - 1].pt;
      done = SMD_OFF;
      while (!done) {
	if (bpt) {
	  bpt = cut(bpt, stc);
	  if (stc[0] == '/' && (stc[1] == 'f' || stc[1] == 'b')) {
	    /* /f or /b */
	    fcmode = stc[1];
	    bpt = cut(bpt, val[0]); bn = atoi(val[0]) - 1;
	    bpt = cut(bpt, val[1]); en = atoi(val[1]) - 1;
	    for (i = bn; i <= en; ++i) {
	      fc = ofc[i].fc;
	      fc->bppt = pt;
	    }
	  }
	} else done = SMD_ON;
      }
      
    } else if (mode == PPD_VERTEX) {
      
      sscanf(buf, "%s%s%s%s", val[0], val[1], val[2], val[3]);
      vt = ovt[id].vt;
      ++id;
      vt->vec.x = atof(val[1]);
      vt->vec.y = atof(val[2]);
      vt->vec.z = atof(val[3]);
      
    } else if (mode == PPD_NORMAL) {
			
      sscanf(buf, "%s%s%s%s", val[0], val[1], val[2], val[3]);
      nm = onm[id].nm; ++id;
      nm->vec.x = atof(val[1]);
      nm->vec.y = atof(val[2]);
      nm->vec.z = atof(val[3]);
			
    } else if (mode == PPD_UVW) {
      
      sscanf(buf, "%s%s%s%s", val[0], val[1], val[2], val[3]);
      vt = ovt[id].vt;
      ++id;
      vt->uvw.x = atof(val[1]);
      vt->uvw.y = atof(val[2]);
      
    } else if (mode == PPD_FACE) {
			
      bpt = buf;
      bpt = cut(bpt, stc);
      fcmode = '\0';
      if (stc[0] == '/' && (stc[1] == 'n' || stc[1] == 'u'
			    || stc[1] == 'c' || stc[1] == 'h')) 
	{ // /n or /u or /c or /h 
	  fcmode = stc[1];
	}

      int n  = 0;
      int un = 0;
      int nn = 0;
      BOOL done = FALSE;
      BOOL ndone = FALSE;
      BOOL udone = FALSE;
      char ubuf[BUFSIZ], nbuf[BUFSIZ];
      while ( !done )
	{
	  bpt = cut(bpt, stc);
	  if (!bpt)
	    {
	      if (strcmp(stc, "\\"))
		{
		  ibuf[n++] = atoi(stc);
		}
	      done = TRUE;
	    }
	  else
	    {
	      if ( stc[0] != '/' ) // coordinate
		{
		  ibuf[n++] = atoi(stc);
		}
	      else if ( (stc[0] == '/') && (stc[1] == 'n') ) // normal
		{
		  ndone = FALSE;
		  while ( !ndone )
		    {
		      bpt = cut(bpt, stc);
		      if (!bpt)
			{
			  if ( strcmp(stc, "\\") )
			    {
			      nbuf[nn++] = atoi(stc);
			    }
			  done  = TRUE;
			  ndone = TRUE;
			  udone = TRUE;
			}
		      else
			{
			  nbuf[nn++] = atoi(stc);
			}
		    }
		}
	      else if ((stc[0] == '/') && (stc[1] == 'u')) //texture
		{
		  udone = FALSE;
		  while (!udone)
		    {
		      bpt = cut(bpt, stc);
		      if (!bpt) {
			if (strcmp(stc, "\\"))
			  {
			    ubuf[un++] = atoi(stc);
			  }
			done  = TRUE;
			udone = TRUE;
			ndone = TRUE;
		      }
		      else
			{
			  ubuf[un++] = atoi(stc);
			}
		    }
		}
	    }
	}

#if 0      
//        n = 0;		
      done = SMD_OFF;
      while (!done) {
	bpt = cut(bpt, stc);
	if (!bpt) {
	  if (strcmp(stc, "\\")) {
	    ibuf[n++] = atoi(stc);
	  }
	  done = SMD_ON;
	} else {
	  ibuf[n++] = atoi(stc);
	}
      }
#endif      

      if (fcmode == '\0') {
				
	// printf("n %d\n", n );
	// for (i = 0; i < n; ++i) 
	// printf("%d %d\n", i, ibuf[i]);
				
	if ( n != TRIANGLE ) {
	  display("vertex number of Face No.%d is not a triangle. \n",
		  id + 1 );
	  //exit(1);
	}
	fc = ofc[id].fc; ++id;
	for (i = 0; i < n; ++i) {
	  he = create_ppdhalfedge( fc );
	  he->vt = ovt[ibuf[i] - 1].vt;
	}
				
				// BEGIN: make Edge 
	he = fc->sphe;
	do {
					
	  sv = he->vt;
	  ev = he->nxt->vt;
	  
	  // vertex -> halfedge
	  // あとで境界頂点のすべてのハーフエッジを簡単にアクセスできるように
	  // 再配列する
	  if ( sv->sphe == (Sphe *) NULL ) {
	    sv->sphe = he;
	  }
					
	  if ((ed = find_ppdedge(sv, ev)) == (Sped *) NULL) {
	    // create an edge
	    ed = create_ppdedge( ppd );
	    // edge - vertex
	    ed->sv = sv;
	    ed->ev = ev;
	    ed->length = V3DistanceBetween2Points( &(sv->vec), &(ev->vec) );
	    // vertex - edge
	    (void) create_vtxed( sv, ed );
	    (void) create_vtxed( ev, ed );
	    // edge - halfedge
	    he->ed = ed;
	    ed->lhe = he;
//  	    ++(ed->fn);

	  } else {
	    // an edge have already existed
	    he->ed = ed;
	    if ( (ed->sv == sv) && (ed->ev == ev) ) {
	      if ( ed->lhe != (Sphe *) NULL ) {
		display("Warning. Edge %d has more than three halfedges. \n",
			ed->no );
	      }
	      ed->lhe  = he;
	      // set mate
	      if ( ed->rhe != NULL ) {
		ed->rhe->mate = he;
		he->mate = ed->rhe;
	      }
//  	      ++(ed->fn);
	      
	    } else {
							
	      if ( ed->rhe != (Sphe *) NULL ) {
		display("Warning. Edge %d has more than three halfedges. \n",
			ed->no);
	      }
	      ed->rhe  = he;
	      // set mate
	      if ( ed->lhe != NULL ) {
		ed->lhe->mate = he;
		he->mate = ed->lhe;
	      }
//  	      ++(ed->fn);
	    }
						
	  }
					
	} while ( (he = he->nxt) != fc->sphe );
	
	// END: make Edge
				
	// normal
	calc_fnorm(fc);

	if ( nn ) // normal
	  {
	    int k = 0;
	    he = fc->sphe;
	    do
	      {
		he->nm = onm[nbuf[k]-1].nm;
		++k;
	      }
	    while ((he = he->nxt) != fc->sphe);
	  }

#if 0	
	if ( un ) // texture
	  {
	    int k = 0;
	    he = fc->sphe;
	    do
	      {
//  		he->vt->uvw.x = ouv[un[k]-1].x;
//  		he->vt->uvw.y = ouv[un[k]-1].y;
		++k;
	      }
	    while ( (he = he->nxt) != fc->sphe );
	  }
#endif	

      } else if (fcmode == 'n')
	{
	  i = 0;
	  he = fc->sphe;
	  do
	    {
	      he->nm = onm[ibuf[i++] - 1].nm;
	    }
	  while ( (he = he->nxt) != fc->sphe );
	  
      } else if (fcmode == 'u') {
      } else if (fcmode == 'c') {
      } else if (fcmode == 'h') {
      }
      
    } else if ( mode == PPD_LOOP ) {
      
      bpt = buf;
      if ( readmode == SMD_OFF ) { // first line
	readmode = SMD_ON;
	bpt = cut( bpt, stc );
	++id;
	lp = create_ppdloop( ppd );
	if (atoi(stc) < 0) lp->type = OPENLOOP;
	n  = 0;
      }

      // read vertex num
      while (bpt != (char *) NULL) {
	bpt = cut(bpt, stc);
	if (strcmp(stc, "\\")) {
	  a = atoi(stc);
	  lv = create_ppdloopvertex( lp );
	  if (a < 0) {
	    lv->type = LOOPVT_ONBOUND;
	    a *= -1;
	  } else {
	    lv->type = LOOPVT_CORNER;
	    ++(lp->lcvn);
	  }
	  lv->vt = ovt[a - 1].vt;
	}
      }
      /* last line */

#if 0      
      if (strcmp(stc, "\\")) {

	readmode = SMD_OFF;
	for ( lv = lp->splv; lv->nxt != (Splv *) NULL; lv = lv->nxt ) {
	  if ( (ed = find_ppdedge( lv->vt, lv->nxt->vt )) != NULL ) {
	    ed->used_loop = SMD_ON;
	  }
	}
	if ( (ed = find_ppdedge( lp->splv->vt, lp->eplv->vt )) != NULL ) {
	  ed->used_loop = SMD_ON;
	}
				
      }
#endif
			
    } else if (mode == PPD_NONE) {
      if (!strcmp(key, "header")) {
	mode = PPD_HEADER;
      } else if (!strcmp(key, "solid")) {
	mode = PPD_SOLID;
	id = 0;
      } else if (!strcmp(key, "part")) {
	mode = PPD_PART;
	id = 0;
      } else if (!strcmp(key, "face")) {
	mode = PPD_FACE;
	id = 0;
      } else if (!strcmp(key, "vertex")) {
	mode = PPD_VERTEX;
	id = 0;
      } else if (!strcmp(key, "normal")) {
	mode = PPD_NORMAL;
	id = 0;
      } else if (!strcmp(key, "uvw")) {
	mode = PPD_UVW;
	id = 0;
      } else if (!strcmp(key, "line_mdl")) {
	mode = PPD_LOOP;
	id = 0;
	readmode = SMD_OFF;
      }
    }
  }
	
  fclose( fp );

  free_oppd( oppd );
	
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

  return ppd;
}

/***************************************
Printing Functions
****************************************/

// void printppd( Sppd *ppd )
// {
//   Spso *so;
//   Sppt *pt;
//   Spfc *fc;
//   Spnm *nm;
//   Spvt *vt;
//   Sped *ed;
//   Sphe *he;

//   if ( ppd->sn ) {
//     display("solid %d\n", ppd->sn);
//     for (so = ppd->spso; so != (Spso *) NULL; so = so->nxt) 
//       display("(no) %d\n", so->no);
//   }
		
//   if ( ppd->pn ) {
//     display("part %d\n", ppd->pn);
//     for (pt = ppd->sppt; pt != (Sppt *) NULL; pt = pt->nxt) 
//       display("(no) %d\n", pt->no);
//   }
		 
//   if ( ppd->vn ) {
//     display("vertex %d\n", ppd->vn);
//     for (vt = ppd->spvt; vt != (Spvt *) NULL; vt = vt->nxt) 
//       display("(no) %d type %d sp_type %d ven %d (vec) %g %g %g\n", vt->no,
// 	      vt->type, vt->sp_type, vt->ven, 
// 	      vt->vec.x, vt->vec.y, vt->vec.z);
//   }
//   if ( ppd->nn ) {
//     display("normal %d\n", ppd->nn);
//     for (nm = ppd->spnm; nm != (Spnm *) NULL; nm = nm->nxt) 
//       display("(no) %d (vec) %g %g %g\n", nm->no,
// 	      nm->vec.x, nm->vec.y, nm->vec.z);
//   }
			
//   if ( ppd->fn ) {
//     display("face %d\n", ppd->fn);
//     for (fc = ppd->spfc; fc != (Spfc *) NULL; fc = fc->nxt) {
//       display("(no) %d \n", fc->no);
//       he = fc->sphe;
//       do {
// 	display("[%d] (v) %d (e) %d\n", he->no,
// 		(he->vt != (Spvt *) NULL) ? he->vt->no : SMDNULL,
// 		(he->ed != (Sped *) NULL) ? he->ed->no : SMDNULL);
//       } while ((he = he->nxt) != fc->sphe);
//     }
//   }
			 
//   if ( ppd->en ) {
//     display("edge %d\n", ppd->en);
//     for (ed = ppd->sped; ed != (Sped *) NULL; ed = ed->nxt) {
//       display("(no) %d type %d sp_type %d (sv) %d (ev) %d (lf) %d (rf) %d\n", ed->no,
// 	      ed->type,
// 	      ed->sp_type,
// 	      (ed->sv  != (Spvt *) NULL) ? ed->sv->no : SMDNULL,
// 	      (ed->ev  != (Spvt *) NULL) ? ed->ev->no : SMDNULL,
// 	      (ed->lf != (Spfc *) NULL) ? ed->lf->no : SMDNULL,
// 	      (ed->rf != (Spfc *) NULL) ? ed->rf->no : SMDNULL);
//     }
//   }
// }
