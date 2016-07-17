//
// gltile.cpp
//  
// Copyright (c) 1999 by Takashi Kanai; All rights researved.
//

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include <math.h>
#include "gl\gl.h"
#include "gl\glu.h"

#include "smd.h"
#include "screen.h"
#include "tile.h"
#include "glppd.h"
#include "draw.h"
#include "sgraph.h"

#ifndef _COLOR_H
#include "color.h"
#endif

#ifndef _MTL_H
#include "mtl.h"
#endif

// for displaying sgraph edge
#define VOFFSET 0.001

static Material line_mat[] = {
  0.187004f, 0.138930f, 0.138930f, 1.0f,
  0.748016f, 0.75721f,  0.155721f, 1.0f,
  0.0f, 0.0f, 0.0f, 1.0f,
  0.836364f, 0.836364f, 0.836364f, 1.0f,
  77.575806f
};

static Material pointred_mat[] = {
  0.187004f, 0.0f, 0.0f, 1.0f,
  0.748016f, 0.0f, 0.0f, 1.0f,
  0.0f, 0.0f, 0.0f, 1.0f,
  0.672727f, 0.672727f, 0.672727f, 1.0f,
  29.478785f
};

static Material point_mat[] = {
  0.005183f, 0.084911f, 0.084911f, 1.0f,
  0.020732f, 0.339645f, 0.339645f, 1.0f,
  0.0f, 0.0f, 0.0f, 1.0f,
  0.903030f, 0.903030f, 0.903030f, 1.0f,
  118.690941f
};

static Material vertex_mat[] = {
  0.0f, 0.187004f, 0.187004f, 1.0f, 
  0.0f, 0.748016f, 0.748016f, 1.0f, 
  0.0f, 0.0f, 0.0f, 1.0f, 
  0.981818f, 0.981818f, 0.981818f, 1.0f, 
  92.315140f
};

static Material edge_mat[] = {
  0.038095f, 0.036285f, 0.036285f, 1.0f, 
  0.152381f, 0.145141f, 0.145141f, 1.0f, 
  0.0f, 0.0f, 0.0f, 1.0f, 
  0.745455f, 0.745455f, 0.745455f, 1.0f, 
  122.569725f
};

void draw_tvertex_enhanced( Tile *tile, ScreenAtr *screen )
{
  if ( tile == NULL ) return;
	
  glEnable(GL_LIGHTING);
	
  TVertex *tvt;
  for ( tvt = tile->stvt; tvt != NULL; tvt = tvt->nxt ) {
		
    //material_binding( tropical20 );
    material_binding( point_mat );
    if ( tvt->vt->col == PNTRED ) material_binding( pointred_mat );
    
    glu_sphere( &(tvt->vt->vec), screen->rad_sph );
    
  }
	
  glDisable(GL_LIGHTING);
}

//
// タイルの境界
// 
void draw_tile_tedge( Tile *tile, ScreenAtr *screen )
{
  TEdge *ted;
	
  if ( tile == NULL ) return;

  for ( ted = tile->sted; ted != NULL; ted = ted->nxt ) {
    if ( swin->dis3d.enhanced == FALSE ) {
      draw_splp( ted->lp, blackvec, 3.0 );
    } else {
      draw_splp_enhanced( ted->lp, blackvec, 5.0, screen );
    }
  }
}

//
// 選択用
//
void draw_tile_tface_boundary( Tile *tile, ScreenAtr *screen )
{
  if ( tile == NULL ) return;
	
  for ( TFace *tfc = tile->stfc; tfc != NULL; tfc = tfc->nxt ) {
    if ( tfc->col == FACEBLUE ) continue;
    draw_splp( tfc->clp, redvec, 6.0 );
  }
}

void draw_tile_tface_remeshppd( Tile *tile, ScreenAtr *screen )
{
  for ( TFace *tfc = tile->stfc; tfc != NULL; tfc = tfc->nxt ) {
    if ( tfc->rppd == NULL ) continue;
    //display("remesh.\n");
    drawppd_shading( tfc->rppd, screen );
    drawppd_wireframe( tfc->rppd, screen );
  }
}
  
