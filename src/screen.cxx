//
// screen.cxx
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
#include "draw.h"
#include "tile.h"

#ifndef _COLOR_H
#include "color.h"
#endif

#include "veclib.h"

// screen initialize
void screenatr_initialize( ScreenAtr *screen, int width, int height )
{
  // width, height
  screen->width  = width;
  screen->height = height;

  // center
  screen->xcenter = (double) screen->width  / 2.0;
  screen->ycenter = (double) screen->height / 2.0;
  
  // resize or not
  screen->isResized = FALSE;

  // zoom parameter
  screen->isZoom = FALSE;
  screen->zoom = 1.0;

  // rotate parameter
  screen->isRotate = FALSE;
  screen->rotate_type = ROTATE_XY;
  screen->rotate_x = 0.0;
  screen->rotate_y = 0.0;
  screen->rotate_z = 0.0;

  // background color
//    screen->bgrgb[0] = (float) bisquevec[0];
//    screen->bgrgb[1] = (float) bisquevec[1];
//    screen->bgrgb[2] = (float) bisquevec[2];
  screen->bgrgb[0] = (float) blackvec[0];
  screen->bgrgb[1] = (float) blackvec[1];
  screen->bgrgb[2] = (float) blackvec[2];

  // set view initialize
  set_default_view( screen );
  
  // for select area
  screen->isAreaSelected  = FALSE;
  screen->areaorg.x = 0.0;
  screen->areaorg.y = 0.0;
  screen->areaatv.x = 0.0;
  screen->areaatv.y = 0.0;

  // select list
  screen->isSelected = FALSE;
  screen->n_sellist  = 0;
  screen->sel_first = screen->sel_last = (SelList *) NULL;

  // for current_ppd
  screen->current_ppd = (Sppd *) NULL;
  screen->view_ppd = (Sppd *) NULL;

  // for current tile
  screen->current_tile = (Tile *) NULL;

  // for shortest-path
  screen->view_sg   = (SGraph *) NULL;
  screen->view_sglp = (SGlp *) NULL;
  screen->alp = (Splp *) NULL;
  screen->elp = (Splp *) NULL;
  screen->alpary = (Splp **) NULL;
  screen->elpary = (Splp **) NULL;

  // enhanced display parameter
  //screen->rad_sph = 0.015;
  //screen->rad_sph = 0.025;
  screen->rad_sph = 0.003;
  screen->rad_cyl = 0.0015;
}

// rotate 3D
void start_screen3d_rotate( ScreenAtr *screen )
{
  screen->isRotate = TRUE;
}

void update_screen3d_rotate(ScreenAtr *screen, int dx, int dy)
{
  screen->isRotate = TRUE;
  screen->rotate_type   = ROTATE_XY;
  screen->rotate_x = (double) dx;
  screen->rotate_y = (double) dy;
}

void update_screen3d_rotate_z(ScreenAtr *screen, double dz)
{
	screen->isRotate = TRUE;
  screen->rotate_type   = ROTATE_Z;
  screen->rotate_z      = dz;
  screen->zrot_rad      += (dz * SMDPI / 180.0);
}

void finish_screen3d_rotate(ScreenAtr *screen)
{
  screen->isRotate = FALSE;
}

/* zoom 3D */

void start_screen3d_zoom(ScreenAtr *screen)
{
  screen->isZoom = TRUE;
}

void update_screen3d_zoom(ScreenAtr *screen, int dx, int dy)
{
  screen->isZoom = TRUE;
  screen->zoom += ((float) 5.0e-03 * (dx - dy));
  if (screen->zoom < 1.0e-04) screen->zoom = (float) 1.0e-04;
  
}

void finish_screen3d_zoom(ScreenAtr *screen)
{
  screen->isZoom = FALSE;
}

void update_screen3d_translate(ScreenAtr *screen, int dx, int dy)
{
//  double tx, ty, tz;
  Vec vec1, vec2, vec3;
  
  screen->isRotate = TRUE;
  V3Scr( (double) -dx / ( 500.0 * screen->zoom),
	 &(screen->rotate_xvec), &vec1 );
  V3Scr( (double) dy  / ( 500.0 * screen->zoom),
	 &(screen->rotate_yvec), &vec2 );
  V3Add(&vec1, &vec2, &vec3);
  V3Add(&(screen->translate_vec), &vec3, &(screen->translate_vec));
}

void finish_screen3d_translate( ScreenAtr *screen )
{
  screen->isRotate = FALSE;
}
