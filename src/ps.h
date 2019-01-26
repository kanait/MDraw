//
// ps.h
//
// Copyright (c) 1997-1999 Takashi Kanai
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
//

#ifndef _PS_H
#define _PS_H

extern FILE *open_ps_ppd( char *, Sppd * );
extern FILE *open_ps(char *);
extern void close_ps(FILE *);
extern void pstran(Vec2d *, Vec2d *);
extern void pstran_p(Vec2d *, Vec2d *);
extern void pslinewidth(FILE *, double);
extern void psoffsetwidth(FILE *, double);
extern void pscolor(FILE *, double, double, double);
extern void pscircle( FILE*, Vec2d*, double );
extern void psline(FILE *, Vec2d *, Vec2d *);
extern void psline_4values(FILE *, double, double, double, double);
extern void psdashline(FILE *, Vec2d *, Vec2d *);
extern void psdashline2(FILE *, Vec2d *, Vec2d *);
extern void psstar(FILE *, Vec2d *);
extern void psrectangle(FILE *, Vec2d *, double);
extern void psfillrectangle(FILE *, Vec2d *);
extern void psstar_d(FILE *, Vec2d *, double);
extern void psmark(FILE *, Vec2d *);
extern void pstext(FILE *, Vec2d *, char *);
extern void pstextbold(FILE *, Vec2d *, char *);
extern void psprintzip(FILE *, Vec2d *, char *);
extern void psline_p(FILE *, Vec2d *, Vec2d *);
extern void psdashline_p(FILE *, Vec2d *, Vec2d *);
extern void psdashline2_p(FILE *, Vec2d *, Vec2d *);
extern void psstar_p(FILE *, Vec2d *);
extern void psstar_d_p(FILE *, Vec2d *, double);
extern void psmarp(FILE *, Vec2d *);
extern void pstext_p(FILE *, Vec2d *, char *);
extern void harmonicmap_to_ps(char *, Sppd *);
extern void psharmonicppdedge(FILE *, Sppd *);
extern void psppdedgeA(FILE *, Sppd *);
extern void psppdedge(FILE *, Sppd *, double, int);
extern void psppdvertex(FILE *, Sppd *, double, int);
extern void psmakeboundingbox( Sppd *, int *, int *, int *, int * );

#endif // _PS_H
