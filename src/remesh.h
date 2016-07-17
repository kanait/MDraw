//
// remesh.h
// 
// Copyright (c) 1999 Takashi Kanai; All rights reserved. 
//

#ifndef _REMESH_H
#define _REMESH_H

// functions
extern Sppd *tolatticeppd(Sppd *, int);
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
