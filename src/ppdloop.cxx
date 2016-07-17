//
// ppdloop.cxx
//
// Copyright (c) 1997-1999 Takashi Kanai; All rights reserved. 
//

#include "StdAfx.h"

#if defined(_DEBUG) && defined(_WINDOWS)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "smd.h"
#include "ppdvertex.h"
#include "file.h"

#include "ppdloop.h"

//
//  Splp functions
//

Splp *create_splp( void )
{
  Splp *lp;

  lp = (Splp *) malloc(sizeof(Splp));

  lp->nxt = (Splp *) NULL;
  lp->prv = (Splp *) NULL;

  // type
  lp->type = OPENLOOP;
  
  // total loop vertex
  lp->lvn   = 0;

  // corner loop vertex
  lp->lcvn  = 0; 
  lp->splv  = (Splv *) NULL;
  lp->eplv  = (Splv *) NULL;

  // total loop edge
  lp->len   = 0;
  lp->sple  = (Sple *) NULL;
  lp->eple  = (Sple *) NULL;

  //   lp->hed   = (HEdge *) NULL;
  lp->col   = LOOPBLUE;

  // time
  lp->realtime = 0.0;
  lp->usertime = 0.0;
  lp->systime = 0.0;

  // the number of iterations
  lp->num_iter = 0;

  return lp;
}

/* loop */
Splp *create_ppdloop( Sppd *ppd )
{
  Splp *lp;

  lp = (Splp *) malloc(sizeof(Splp));

  lp->nxt = (Splp *) NULL;
  if (ppd->splp == (Splp *) NULL) {
    lp->prv = (Splp *) NULL;
    ppd->eplp = ppd->splp = lp;
  } else {
    lp->prv   = ppd->eplp;
    ppd->eplp = ppd->eplp->nxt = lp;
  }

  /*  total loop vertex */
  lp->lcvn  = 0; 
  lp->lvn   = 0;
  lp->splv  = (Splv *) NULL;
  lp->eplv  = (Splv *) NULL;

  /*  total loop edge */
  lp->len   = 0;
  lp->sple  = (Sple *) NULL;
  lp->eple  = (Sple *) NULL;

  /* id */
  lp->no   = ppd->lid;
  
  // open flag
  lp->type = OPENLOOP;

  /* links to _hedge */
//   lp->hed  = (HEdge *) NULL;
  
  lp->col   = LOOPBLUE;
  
  ++( ppd->ln );
  ++( ppd->lid );
  
  return lp;
}

Splp *find_ppdloop( Sppd *ppd, Spvt *vt )
{
  Splv *lv;
  Splp *lp;
  
  for ( lp = ppd->splp; lp != (Splp *) NULL; lp = lp->nxt ) {

    for ( lv = lp->splv; lv != (Splv *) NULL; lv = lv->nxt ) {
      if ( lv->vt == vt ) return lp;
    }
  }
  return (Splp *) NULL;
}

void free_ppdvertex_splp( Splp *lp, Sppd *ppd )
{
  for ( Splv *lv = lp->splv; lv != NULL; lv = lv->nxt ) {
    if ( lv->vt->type == VERTEX_ADDED ) 
      free_ppdvertex( lv->vt, ppd );
  }
}

void free_splp( Splp *lp )
{
  if (lp == (Splp *) NULL) return;
  
  // free loopvertex
  Splv *lv, *nlv;
  for (lv = lp->splv; lv != (Splv *) NULL; lv = nlv) {

    nlv = lv->nxt;
    // delete ppdvertex if vt->type == VERTEX_ADDED
    free_ppdloopvertex( lv, lp );
  }
  
  // free loopedge
  Sple *le, *nle;
  for (le = lp->sple; le != (Sple *) NULL; le = nle) {
    nle = le->nxt;
    free_ppdloopedge(le, lp);
  }
  
  free( lp );
}

void free_ppdloop(Splp *lp, Sppd *ppd)
{
  if (lp == (Splp *) NULL) return;
  
  // free loopvertex
  Splv *lv, *nlv;
  for (lv = lp->splv; lv != (Splv *) NULL; lv = nlv) {
    nlv = lv->nxt;
    free_ppdloopvertex(lv, lp);
  }
  
  // free loopedge
  Sple *le, *nle;
  for (le = lp->sple; le != (Sple *) NULL; le = nle) {
    nle = le->nxt;
    free_ppdloopedge(le, lp);
  }
  
  if (ppd->splp == lp) {
    if ((ppd->splp = lp->nxt) != (Splp *) NULL)
      lp->nxt->prv = (Splp *) NULL;
    else {
      ppd->eplp = (Splp *) NULL;
    }
  } else if (ppd->eplp == lp) {
    lp->prv->nxt = (Splp *) NULL;
    ppd->eplp = lp->prv;
    
  } else {
    lp->prv->nxt = lp->nxt;
    lp->nxt->prv = lp->prv;
  }
  
  free( lp );
  --( ppd->ln );
}

// loop vertex
Splv *create_ppdloopvertex( Splp *loop )
{
  Splv *lv;

  lv = (Splv *) malloc(sizeof(Splv));
  
  lv->nxt = (Splv *) NULL;
  if (loop->splv == (Splv *) NULL) {
    lv->prv = (Splv *) NULL;
    loop->eplv = loop->splv = lv;
  } else {
    lv->prv    = loop->eplv;
    loop->eplv = loop->eplv->nxt = lv;
  }

  // id
  lv->no   = loop->lvn;
  
  // vertex
  lv->vt  = (Spvt *) NULL;
  
  // vertex type
  lv->type = LOOPVT_CORNER;

  ++( loop->lvn );

  return lv;
}

Splv *find_ppdloopvertex( Splp *lp, Spvt *vt )
{
  Splv *lv;

  if ( lp == (Splp *) NULL ) return (Splv *) NULL;
  
  for ( lv = lp->splv; lv != (Splv *) NULL; lv = lv->nxt ) {
    if ( lv->vt == vt ) return lv;
  }
  return (Splv *) NULL;
}

void free_ppdloopvertex( Splv *lv, Splp *loop )
{
  if (lv == (Splv *) NULL) return;

  //lv->vt->lp = (Splp *) NULL;
  
  if (loop->splv == lv) {
    if ((loop->splv = lv->nxt) != (Splv *) NULL)
      lv->nxt->prv = (Splv *) NULL;
    else {
      loop->eplv = (Splv *) NULL;
    }
  } else if (loop->eplv == lv) {
    lv->prv->nxt = (Splv *) NULL;
    loop->eplv = lv->prv;
    
  } else {
    lv->prv->nxt = lv->nxt;
    lv->nxt->prv = lv->prv;
  }
  
  free(lv);
  --( loop->lvn );
}

// loop edge
Sple *create_ppdloopedge(Splp *loop)
{
  Sple *le;

  le = (Sple *) malloc(sizeof(Sple));
  
  le->nxt = (Sple *) NULL;
  if (loop->sple == (Sple *) NULL) {
    le->prv = (Sple *) NULL;
    loop->eple = loop->sple = le;
  } else {
    le->prv    = loop->eple;
    loop->eple = loop->eple->nxt = le;
  }

  // id
  le->no   = loop->len;
  
  // edge
  le->ed  = (Sped *) NULL;
  
  ++( loop->len );

  return le;
}

void free_ppdloopedge(Sple *le, Splp *loop)
{
  if (le == (Sple *) NULL) return;
  
  if ( loop->sple == le ) {
    if ((loop->sple = le->nxt) != (Sple *) NULL)
      le->nxt->prv = (Sple *) NULL;
    else {
      loop->eple = (Sple *) NULL;
    }
  } else if ( loop->eple == le ) {

    le->prv->nxt = (Sple *) NULL;
    loop->eple = le->prv;
    
  } else {
    
    le->prv->nxt = le->nxt;
    le->nxt->prv = le->prv;
    
  }
  
  free( le );
  --( loop->len );
}

Splp *reverse_ppdloop( Splp *lp )
{
  Splp *nlp;
  Splv *lv, *nlv;
  Splp *create_splp( void );
  
  nlp = create_splp();
  nlp->type  = SHORTESTPATH;

  lv = lp->eplv;
  while (1) {
    
    nlv = create_ppdloopvertex( nlp );
    nlv->vt = lv->vt;

    if ( lv == lp->splv ) break;
    lv = lv->prv;

  }
  
  return nlp;
}

// lp1 に lp2 を追加
void add_splp( Splp *lp1, Splp *lp2 )
{
  if ( lp2 == NULL ) return;
  for ( Splv *lv2 = lp2->splv; lv2 != NULL; lv2 = lv2->nxt ) {

    if ( lp1->eplv != NULL ) {
      if ( lp1->eplv->vt != lv2->vt ) {
	Splv *lv1 = create_ppdloopvertex( lp1 );
	lv1->vt = lv2->vt;
	lv1->type = lv2->type;
      }
    } else {
      Splv *lv1 = create_ppdloopvertex( lp1 );
      lv1->vt = lv2->vt;
      lv1->type = lv2->type;
    }
      
  }
  //    display("lp1 vn %d\n", lp1->lvn );
}

//////////////////////////////////////////////////////////////////
// for .sp file
//////////////////////////////////////////////////////////////////

// sp file output
void write_sp_file( Splp *lp, char *fname, char *ppdfname )
{
  FILE *fp;

  if ((fp = fopen(fname, "w")) == NULL) {
    return;
  }

  // header
  fprintf(fp, "#ppd %s\n", ppdfname );
  fprintf(fp, "esp\n");

  for ( Splv *lv = lp->splv; lv != NULL; lv = lv->nxt ) {
    fprintf(fp, "\t%d %g %g %g\n", lv->vt->no,
	    lv->vt->vec.x,
	    lv->vt->vec.y,
	    lv->vt->vec.z );
  }
  fprintf(fp, "end\n");
  fclose( fp );
}

Splp *open_sp( char *fname, Sppd *ppd )
{
  FILE *fp;
  int mode = PPD_NONE;

  if ( ppd == NULL ) return NULL;

  if ((fp = fopen(fname, "r")) == NULL) return NULL;

  // read .sp file
  char	buf[BUFSIZ];
  char	key[BUFSIZ], val[3][BUFSIZ];
  Splp *lp;
  Spvt *vt;
  Splv *lv;
  int  n;
  lp = create_splp();
  while (fgets(buf, BUFSIZ, fp)) {
    
    sscanf(buf, "%s", key);
    
    if (comment(buf[0])) continue;
    
    else if ( !strcmp(key, "end") ) {
      
      mode = PPD_NONE;
      
    } else if ( mode == PPD_LOOP ) {
      
      sscanf( buf, "%s%s%s%s", key, val[0], val[1], val[2] );
      n = atoi( key );

      if ( n >= ppd->nvt_org ) {

	// new vertex
	vt = create_ppdvertex( ppd );
	vt->type = VERTEX_ADDED;
	vt->vec.x = atof( val[0] );
	vt->vec.y = atof( val[1] );
	vt->vec.z = atof( val[2] );
	
      } else {
	
	// vertex has been already defined
	vt = list_ppdvertex( ppd, n );
	
      }

      // loop vertex
      lv = create_ppdloopvertex( lp );
      lv->vt = vt;
      
    } else if ( mode == PPD_NONE ) {
      
      if ( !strcmp(key, "esp") ) {
	
	mode = PPD_LOOP;
	
      }
    }
  }
  
  fclose( fp );

  return lp;

}  

