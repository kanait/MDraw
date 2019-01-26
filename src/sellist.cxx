//
// sellist.cxx
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

#include "smd.h"
#include "screen.h"
#include "tile.h"

//
// SelList Functions 
//

static SelList *create_sellist( ScreenAtr *screen )
{
  SelList *sl;

  sl = (SelList *) malloc(sizeof(SelList));

  sl->nxt = NULL;
  if (screen->sel_first == (SelList *) NULL) {
    sl->prv = (SelList *) NULL;
    screen->sel_last = screen->sel_first = sl;
  } else {
    sl->prv = screen->sel_last;
    screen->sel_last = screen->sel_last->nxt = sl;
  }

  sl->pid = SMDNULL;
  sl->vt  = NULL;
  sl->fc  = NULL;
  sl->lp  = NULL;
  sl->ted = NULL;
  sl->tf  = NULL;
  
  ++(screen->n_sellist);

  return sl;
}

void free_sellist(SelList *sl, ScreenAtr *screen)
{
  if (sl == (SelList *) NULL) return;
  if (screen == (ScreenAtr *) NULL) return;

  Spvt *vt;
  if ( (vt = sl->vt) != (Spvt *) NULL ) {
    vt->col = PNTGREEN;
  }

  Spfc *fc;
  if ( (fc = sl->fc) != (Spfc *) NULL ) {
    fc->col = FACEBLUE;
  }

  Splp *lp;
  if ( (lp = sl->lp) != (Splp *) NULL ) {
    lp->col = LOOPBLUE;
  }

  TEdge *ted;
  if ( (ted = sl->ted) != NULL ) {
    ted->lp->col = LOOPBLUE;
  }

  TFace *tf;
  if ( (tf = sl->tf) != NULL ) {
    tf->col = FACEBLUE;
  }

  // cut links
  if ( screen->sel_first == sl ) {
    
    if ( (screen->sel_first = sl->nxt) != (SelList *) NULL )
      sl->nxt->prv = (SelList *) NULL;
    else screen->sel_last = (SelList *) NULL;
    
  } else if ( screen->sel_last == sl ) {
    
    sl->prv->nxt = (SelList *) NULL;
    screen->sel_last = sl->prv;
    
  } else {
    
    sl->prv->nxt = sl->nxt;
    sl->nxt->prv = sl->prv;
    
  }

  free( sl );
  --( screen->n_sellist );
}

// create select list (ppd vertex)
SelList *create_selectlist_vt( ScreenAtr * screen, Spvt *vt )
{
  if (vt == (Spvt *) NULL) return (SelList *) NULL;

  SelList *sl = create_sellist( screen );
  sl->vt = vt;
  
  // change color
  vt->col = PNTRED;

  screen->isSelected = TRUE;

  return sl;
}

// create select list (ppd loop)
SelList *create_selectlist_ppdface( ScreenAtr * screen, Spfc *fc )
{
  if (fc == (Spfc *) NULL) return (SelList *) NULL;
  
  SelList *sl = create_sellist( screen );
  sl->fc = fc;
  
  // change color
  fc->col = FACERED;

  screen->isSelected = TRUE;

  return sl;
}

// create select list (ppd loop)
SelList *create_selectlist_lp( ScreenAtr * screen, Splp *lp )
{
  if ( lp == (Splp *) NULL) return (SelList *) NULL;

  SelList *sl = create_sellist( screen );

  sl->lp = lp;
  
  // change color
  lp->col = LOOPRED;

  screen->isSelected = TRUE;

  return sl;
}

// create select list ( tile edge )
SelList *create_selectlist_tedge( ScreenAtr *screen, TEdge *ted )
{
  if ( ted == NULL ) return NULL;

  SelList *sl = create_sellist( screen );

  sl->ted = ted;
  
  // change color
  ted->lp->col = LOOPRED;

  screen->isSelected = TRUE;

  return sl;
}

// create select list ( tile edge )
SelList *create_selectlist_tface( ScreenAtr *screen, TFace *tf )
{
  if ( tf == NULL ) return NULL;

  SelList *sl = create_sellist( screen );

  sl->tf = tf;
  
  // change color
  sl->tf->col = FACERED;

  screen->isSelected = TRUE;

  return sl;
}

SelList *list_selectlist(ScreenAtr * screen, Id id)
{
  SelList *sl;

  for (sl = screen->sel_first; sl != (SelList *) NULL; sl = sl->nxt) {
    if (sl->pid == id)
      return sl;
  }
  return (SelList *) NULL;
}

SelList *list_selectlist_tfc( ScreenAtr* screen, TFace* tfc )
{
  for ( SelList* sl = screen->sel_first; sl != NULL; sl = sl->nxt )
    {
      if ( sl->tf == tfc )
	return sl;
    }
  return NULL;
}

void FreeSelectList( ScreenAtr *screen )
{
  SelList *sl, *nsl;

  if (screen->sel_first == (SelList *) NULL)
    return;

  nsl = (SelList *) NULL;
  for (sl = screen->sel_first; sl != (SelList *) NULL; sl = nsl) {
    nsl = sl->nxt;
    free_sellist( sl, screen );
  }
  screen->sel_first = screen->sel_last = (SelList *) NULL;
  screen->isSelected = FALSE;
  screen->n_sellist = 0;
  
  screen->isAreaSelected = FALSE;
  screen->areaorg.x = 0.0;
  screen->areaorg.y = 0.0;
  screen->areaatv.x = 0.0;
  screen->areaatv.y = 0.0;
}
