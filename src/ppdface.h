//
// ppdface.h
//
// Copyright (c) 1997-1999 Takashi Kanai
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
//

#ifndef _PPDFACE_H
#define _PPDFACE_H

// functions
extern Spfc *create_ppdface(Sppd *);
extern Spvt *other_vertices_face_left(Spfc *, Spvt *);
extern Spvt *other_vertices_face_right(Spfc *, Spvt *);
extern void free_ppdface(Spfc *, Sppd *);
extern void free_isolated_ppdface(Spfc *, Sppd *);
extern int num_ppdface(Sppd *);
extern Sphe *ppdface_opposite_halfedge(Spfc *, Spvt *);
extern void ppdfacenormal( Sppd * );
extern void calc_fnorm( Spfc * );
extern Sphe *create_ppdhalfedge(Spfc *);
extern Sphe *free_ppdhalfedge(Sphe *, Spfc *);
extern Sphe *free_isolated_ppdhalfedge(Sphe *, Spfc *);
extern Spfc *find_ppdface_ppdedge(Sped *, Spvt *, Spvt *);
extern void ppdface_to_plane(Spfc *, double *, double *, double *, double *);
extern void ppdhalfedge_to_vec(Sphe *, Vec *);
extern void ppdface_find_barycentric_coordinate(Spfc *, double, double, double *, double *, double *);
extern void ppdface_barycentric_coordinate(Spfc *, double, double, double, Vec *);
extern void ppdface_uv_to_coord( Spfc*, Vec2d*, Vec* );
extern void ppdface_triangulation( Spfc *, Sppd * );
extern void ppdface_calc_barycentric_coordinates( Spfc *, Vec2d *, 
						  double *, double *, double * );
extern double calc_farea( Spfc * );

#endif // _PPDFACE_H
