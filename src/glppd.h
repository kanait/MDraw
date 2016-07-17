//
// glppd.h
//
// Copyright (c) 1997-1999 Takashi Kanai; All rights reserved. 
//

#ifndef _GLPPD_H
#define _GLPPD_H

extern void drawppd_vertex(Sppd *, ScreenAtr *);
extern void drawppd_vertex_enhanced(Sppd *, ScreenAtr *);
extern void drawppd_vertexid(Sppd *, ScreenAtr *);
extern void drawppd_wireframe( Sppd *, ScreenAtr * );
extern void drawppd_edge(Sppd *, ScreenAtr *, double * );
extern void drawppd_edgeid(Sppd *, ScreenAtr *);
extern void drawppd_hiddenline(Sppd *, ScreenAtr *);
extern void drawppd_edge_enhanced(Sppd *, ScreenAtr *);
extern void drawppd_shading(Sppd *, ScreenAtr *);
extern void drawppd_loop(Sppd *, ScreenAtr *);
extern void drawppd_loop_enhanced(Sppd *, ScreenAtr *);
extern void draw_loop(Splp *);
extern void draw_splp(Splp *, GLdouble *, GLfloat );
extern void draw_splp_enhanced(Splp *, GLdouble *, GLfloat, ScreenAtr * );
//extern void draw_splp_enhanced(Splp *, ScreenAtr *);
extern void draw_sged( SGraph *, ScreenAtr *, GLdouble *, GLfloat );
extern void draw_sglp(SGlp *);
extern void draw_sglp_enhanced(SGlp *, ScreenAtr *);
extern void draw_loop_enhanced(Splp *, ScreenAtr * );
extern void drawppd_subgraph( Sppd *, ScreenAtr * );
extern void glu_sphere(Vec *, double);
extern void glu_cylinder(Vec *, Vec *, double);
extern void material_binding(Material *);

#endif // _GLPPD_H
