//
// smd.cpp
//
// Copyright (c) 1997-1998 by Takashi Kanai; All rights reserved. 
//

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "smd.h"
#include "screen.h"
#include "smdsrc.h"
#include "file.h"

#include "ppd.h"
#include "sgraph.h"

#include "tile.h"
#include "remesh.h"

#include "params.h"

#ifndef _COLOR_H
#include "color.h"
#endif

#include "spm.h"

Params params;

// screen size per a window
#define SCREEN_WIDTH  600
#define SCREEN_HEIGHT 600
#define WOFFSET 12
#define HOFFSET 69
//  #define WOFFSET 0
//  #define HOFFSET 0

Swin *create_swin( void )
{
  Swin *swin;

  swin = (Swin *) malloc( sizeof(Swin) );

  swin->width  = SCREEN_WIDTH  + WOFFSET;
  swin->height = SCREEN_HEIGHT + HOFFSET;

  display3d_initialize( &(swin->dis3d) );

  // left window (src)
  screenatr_initialize( &(swin->screenatr), SCREEN_WIDTH, SCREEN_HEIGHT );
  swin->screenatr.no  = SCREEN_SRC;

  /* material initialize */

  swin->material = material_initialize();

  // select type initialize 
  swin->select_type = SEL_CLEAR;
  swin->edit_type   = EDIT_NONE;

  // parameterization type
  swin->param_type = SP_TPWEIGHT;
    
  // re-calc parameterization (used in "Remesh")
  swin->recalc_param = FALSE;

  // parameter correction type (used in "Remesh")
  swin->paramcorr_type = DIFF_GEODIS;
  
  // save harmonic map to ps 
  swin->isSaveHMtoPS = FALSE;

  swin->isSaveBMPImage = FALSE;
  swin->savebmpid = 0;

  // sp params
  params_init( &params );
#ifdef _DEBUG
  params.print = TRUE;
#endif

#ifdef WIN32
  params.save_path = TRUE;
#endif
  
  // display to file
  display_file_open("output.log");
  
  return swin;
}

void display3d_initialize( Disp3D *disp )
{
  disp->vertex     = TRUE;
  
  disp->wire       = FALSE;
  disp->shading    = TRUE;
  disp->hidden     = FALSE;
  
  disp->loop       = FALSE;
  disp->coaxis     = FALSE;
  disp->texture    = FALSE;
//    disp->texture    = TRUE;

  disp->group_mesh = TRUE;
  disp->remesh     = TRUE;
  disp->remeshb    = TRUE;
  
  disp->vertexid   = FALSE;
  disp->edgeid     = FALSE;
  
  disp->spath      = FALSE;
  disp->subgraph   = FALSE;
  
  disp->enhanced   = FALSE;
  disp->smooth     = FALSE;
  disp->gradient   = TRUE;
  disp->spath_anim = FALSE;

  // light
  disp->light0     = TRUE;
  disp->light1     = TRUE;
  disp->light2     = FALSE;
  disp->light3     = FALSE;
  
}

Material *material_initialize( void )
{
  Material *matl;

  matl = (Material *) malloc(sizeof(Material));

//    matl->ambient[0] = 0.2f;
//    matl->ambient[1] = 0.2f;
//    matl->ambient[2] = 0.2f;
  matl->ambient[0] = 0.2f;
  matl->ambient[1] = 0.2f;
  matl->ambient[2] = 0.2f;
  matl->ambient[3] = 1.0f;
  matl->diffuse[0] = 0.8f;
  matl->diffuse[1] = 0.8f;
  matl->diffuse[2] = 0.8f;
  matl->emission[0] = 0.0f;
  matl->emission[1] = 0.0f;
  matl->emission[2] = 0.0f;
  matl->emission[3] = 1.0f;
  matl->specular[0] = 0.1f;
  matl->specular[1] = 0.1f;
  matl->specular[2] = 0.1f;
  matl->specular[3] = 1.0f;
  matl->shininess[0] = 40.0f;

  return matl;
}

void swin_free_ppd( Swin *swin )
{
  // free currnt_tile
  if ( swin->screenatr.current_tile != (Tile *) NULL ) 
    free_tile( swin->screenatr.current_tile );
  
  // free current ppd
  if ( swin->screenatr.current_ppd != (Sppd *) NULL ) {
    Sppd *ppd = swin->screenatr.current_ppd;
    if ( ppd->sg != NULL ) {
      free_sgraph( ppd->sg );
      ppd->sg = NULL;
    }
    free_ppd( ppd );
  }
}

void free_swin( Swin *swin )
{
  display_file_close();  

  swin_free_ppd( swin );
  
  free( swin->material );
  free( swin );
}
