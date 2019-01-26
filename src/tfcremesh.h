//
// tfcremesh.h
//
// Copyright (c) 2000 Takashi Kanai
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
//

#ifndef _TFCREMESH_H
#define _TFCREMESH_H

typedef struct _plarray PLArray;
typedef struct _paramline ParamLine;

struct _plarray {

  // parameter line
  // parameter line $B$N?t(B = (div+1) + (div+1)
  // XPARAM $B$NJ}$+$i@h$K3JG<$9$k(B
  int n_line;
  ParamLine* line;

};

#define PL_INTERNAL 0
#define PL_BOUNDARY 1

struct _paramline {
  
  // PL_INTERNAL or PL_BOUNDARY
  int type;
  
  // $BD:E@Ns(B
  int n_vt;
  Spvt** vt;
  // pvt $B$NHV9f(B
  int* vt_id;
  
  // $B:G=i$ND:E@$,$N$C$F$$$k%(%C%8(B, $BLL(B
  Sped* ed;
  Spfc* fc;
  
};

// functions
extern Sppd *tolatticemesh(TFace *, int, int );
extern Sppd *tfc_remL_init(int, Spvt**, PLArray* );
extern void tfc_remL_internal(TFace *, Vec2d *, int, Spvt**, PLArray* );
extern void tfc_remL_internal_update(TFace *, int, Vec2d *, Vec *, PLArray*, int);
extern void tfc_remL_boundary(TFace *, Vec2d *, int, Spvt**, PLArray* );
extern void tfc_remL_corner(TFace *, Vec2d *, int, Spvt**);
extern double tfc_uv_geodesic_distance( ParamLine*, TFace *, Vec2d * );
extern double tfc_uv_chord_length( ParamLine*, Vec* );
extern double tfc_uv_isoparam_diff(TFace*, int, Vec2d *, Vec *, PLArray*, int);
extern Sppd *toradialmesh(TFace *, int);
extern Spfc *remesh_find_intsecface(TFace *, Spvt *, Vec2d *, Vec2d *);
extern Sped *remesh_find_intsecedge(TFace *, Spfc *, Spvt *);
extern Sped *remesh_find_boundaryedge(TFace *, Splp *, double, double);
extern Sped *remesh_nextedge(TFace *, Spfc *, Sped *, Vec2d *, Vec2d *, Vec2d *);
extern BOOL remesh_isParamsInFace(TFace *, Spfc *, double, double);
extern void remesh_uv_to_coord(TFace *, Spfc *, Vec2d *, Vec *);
extern void remesh_find_barycentric_coordinate(TFace *, Spfc *, double, double, double *, double *, double *);
extern PLArray* init_plarray( void );
extern void init_paramline( ParamLine* );
extern void free_plarray( PLArray* );

#endif // _TFCREMESH_H
