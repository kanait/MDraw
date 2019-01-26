//
// gltile.h
//
// Copyright (c) 1999 by Takashi Kanai
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
//

#ifndef _GLTILE_H
#define _GLTILE_H

extern void draw_tvertex_enhanced( Tile *, ScreenAtr * );
extern void draw_tile_tedge( Tile *, ScreenAtr * );
extern void draw_tile_tface_boundary( Tile *, ScreenAtr * );
extern void draw_tile_tface_remeshppd( Tile *, ScreenAtr * );

#endif // _GLTILE_H
