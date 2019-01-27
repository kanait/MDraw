//
// pick.cpp
//
// Copyright (c) 1997-1999 Takashi Kanai
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
//

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include <math.h>
#include "smd.h"
#include "screen.h"
#include "draw.h"
#include "pick.h"

#include "ppdvertex.h"
#if 0
#include "tile.h"
#endif

#include "sellist.h"
#include "intsec.h"
#include "veclib.h"

#include "resource.h"
#if 0
#include "TileDialog.h"
#endif

static unsigned short pick_vertex=0;

#if 0
extern CTileDialog *td;
#endif

BOOL edit_pick( int x, int y, int edit_mode, ScreenAtr *screen )
{
  Spvt *vt = NULL;
  Splp *lp = NULL;
  Spfc *fc = NULL;
  //  TEdge *ted = NULL;
  int id = SMDNULL;

  Sppd *ppd = screen->current_ppd;
  //  Tile *tile = screen->current_tile;

  if ( ppd == NULL ) return FALSE;

  switch ( edit_mode ) {

  case EDIT_CREATE_SPATH:

    if ( screen->n_sellist == 2 ) return FALSE;

    if ( (vt = pick_ppdvertex_screen( x, y, screen, ppd )) == NULL ) {
      return FALSE;
    }
    if ( vt->col == PNTRED ) return FALSE;

    vt->col = PNTRED;
    create_selectlist_vt( screen, vt );

    return TRUE;
#if 0
  case EDIT_MAKE_LINES:
    
    if ( (vt = pick_ppdvertex_screen( x, y, screen, ppd )) == NULL ) {
      return FALSE;
    }
    if ( vt->col == PNTRED ) return FALSE;

    vt->col = PNTRED;

    create_selectlist_vt( screen, vt );

    char string[BUFSIZ];
    sprintf( string, "vertex No.%d", vt->no );
    td->ListInsertString( screen->n_sellist - 1, string );

    return TRUE;
#endif
#if 0
  case EDIT_DEL_LINES:

    // tile edge にする
    if ( (ted = pick_tedge_screen( x, y, screen, tile )) == NULL ) {
      return FALSE;
    }
    if ( ted->lp->col == LOOPRED ) return FALSE;

    //FreeSelectList( screen );
    create_selectlist_tedge( screen, ted );

    //char string[BUFSIZ];
    sprintf( string, "tedge No.%d", ted->no );
    td->ListInsertString( screen->n_sellist - 1, string );

    return TRUE;
#endif
#if 0
  case EDIT_MAKE_TILE_FACES:

    // tile edge にする
    if ( (ted = pick_tedge_screen( x, y, screen, tile )) == NULL ) {
      return FALSE;
    }
    if ( ted->lp->col == LOOPRED ) return FALSE;

    //FreeSelectList( screen );
    create_selectlist_tedge( screen, ted );

    //char string[BUFSIZ];
    sprintf( string, "tedge No.%d", ted->no );
    td->ListInsertString( screen->n_sellist - 1, string );

    return TRUE;
#endif
#if 0
  case EDIT_MAKE_TILE_CYL_FACES:

    // tile edge にする
    if ( (ted = pick_tedge_screen( x, y, screen, tile )) == NULL ) {
      return FALSE;
    }
    if ( ted->lp->col == LOOPRED ) return FALSE;

    create_selectlist_tedge( screen, ted );

    //char string[BUFSIZ];
    sprintf( string, "tedge No.%d", ted->no );
    td->ListInsertString( screen->n_sellist - 1, string );

    return TRUE;
#endif
#if 0
  case EDIT_REMESH:

//      // 一つだけしか選択できない
//      if ( screen->isSelected == TRUE ) {
//        FreeSelectList( screen );
//      }

    // tile のピック
    if ( (id = pick_tface_ppd_screen( x, y, screen, ppd )) == SMDNULL ) {
      return FALSE;
    }

    TFace *tf;
    if ( (tf = find_tface( screen->current_tile, id )) == NULL ) {
      return FALSE;
    }

    // すでに選択されている場合は，選択をはずす (トグルボタン形式)
    if ( tf->col == FACEBLUE )
      {

        //display("selected. id %d\n", id );
        create_selectlist_tface( screen, tf );

        // tface の属性設定
        td->set_selected_tile_txt( id );
        td->set_remesh_ndiv( tf->ndiv );
        td->set_remesh_combo_cursel( tf->tptype );
        if ( tf->cv != NULL ) {
          td->set_selected_center_vertex_txt( tf->cv->no );
        } else {
          td->clear_selected_center_vertex_txt();
        }
      }

    else
      {
        SelList *sl = list_selectlist_tfc( screen, tf );
        if ( sl != NULL ) free_sellist( sl, screen );
      }

    return TRUE;
#endif
#if 0
  case EDIT_SEL_CVERTEX:

    // 一つだけしか選択できない
    if ( (vt = pick_ppdvertex_screen( x, y, screen, ppd )) == NULL ) {
      return FALSE;
    }

    vt->col = PNTBLUE;

    td->set_center_vertex( vt );
    td->set_selected_center_vertex_txt( vt->no );
    td->reset_edit_type();

    return TRUE;
#endif

  default:
    break;
  }

  return FALSE;
}

BOOL edit_pick_area( int bx, int by, int ex, int ey,
                     int edit_mode, ScreenAtr *screen )
{
  Spvt *vt = NULL;
  Sppd *ppd = screen->current_ppd;

  if ( ppd == NULL ) return FALSE;

  switch ( edit_mode ) {

  case EDIT_CREATE_SPATH:
    if ( (vt = pick_ppdvertex_screen_area( bx, by, ex, ey, screen, ppd )) == NULL ) {
      return FALSE;
    }
    if ( vt->col == PNTRED ) return FALSE;

    FreeSelectList( screen );

    vt->col = PNTRED;
    create_selectlist_vt( screen, vt );
    return TRUE;
  }

  return FALSE;
}

//////////////////////////////////////////////////////////////////////
// pick hface functions
//////////////////////////////////////////////////////////////////////

#define PICKPOINTSIZE 10

static BOOL pick_on_point( double x, double y, double ix, double iy, double *dis )
{

  double subx, suby;

  subx = x - ix; suby = y - iy;

  if ( (fabs( subx ) < PICKPOINTSIZE) &&
       (fabs( suby ) < PICKPOINTSIZE) ) {
    *dis = sqrt(subx*subx+suby*suby);
    return TRUE;
  }
  return FALSE;
}

Spvt *pick_ppdvertex_screen( int x, int y, ScreenAtr *screen, Sppd *ppd )
{
  if ( ppd == (Sppd *) NULL ) return (Spvt *) NULL;

  get_GL_attributes( screen );

  // added 99/01/21
  Vec eye;
  V3Equal( &eye, &(screen->pers.eye_dir) );

  double ix = (double) x;
  double iy = (double) screen->viewport[3] - (double) y;

  int cnt = 0;
  Spvt *pvt = NULL;

  double dis, dis0;
  Spfc *f;
  for ( f = ppd->spfc; f != (Spfc *) NULL; f = f->nxt ) {

    if ( V3Dot( &eye, &(f->nrm) ) < 0.0 ) continue;

    Sphe *he = f->sphe;
    do {
      Spvt* v = he->vt;
      Vec2d win2;
      world_to_win( &(v->vec), &(win2), screen->mmat, screen->pmat, 
                    screen->viewport );
      // pick.
      if ( pick_on_point( win2.x, win2.y, ix, iy, &dis ) == TRUE ) {
//  display("in.\n");
        if ( cnt ) {
          if ( dis < dis0 ) {
            dis0 = dis;
            pvt  = v;
            ++cnt;
          }
        } else {
          dis0 = dis;
          pvt  = v;
          ++cnt;
        }
      }

    } while ( (he = he->nxt) != f->sphe );
  }

  return pvt;
}

static int sc_in_area(Vec2d * vec, Vec2d * org, Vec2d * atv)
{
  if ((vec->x >= org->x) && (vec->x <= atv->x) &&
      (vec->y <= org->y) && (vec->y >= atv->y))
    return SMD_ON;

  return SMD_OFF;
}

Spvt *pick_ppdvertex_screen_area(int bx, int by, int ex, int ey, ScreenAtr *screen, Sppd *ppd)
{
  Vec2d  win2, vb, ve;
	
  if ( ppd == (Sppd *) NULL ) return (Spvt *) NULL;
	
  get_GL_attributes(screen);
	
  vb.x = (double) bx;
  vb.y = (double) screen->viewport[3] - (double) by;
	
  ve.x = (double) ex;
  ve.y = (double) screen->viewport[3] - (double) ey;
	
  for ( Spvt *v = ppd->spvt; v != (Spvt *) NULL; v = v->nxt ) {
    world_to_win( &(v->vec), &(win2), screen->mmat, screen->pmat,
		  screen->viewport );
    if ( sc_in_area(&(win2), &(vb), &(ve)) ) {
      return v;
    }
  }
	
  return NULL;
}

//////////////////////////////////////////////////////////////////////
// pick hface functions
//////////////////////////////////////////////////////////////////////

// left or right ?
static BOOL V2LR(Vec2d *a, Vec2d *b)
{
  double fg;
	
  fg =a->x * b->y - b->x * a->y;
	
  if ( fg >= 0.0 ) return TRUE;
  else return FALSE;
}

static BOOL point_on_line( Vec2d *v, Vec2d *ev1, Vec2d *ev2, double *dis )
{
  double t;
	
  *dis = point_line_distance_param( v, ev1, ev2, &t );
	
  if ( (*dis < 5) && ( t > 0.0 ) && ( t < 1.0) ) {
    return TRUE;
  } else {
    return FALSE;
  }

}

Splp *pick_ppdloop_screen( int x, int y, ScreenAtr *screen, Sppd *ppd )
{
  int    cnt;
  Splp   *lp, *rlp;
  double dis0, dis;
  Splv   *lv, *nlv;
  Vec2d  ivec, vec1, vec2;
	
  if ( ppd == (Sppd *) NULL ) return (Splp *) NULL;
	
  get_GL_attributes( screen );
	
  ivec.x = (double) x;
  ivec.y = (double) screen->viewport[3] - (double) y;
	
  cnt = 0;
  rlp = NULL;
  for ( lp = ppd->splp; lp != (Splp *) NULL; lp = lp->nxt ) {
		
    nlv = lp->splv;
    lv = nlv->nxt;
		
    world_to_win( &(nlv->vt->vec), &(vec1), screen->mmat, screen->pmat,
		  screen->viewport );
		
    while ( lv != (Splv *) NULL ) {
			
      world_to_win( &(lv->vt->vec), &(vec2), screen->mmat, screen->pmat,
		    screen->viewport );
			
      // pick.
      if ( point_on_line( &(ivec), &(vec1), &vec2, &dis) == TRUE ) {
	if ( cnt ) {
	  if ( dis < dis0 ) {
	    dis0 = dis;
	    rlp = lp;
	    ++cnt;
	  }
	} else {
	  ++cnt;
	  dis0 = dis;
	  rlp = lp;
	}
      }
			
      nlv = lv;
      lv = lv->nxt;
      vec1.x = vec2.x;
      vec1.y = vec2.y;
			
    }
		
  }
	
  return rlp;
}

#if 0
TEdge *pick_tedge_screen( int x, int y, ScreenAtr *screen, Tile *tile )
{
  if ( tile == NULL ) return NULL;
	
  get_GL_attributes( screen );

  Vec2d ivec;
  ivec.x = (double) x;
  ivec.y = (double) screen->viewport[3] - (double) y;
	
  int cnt = 0;
  TEdge *rted = NULL;
  for ( TEdge *ted = tile->sted; ted != NULL; ted = ted->nxt ) {
    
    double dis0, dis;
    Vec2d  vec1, vec2;
    Splp *lp = ted->lp;
    Splv *nlv = lp->splv;
    Splv *lv = nlv->nxt;
		
    world_to_win( &(nlv->vt->vec), &(vec1), screen->mmat, screen->pmat,
		  screen->viewport );
		
    while ( lv != (Splv *) NULL ) {
			
      world_to_win( &(lv->vt->vec), &(vec2), screen->mmat, screen->pmat,
		    screen->viewport );
			
      // pick.
      if ( point_on_line( &(ivec), &(vec1), &vec2, &dis) == TRUE ) {
	if ( cnt ) {
	  if ( dis < dis0 ) {
	    dis0 = dis;
	    rted = ted;
	    ++cnt;
	  }
	} else {
	  ++cnt;
	  dis0 = dis;
	  rted = ted;
	}
      }
			
      nlv = lv;
      lv = lv->nxt;
      vec1.x = vec2.x;
      vec1.y = vec2.y;
			
    }
		
  }
	
  return rted;
}
#endif

static BOOL point_in_ppdface( Spfc *, double, double, ScreenAtr * );

int pick_tface_ppd_screen( int x, int y, ScreenAtr *screen, Sppd *ppd )
{
  if ( ppd == (Sppd *) NULL ) return SMDNULL;
  
  get_GL_attributes( screen );
  Vec eye;
  V3Equal( &eye, &(screen->pers.eye_dir) );
  double ix = (double) x;
  double iy = (double) screen->viewport[3] - (double) y;
  
  for ( Spfc *fc = ppd->spfc; fc != (Spfc *) NULL; fc = fc->nxt ) {

    if ( fc->tile_id == SMDNULL ) continue;
    if ( V3Dot( &eye, &(fc->nrm) ) < 0.0 ) continue;
    
    // 法線方向も評価にいれるようにする
    if( point_in_ppdface( fc, ix, iy, screen ) == TRUE ) {
      return fc->tile_id;
    }

  }
  
  return SMDNULL;
}

// Splp *pick_splp_screen( int x, int y, ScreenAtr *screen, HPpd *hppd )
// {
// 	int    cnt;
// 	HEdge  *hed;
// 	Splp   *lp, *rlp;
// 	double dis0, dis;
// 	Splv   *lv, *nlv;
// 	Vec2d  ivec, vec1, vec2;
	
// 	if ( hppd == (HPpd *) NULL ) return (Splp *) NULL;
	
// 	get_GL_attributes( screen );
	
// 	ivec.x = (double) x;
// 	ivec.y = (double) screen->viewport[3] - (double) y;
	
// 	cnt = 0;
// 	rlp = (Splp *) NULL;
// 	for ( hed = hppd->shed; hed != (HEdge *) NULL; hed = hed->nxt ) {
		
// 		lp = (!screen->no) ? hed->lp1 : hed->lp2;
		
// 		if ( lp != (Splp *) NULL ) {
			
// 			nlv = lp->splv;
// 			lv = nlv->nxt;
			
// 			world_to_win( &(nlv->vt->vec), &(vec1), screen->mmat, screen->pmat,
// 				screen->viewport );
			
// 			while ( lv != (Splv *) NULL ) {
				
// 				world_to_win( &(lv->vt->vec), &(vec2), screen->mmat, screen->pmat,
// 					screen->viewport );

// 				// pick.
// 				if ( point_on_line( &(ivec), &(vec1), &vec2, &dis) == TRUE ) {
// 					if ( cnt ) {
// 						if ( dis < dis0 ) {
// 							dis0 = dis;
// 							rlp = lp;
// 							++cnt;
// 						}
// 					} else {
// 						++cnt;
// 						dis0 = dis;
// 						rlp = lp;
// 					}
// 				}
				
// 				nlv = lv;
// 				lv = lv->nxt;
// 				vec1.x = vec2.x;
// 				vec1.y = vec2.y;
				
// 			}
			
// 		}
// 	}
	
// 	return rlp;
// }

//////////////////////////////////////////////////////////////////////
// pick ppdface functions
//////////////////////////////////////////////////////////////////////

static BOOL point_in_ppdface( Spfc *fc, double x, double y, ScreenAtr *screen )
{
  Sphe *he1, *he2;
  Spvt   *vt1, *vt2;
  Vec2d  v1, v2, sub1, sub0;
  BOOL first_sign, sign;

  he1 = fc->sphe;
  he2 = he1->nxt;

  vt1 = he1->vt; vt2 = he2->vt;
  
  /* first edge */ 
  world_to_win( &(vt1->vec), &(v1), screen->mmat, screen->pmat, screen->viewport );
  world_to_win( &(vt2->vec), &(v2), screen->mmat, screen->pmat,	screen->viewport );
  
  V2Sub( &(v2), &(v1), &sub0 );
  sub1.x = x - v1.x;
  sub1.y = y - v1.y;
  first_sign = V2LR( &(sub0), &(sub1) );

  he1 = he2;
  he2 = he2->nxt;

  do {
    vt1 = he1->vt;
    vt2 = he2->vt;
    world_to_win( &(vt1->vec), &(v1), screen->mmat, screen->pmat,
		  screen->viewport );
    world_to_win( &(vt2->vec), &(v2), screen->mmat, screen->pmat,
		  screen->viewport );
  
    V2Sub( &(v2), &(v1), &sub0 );
    sub1.x = x - v1.x;
    sub1.y = y - v1.y;
    sign = V2LR( &(sub0), &(sub1) );

    if ( sign != first_sign ) {
      return FALSE;
    }

    he1 = he2;
    he2 = he2->nxt;
    
  } while ( he1 != fc->sphe );

  return TRUE;
}    

Spfc *pick_ppdface_screen( int x, int y, ScreenAtr *screen, Sppd *ppd )
{
  Spfc  *fc;
  double ix, iy;
	
  if ( ppd == (Sppd *) NULL ) return (Spfc *) NULL;
	
  get_GL_attributes( screen );
	
  ix = (double) x;
  iy = (double) screen->viewport[3] - (double) y;
  
  for ( fc = ppd->spfc; fc != (Spfc *) NULL; fc = fc->nxt ) {
		
    // 法線方向も評価にいれるようにする
    if( point_in_ppdface( fc, ix, iy, screen ) == TRUE ) {
      return fc;
    }
		
  }
  return NULL;
}
