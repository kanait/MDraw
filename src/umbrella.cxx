//
// umbrella.cxx
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

#include <assert.h>
#include "smd.h"
#include "tile.h"
#include "ppdvertex.h"

//
// unbrella_operator
//
void uvw_umbrella_operator( Spvt *vt, Vec2d *d )
{
  if ( vt == NULL )
    {
      d->x = d->y = 0.0; return;
    }

  if ( vt->isBoundary == TRUE )
    {
      d->x = vt->uvw.x;
      d->y = vt->uvw.y;
      return;
    }

  d->x = d->y = 0.0;
  double frac = 1.0 / (double) vt->ven;
  for ( Vted *vted = vt->svted; vted != NULL; vted = vted->nxt )
    {
      Spvt *avt = another_vt( vted->ed, vt );
      d->x += (frac * avt->uvw.x);
      d->y += (frac * avt->uvw.y);
    }
  d->x -= vt->uvw.x;
  d->y -= vt->uvw.y;
}

//
// 境界の頂点も動かすアンブレラオペレータ
// (thin-plate energy ラプラシアンで使用)
//
void uvw_umbrella_operator_bfree( Spvt *vt, Vec2d *d )
{
  if ( vt == NULL )
    {
      d->x = d->y = 0.0; return;
    }

  d->x = d->y = 0.0;
  double frac = 1.0 / (double) vt->ven;
  for ( Vted *vted = vt->svted; vted != NULL; vted = vted->nxt )
    {
      Spvt *avt = another_vt( vted->ed, vt );
      d->x += (frac * avt->uvw.x);
      d->y += (frac * avt->uvw.y);
    }
  d->x -= vt->uvw.x;
  d->y -= vt->uvw.y;
}

//
// scale-dependant umbrella operator
//
void uvw_sd_umbrella_operator( Spvt *vt, Vec2d *d )
{
  if ( vt == NULL )
    {
      d->x = d->y = 0.0; return;
    }

  if ( vt->isBoundary == TRUE )
    {
      d->x = vt->uvw.x;
      d->y = vt->uvw.y;
      return;
    }

  double sum = 0.0;
  d->x = d->y = 0.0;
  for ( Vted *vted = vt->svted; vted != NULL; vted = vted->nxt )
    {
      Spvt *avt = another_vt( vted->ed, vt );
      d->x += ((avt->uvw.x - vt->uvw.x) / vted->ed->length);
      d->y += ((avt->uvw.y - vt->uvw.y) / vted->ed->length);
      sum += (vted->ed->length);
    }
  d->x *= 2.0;
  d->y *= 2.0;
  d->x /= sum;
  d->y /= sum;
}

//
// unbrella_operator (tfc 版)
//
void tfc_umbrella_operator( Spvt *vt, Vec2d *d )
{
  if ( vt == NULL )
    {
      d->x = d->y = 0.0; return;
    }

  if ( vt->sp_type != SP_VERTEX_NORMAL ) // 境界頂点に関する処理
    {
      d->x = vt->uvw.x;
      d->y = vt->uvw.y;
      return;
    }

  d->x = d->y = 0.0;
  double frac = 1.0 / (double) tfc_num_neighbor( vt );
  for ( Vted *vted = vt->svted; vted != NULL; vted = vted->nxt )
    {
      Spvt *avt = another_vt( vted->ed, vt );
      if ( avt->tile_id == vt->tile_id )
	{
	  d->x += (frac * avt->uvw.x);
	  d->y += (frac * avt->uvw.y);
	}
    }
  d->x -= vt->uvw.x;
  d->y -= vt->uvw.y;
}

//
// 境界の頂点も動かすアンブレラオペレータ
// (thin-plate energy ラプラシアンで使用)
//
void tfc_umbrella_operator_bfree( Spvt *vt, Vec2d *d )
{
  if ( vt == NULL )
    {
      d->x = d->y = 0.0; return;
    }

  d->x = d->y = 0.0;
  int n = 0;
  double frac = 1.0 / (double) tfc_num_neighbor( vt );
  for ( Vted *vted = vt->svted; vted != NULL; vted = vted->nxt )
    {
      Spvt *avt = another_vt( vted->ed, vt );
      if ( avt->tile_id == vt->tile_id )
	{
	  d->x += (frac * avt->uvw.x);
	  d->y += (frac * avt->uvw.y);
	  ++n;
	}
    }
  assert( n == tfc_num_neighbor( vt ) );
  d->x -= vt->uvw.x;
  d->y -= vt->uvw.y;
}

