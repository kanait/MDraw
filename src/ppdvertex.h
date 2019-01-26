//
// ppdvertex.h
//
// Copyright (c) 1997-1999 Takashi Kanai
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
//

#ifndef _PPDVERTEX_H
#define _PPDVERTEX_H

// functions
extern Spvt *create_ppdvertex(Sppd *);
extern Spvt *list_ppdvertex(Sppd *, Id);
extern int num_ppdvertex(Sppd *);
extern void free_ppdvertex(Spvt *, Sppd *);
extern Vted *create_vtxed(Spvt *, Sped *);
extern Spvt *another_vt(Sped *, Spvt *);
extern Vted *find_vtxed(Spvt *, Sped *);
extern double calc_angle(Spvt *, Spvt *, Spvt *);
extern Vted *next_vted(Vted *);
extern Vted *previous_vted(Vted *);
extern void insert_vtxed(Spvt *, Sped *);
extern void make_vtxedlink(Sppd *);
extern void FreeVted(Spvt *, Sped *);
extern void free_ppdvertexedge(Spvt *);
extern void free_vertexedge(Spvt *, Sped *);
extern void free_vtxed(Vted *, Spvt *);
extern Vtnm *create_vtxnm(Spvt *, Spnm *);
extern void free_ppdvertexnormal(Spvt *);
extern void free_vtxnm(Vtnm *, Spvt *);
extern Vtnm *find_ppdnormal(Spvt *, Vec *, Sppd *);
extern void free_ppdvertex_noedge(Sppd *);
extern void ppdvertex_noedge_check(Sppd *);
extern Sphe *ppdvertex_ccwnext_halfedge(Sphe *);
extern Sphe *ppdvertex_cwnext_halfedge(Sphe *);
extern int ppdvertex_num_face(Spvt *);
extern int ppdvertex_num_edge(Spvt *);
extern void reattach_ppdvertex_sphe( Spvt * );
extern void reattach_ppdvertex_halfedge(Sppd *);
extern Sped *ppdvertex_first_edge(Spvt *);
extern Sped *ppdvertex_next_edge(Sped *, Spvt *);
extern Sped *ppdvertex_1ring_ppdedge( Spvt *, Spfc * );

#endif // _PPDVERTEX_H
