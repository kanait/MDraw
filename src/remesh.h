//
// remesh.h
//
// Copyright (c) 1999 Takashi Kanai
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
//

#ifndef _REMESH_H
#define _REMESH_H

// remesh におけるパラメータ補正のタイプ
#define DIFF_GEODIS   0   // 測地線による補正
#define DIFF_CHORD    1   // 弦長による補正
#define DIFF_NO       2   // 補正しない

// functions
extern Sppd *tolatticeppd(Sppd *, int, int);
extern Sppd *remL_init(int, Spvt **);
extern void remL_internal(Splp *, Vec2d *, int, Spvt **);
extern void remL_internal_update(Splp *, Vec2d *, int, Vec2d *, Vec *, int);
extern void remL_boundary(Splp *, Vec2d *, int, Spvt **);
extern void remL_corner(Splp *, Vec2d *, int, Spvt **);
extern double uv_geodesic_distance_old(Vec2d *, Vec2d *, Sped *, Spfc *);
extern double uv_geodesic_distance(Sped *, Spfc *, int, int, Vec2d *, int);
extern double uv_chord_length(int, int, Vec *, int);
extern void uv_isoparam_diff_old(Splp *, int, Vec2d *, Vec2d *, Vec *, int);
extern double uv_isoparam_diff(Splp *, int, Vec2d *, Vec2d *, Vec *, int, int);
extern Sped *ppd_find_boundaryedge(Splp *, double, double);
extern Sped *ppd_find_nextedge(Spfc *, Sped *, Vec2d *, Vec2d *);
extern Sped *ppd_find_nextedge_intsec(Spfc *, Sped *, Vec2d *, Vec2d *, Vec2d *);
extern Sppd *toradialppd(Sppd *, int);
extern void remL_reparam(Sppd *, Sppd *, Vec2d *);
extern Spfc *remL_find_ppdface(Sppd *, Vec2d *);

#endif // _REMESH_H
