//
// ps.cxx
// functions for Adobe Postsrcipt file
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

#include <math.h>
#include "smd.h"
#include "ps.h"

#define WINDOWCENTERX 250.0
#define WINDOWCENTERY 400.0
#define PARA_WINDOWCENTERX 100
#define PARA_WINDOWCENTERY 450

#define RADIUS 200.0

//
// open Postscript file with ppd (for bounding box)
//
FILE *open_ps_ppd( char *fname, Sppd *ppd )
{
  FILE *fp;
	
  if ((fp = fopen(fname, "w")) == NULL) {
    return NULL;
  }
  int xmin, xmax, ymin, ymax;
  psmakeboundingbox( ppd, &xmin, &ymin, &xmax, &ymax );
  printf("xmin %d ymin %d xmax %d ymax %d\n", xmin, ymin, xmax, ymax );
  
  fprintf(fp, "%%!PS-Adobe-2.0\n");
  fprintf(fp, "%%%%BoundingBox: %d %d %d %d\n", xmin, ymin, xmax, ymax );
  fprintf(fp, "%%EndComments\n");
  fprintf(fp, "\n");
  fprintf(fp, "gsave\n");
  fprintf(fp, "\n");
  fprintf(fp, "/l {moveto lineto [] 0 setdash stroke} def\n");
  fprintf(fp, "/sprint {moveto /Times-Roman findfont 3 scalefont setfont show} def\n");
  fprintf(fp, "/sprintbold {moveto /Times-Bold findfont 25 scalefont setfont show} def\n");
//      fprintf(fp, "/sprint2 {moveto /Courier findfont 15 scalefont setfont show} def\n");
//      fprintf(fp, "/printZip {0 0 moveto /Helvetica-BoldOblique findfont 30 scalefont setfont show} def\n");
  fprintf(fp, "/d {moveto lineto [4 2] 0 setdash stroke} def\n");
  fprintf(fp, "/d2 {moveto lineto [2 2] 0 setdash stroke} def\n");
//      fprintf(fp, "%%/star {moveto lineto [] 0 setdash starcolor setrgbcolor stroke} def\n");
//      fprintf(fp, "/star {\n");
//      fprintf(fp, " /factor 0.5 def\n");
//      fprintf(fp, " /y1 exch def /x1 exch def /y0 exch def /x0 exch def\n");
//      fprintf(fp, " /ym y0 y1 add 2 div def /xm x0 x1 add 2 div def\n");
//      fprintf(fp, " /yd y0 y1 sub 2 div factor mul def\n");
//      fprintf(fp, " /xd x0 x1 sub 2 div factor mul def\n");
//      fprintf(fp, " xm xd add ym yd add moveto\n");
//      fprintf(fp, " xm xd sub ym yd sub lineto\n");
//      fprintf(fp, "[] 0 setdash starcolor setrgbcolor stroke\n");
//      fprintf(fp, "} def\n");
	
//      fprintf(fp, "/linecolor {0 0 0} def\n");
//      fprintf(fp, "/markcolor {0 0 0} def\n");
//      fprintf(fp, "/starcolor {0 0 0} def\n");

    fprintf(fp, "/offset 3 def\n");
    fprintf(fp, "/f {2 copy 4 copy\n");
    fprintf(fp, "  offset sub exch offset sub exch moveto\n");
    fprintf(fp, "  offset add exch offset sub exch lineto\n");
    fprintf(fp, "  offset add exch offset add exch lineto\n");
    fprintf(fp, "  offset sub exch offset add exch lineto\n");
    fprintf(fp, "  closepath\n");
    /*    fprintf(fp, "  0 setgray\n"); */
//      fprintf(fp, "  markcolor setrgbcolor\n");
    fprintf(fp, "  fill\n");
    fprintf(fp, "} def\n");
    
    fprintf(fp, "newpath\n");
    //fprintf(fp, "1.0 0.0 0.0 setrgbcolor\n");
    //fprintf(fp, "%f setlinewidth\n", 0.001);
    fprintf(fp, "\n");
	
    return fp;
}

//
// open Postscript file
//
FILE *open_ps( char *fname )
{
  FILE *fp;
	
  if ((fp = fopen(fname, "w")) == NULL) {
    return NULL;
  }
  
  fprintf(fp, "%%!\n");
    fprintf(fp, "/l {moveto lineto [] 0 setdash linecolor setrgbcolor stroke} def\n");
    fprintf(fp, "/sprint {moveto /NewCenturySchlbk-Italic findfont 3 scalefont setfont show} def\n");
    fprintf(fp, "/sprintbold {moveto /NewCenturySchlbk-BoldItalic findfont 25 scalefont setfont show} def\n");
    fprintf(fp, "/sprint2 {moveto /Courier findfont 15 scalefont setfont show} def\n");
    fprintf(fp, "/printZip {0 0 moveto /Helvetica-BoldOblique findfont 30 scalefont setfont show} def\n");
    fprintf(fp, "/d {moveto lineto [4 2] 0 setdash linecolor setrgbcolor stroke} def\n");
    fprintf(fp, "/d2 {moveto lineto [2 2] 0 setdash linecolor setrgbcolor stroke} def\n");
    fprintf(fp, "%%/star {moveto lineto [] 0 setdash starcolor setrgbcolor stroke} def\n");
    fprintf(fp, "/star {\n");
    fprintf(fp, " /factor 0.5 def\n");
    fprintf(fp, " /y1 exch def /x1 exch def /y0 exch def /x0 exch def\n");
    fprintf(fp, " /ym y0 y1 add 2 div def /xm x0 x1 add 2 div def\n");
    fprintf(fp, " /yd y0 y1 sub 2 div factor mul def\n");
    fprintf(fp, " /xd x0 x1 sub 2 div factor mul def\n");
    fprintf(fp, " xm xd add ym yd add moveto\n");
    fprintf(fp, " xm xd sub ym yd sub lineto\n");
    fprintf(fp, "[] 0 setdash starcolor setrgbcolor stroke\n");
    fprintf(fp, "} def\n");
	
    fprintf(fp, "/linecolor {0 0 0} def\n");
    fprintf(fp, "/markcolor {0 0 0} def\n");
    fprintf(fp, "/starcolor {0 0 0} def\n");
    fprintf(fp, "/offset 3 def\n");
    fprintf(fp, "/f {2 copy 4 copy\n");
    fprintf(fp, "  offset sub exch offset sub exch moveto\n");
    fprintf(fp, "  offset add exch offset sub exch lineto\n");
    fprintf(fp, "  offset add exch offset add exch lineto\n");
    fprintf(fp, "  offset sub exch offset add exch lineto\n");
    fprintf(fp, "  closepath\n");
    /*    fprintf(fp, "  0 setgray\n"); */
    fprintf(fp, "  markcolor setrgbcolor\n");
    fprintf(fp, "  fill\n");
    fprintf(fp, "} def\n");
    fprintf(fp, "newpath\n");
    fprintf(fp, "%f setlinewidth\n", 0.001);
    fprintf(fp, "\n");
	
    return fp;
}

/* close post-script file */
void close_ps( FILE * fp )
{
  fprintf(fp, "grestore\n");
  fprintf(fp, "\n");
  fprintf(fp, "showpage");
  fclose(fp);
}

/* transformation from 2D point to PS point */
void pstran( Vec2d * p1, Vec2d * p2 )
{
  p2->x = WINDOWCENTERX + p1->x;
  p2->y = WINDOWCENTERY + p1->y;
}

/* transformation from 2D point to PS point in parameter space */
void pstran_p(Vec2d * p1, Vec2d * p2)
{
  p2->x = PARA_WINDOWCENTERX + p1->x;
  p2->y = PARA_WINDOWCENTERY + p1->y;
}

/* set line-width */
void pslinewidth(FILE * fp, double width)
{
    fprintf(fp, "%.4f setlinewidth\n", width);
}

/* eps offset-width */
void psoffsetwidth(FILE * fp, double d)
{
    fprintf(fp, "/offset %.4f def\n", d);
}

/* eps line color */

void pscolor(FILE * fp, double r, double g, double b)
{
  fprintf(fp, "%g %g %g setrgbcolor\n", r, g, b);
}

/* eps mark color */

/*void 
psmarkcolor (ChrP color)
{
double rgb[3];

 psrgbcolor(color, rgb);
 fprintf(fp, "/markcolor {%.4f %.4f %.4f} def\n", 
 rgb[0], rgb[1], rgb[2]);
   } */

/* eps star color */

/*void 
psstarcolor (ChrP color)
{
double rgb[3];

 psrgbcolor(color, rgb);
 fprintf(fp, "/starcolor {%.4f %.4f %.4f} def\n", 
 rgb[0], rgb[1], rgb[2]);
   } */

/********************************* 2D *********************************/

// print line
void pscircle( FILE *fp, Vec2d *center, double rad )
{
  int div = 100;

  pslinewidth( fp, 0.01 );
  for ( int i = 1; i <= div; ++i )
    {
      Vec2d p1, p2;
      double theta0 = 2 * SMDPI * ((double) (i-1) / (double) div);
      double theta1 = 2 * SMDPI * ((double) i / (double) div);
      p1.x = center->x + rad * cos( theta0 );
      p1.y = center->y + rad * sin( theta0 );
      p2.x = center->x + rad * cos( theta1 );
      p2.y = center->y + rad * sin( theta1 );
  
      Vec2d q1, q2;
      q1.x = WINDOWCENTERX + RADIUS * p1.x;
      q1.y = WINDOWCENTERY + RADIUS * p1.y;
      q2.x = WINDOWCENTERX + RADIUS * p2.x;
      q2.y = WINDOWCENTERY + RADIUS * p2.y;
      fprintf(fp, "%f %f %f %f l\n", q1.x, q1.y, q2.x, q2.y);
    }
}

void psline(FILE * fp, Vec2d *p1, Vec2d *p2)
{
  Vec2d q1, q2;
  q1.x = WINDOWCENTERX + RADIUS * p1->x;
  q1.y = WINDOWCENTERY + RADIUS * p1->y;
  q2.x = WINDOWCENTERX + RADIUS * p2->x;
  q2.y = WINDOWCENTERY + RADIUS * p2->y;
  fprintf(fp, "%f %f %f %f l\n", q1.x, q1.y, q2.x, q2.y);
}

void psline_4values(FILE * fp, double x1, double y1, double x2, double y2)
{
    fprintf(fp, "%f %f %f %f l\n", x1, y1, x2, y2);
}

//
// print dashline
//
void psdashline( FILE * fp, Vec2d *p1, Vec2d *p2 )
{
  Vec2d v1, v2;
  v1.x = WINDOWCENTERX + RADIUS * p1->x;
  v1.y = WINDOWCENTERY + RADIUS * p1->y;
  v2.x = WINDOWCENTERX + RADIUS * p2->x;
  v2.y = WINDOWCENTERY + RADIUS * p2->y;
  fprintf(fp, "%g %g %g %g d\n", v1.x, v1.y, v2.x, v2.y);
}

//
// print dashline 2
//
void psdashline2( FILE *fp, Vec2d *p1, Vec2d *p2 )
{
  Vec2d q1, q2;
  q1.x = WINDOWCENTERX + RADIUS * p1->x;
  q1.y = WINDOWCENTERY + RADIUS * p1->y;
  q2.x = WINDOWCENTERX + RADIUS * p2->x;
  q2.y = WINDOWCENTERY + RADIUS * p2->y;
  fprintf(fp, "%f %f %f %f d2\n", q1.x, q1.y, q2.x, q2.y);
}

/* print star */

void psstar(FILE * fp, Vec2d * p)
{
    Vec2d c;
    Vec2d c1, c2;
	
    pstran(p, &c);
    c1.x = c.x - 5.0;
    c2.x = c.x + 5.0;
    c1.y = c.y - 5.0;
    c2.y = c.y + 5.0;
    fprintf(fp, "%.6f %.6f %.6f %.6f star\n", c1.x, c1.y, c2.x, c2.y);
    c1.x = c.x - 5.0;
    c2.x = c.x + 5.0;
    c1.y = c.y + 5.0;
    c2.y = c.y - 5.0;
    fprintf(fp, "%.6f %.6f %.6f %.6f star\n", c1.x, c1.y, c2.x, c2.y);
}

/* print rectangle */

void psrectangle( FILE * fp, Vec2d * p, double len )
{
  Vec2d q;
  q.x = WINDOWCENTERX + RADIUS * p->x;
  q.y = WINDOWCENTERY + RADIUS * p->y;
  Vec2d c1, c2;
  c1.x = q.x - len;
  c2.x = q.x - len;
  c1.y = q.y - len;
  c2.y = q.y + len;
  fprintf(fp, "%.6f %.6f %.6f %.6f l\n", c1.x, c1.y, c2.x, c2.y);
  c1.x = q.x - len;
  c2.x = q.x + len;
  c1.y = q.y - len;
  c2.y = q.y - len;
  fprintf(fp, "%.6f %.6f %.6f %.6f l\n", c1.x, c1.y, c2.x, c2.y);
  c1.x = q.x - len;
  c2.x = q.x + len;
  c1.y = q.y + len;
  c2.y = q.y + len;
  fprintf(fp, "%.6f %.6f %.6f %.6f l\n", c1.x, c1.y, c2.x, c2.y);
  c1.x = q.x + len;
  c2.x = q.x + len;
  c1.y = q.y - len;
  c2.y = q.y + len;
  fprintf(fp, "%.6f %.6f %.6f %.6f l\n", c1.x, c1.y, c2.x, c2.y);
}

void psfillrectangle(FILE * fp, Vec2d *p)
{
  Vec2d q;
  q.x = WINDOWCENTERX + RADIUS * p->x;
  q.y = WINDOWCENTERY + RADIUS * p->y;
  fprintf(fp, "%.6f %.6f f\n", q.x, q.y);
}

/* print star with adaptive sacale */
void psstar_d(FILE * fp, Vec2d * p, double d)
{
    Vec2d c;
    Vec2d c1, c2;
	
    pstran(p, &c);
    c1.x = c.x - 5.0 * d;
    c2.x = c.x + 5.0 * d;
    c1.y = c.y - 5.0 * d;
    c2.y = c.y + 5.0 * d;
    fprintf(fp, "%.6f %.6f %.6f %.6f star\n", c1.x, c1.y, c2.x, c2.y);
    c1.x = c.x - 5.0 * d;
    c2.x = c.x + 5.0 * d;
    c1.y = c.y + 5.0 * d;
    c2.y = c.y - 5.0 * d;
    fprintf(fp, "%.6f %.6f %.6f %.6f star\n", c1.x, c1.y, c2.x, c2.y);
}

/* print mark */
void psmark(FILE * fp, Vec2d * p)
{
    Vec2d c;
	
    pstran(p, &c);
    fprintf(fp, "%.6f %.6f f\n", c.x, c.y);
}

/* print text */
void pstext(FILE *fp, Vec2d *p, char *text)
{
  Vec2d q;
  q.x = WINDOWCENTERX + RADIUS * p->x;
  q.y = WINDOWCENTERY + RADIUS * p->y;
  fprintf(fp, "(%s) %.6f %.6f sprint\n", text, q.x, q.y);
}

/* print text */
void
pstextbold(FILE * fp, Vec2d * p, char *text)
{
    Vec2d c;
	
    pstran(p, &c);
    c.x += 2.0;
    c.y += 2.0;
    fprintf(fp, "(%s) %.6f %.6f sprintbold\n", text, c.x, c.y);
}

void psprintzip(FILE * fp, Vec2d * p, char *text)
{
    Vec2d c;
	
    pstran(p, &c);
    c.x += 2.0;
    c.y += 2.0;
    fprintf(fp, "%.0f %.0f translate\n", c.x, c.y);
    fprintf(fp, ".95 -.05 0\n");
    fprintf(fp, "{setgray (%s) printZip -0.2 .2 translate} for\n",
		text);
    fprintf(fp, "gsave 1 setgray printZip grestore\n");
    fprintf(fp, "\n");
	
}

/********************* in parameter space ************************/
/* print line in parameter space */
void psline_p(FILE * fp, Vec2d * p1, Vec2d * p2)
{
    Vec2d c1, c2;
	
    pstran_p(p1, &c1);
    pstran_p(p2, &c2);
    fprintf(fp, "%.6f %.6f %.6f %.6f l\n", c1.x, c1.y, c2.x, c2.y);
}

/* print dashline in parameter space */
void psdashline_p(FILE * fp, Vec2d * p1, Vec2d * p2)
{
    Vec2d c1, c2;
	
    pstran_p(p1, &c1);
    pstran_p(p2, &c2);
    fprintf(fp, "%.6f %.6f %.6f %.6f d\n", c1.x, c1.y, c2.x, c2.y);
}

/* print dashline 2 in parameter space */
void psdashline2_p(FILE * fp, Vec2d * p1, Vec2d * p2)
{
    Vec2d c1, c2;
	
    pstran_p(p1, &c1);
    pstran_p(p2, &c2);
    fprintf(fp, "%.6f %.6f %.6f %.6f d2\n",
		c1.x, c1.y, c2.x, c2.y);
}

/* print star in parameter space */
void psstar_p(FILE * fp, Vec2d * p)
{
    Vec2d c1, c2;
	
    c1.x = p->x - 0.01;
    c2.x = p->x + 0.01;
    c1.y = p->y - 0.01;
    c2.y = p->y + 0.01;
    pstran_p(&c1, &c1);
    pstran_p(&c2, &c2);
    fprintf(fp, "%.6f %.6f %.6f %.6f star\n", c1.x, c1.y, c2.x, c2.y);
    c1.x = p->x - 0.01;
    c2.x = p->x + 0.01;
    c1.y = p->y + 0.01;
    c2.y = p->y - 0.01;
    pstran_p(&c1, &c1);
    pstran_p(&c2, &c2);
    fprintf(fp, "%.6f %.6f %.6f %.6f star\n", c1.x, c1.y, c2.x, c2.y);
}

/* print star with adaptive sacale in parameter space */
void psstar_d_p(FILE * fp, Vec2d * p, double d)
{
    Vec2d c;
    Vec2d c1, c2;
	
    c1.x = p->x - 0.01 * d * 1.0;
    c2.x = p->x + 0.01 * d * 1.0;
    c1.y = p->y - 0.01 * d * 1.0;
    c2.y = p->y + 0.01 * d * 1.0;
    pstran_p(&c1, &c1);
    pstran_p(&c2, &c2);
    pstran_p(p, &c);
    fprintf(fp, "%.6f %.6f %.6f %.6f star\n", c1.x, c1.y, c2.x, c2.y);
    c1.x = p->x - 0.01 * d * 1.0;
    c2.x = p->x + 0.01 * d * 1.0;
    c1.y = p->y + 0.01 * d * 1.0;
    c2.y = p->y - 0.01 * d * 1.0;
    pstran_p(&c1, &c1);
    pstran_p(&c2, &c2);
    fprintf(fp, "%.6f %.6f %.6f %.6f star\n", c1.x, c1.y, c2.x, c2.y);
}

/* print mark in parameter space */
void psmarp(FILE * fp, Vec2d * p)
{
    Vec2d c;
	
    pstran_p(p, &c);
    fprintf(fp, "%.6f %.6f f\n", c.x, c.y);
}

/* print text in parameter space */
void pstext_p(FILE * fp, Vec2d * p, char *text)
{
    Vec2d c;
	
    pstran_p(p, &c);
    c.x += 2.0;
    c.y += 2.0;
    fprintf(fp, "(%s) %.6f %.6f sprint\n", text, c.x, c.y);
}

void harmonicmap_to_ps(char *file, Sppd * ppd)
{
  FILE *fp;
  FILE *open_ps( char * );
	
  if ((fp = open_ps(file)) == (FILE *) NULL)  return;
	
  psharmonicppdedge(fp, ppd);
	
  close_ps(fp);
}

void psharmonicppdedge(FILE * fp, Sppd *ppd)
{
  pslinewidth( fp, 0.01 );
  for ( Sped *ed = ppd->sped; ed != NULL; ed = ed->nxt ) {
    Vec2d uvw1, uvw2;
    uvw1.x = WINDOWCENTERX + RADIUS * ed->sv->uvw.x;
    uvw1.y = WINDOWCENTERY + RADIUS * ed->sv->uvw.y;
    uvw2.x = WINDOWCENTERX + RADIUS * ed->ev->uvw.x;
    uvw2.y = WINDOWCENTERY + RADIUS * ed->ev->uvw.y;
    psline(fp, &(uvw1), &(uvw2));
  }
}

void psppdedgeA(FILE * fp, Sppd *ppd)
{
  pslinewidth( fp, 0.1 );
  for ( Sped *ed = ppd->sped; ed != NULL; ed = ed->nxt ) {
    Vec2d uvw1, uvw2;
    uvw1.x = ed->sv->vec.x;
    uvw1.y = ed->sv->vec.y;
    uvw2.x = ed->ev->vec.x;
    uvw2.y = ed->ev->vec.y;
    //printf("%g %g %g %g\n", uvw1.x, uvw1.y, uvw2.x, uvw2.y );
    psline(fp, &(uvw1), &(uvw2));
  }
}

//
// width: 線の太さ
// type: 線のタイプ
//       1: 実線
//       2: 点線
//
void psppdedge( FILE *fp, Sppd *ppd, double width, int type )
{
  pslinewidth( fp, width );
  for ( Sped *ed = ppd->sped; ed != NULL; ed = ed->nxt ) {
    if ( type == 1 ) {
      psline( fp, &(ed->sv->uvw), &(ed->ev->uvw) );
    } else if ( type == 2 ) {
      psdashline2( fp, &(ed->sv->uvw), &(ed->ev->uvw) );
    }
//      if ( ed->no == 86 ) {
    Vec2d p;
    p.x = ( ed->sv->uvw.x + ed->ev->uvw.x ) / 2.0;
    p.y = ( ed->sv->uvw.y + ed->ev->uvw.y ) / 2.0;
    char string[BUFSIZ];
    sprintf( string, "E%d", ed->no );
    pstext( fp, &p, string );
//      }
  }
}

//
// radius: 線の半径
// type: 点のタイプ
//       1: rectangle
//       2: filled rectangle
//
void psppdvertex( FILE *fp, Sppd *ppd, double radius, int type )
{
  for ( Spvt *vt = ppd->spvt; vt != NULL; vt = vt->nxt ) {

    if ( type == 1 ) {
      psrectangle( fp, &(vt->uvw), radius );
    } else if ( type == 2 ) {
      psfillrectangle( fp, &(vt->uvw) );
    }

    Vec2d p;
    p.x = vt->uvw.x;
    p.y = vt->uvw.y;
    char string[BUFSIZ];
    sprintf( string, "v%d", vt->no );
    pstext( fp, &p, string );
  }
}

void psmakeboundingbox( Sppd *ppd, int *xmin, int *ymin, int *xmax, int *ymax )
{
  double dxmin = WINDOWCENTERX - 1.2 * RADIUS;
  double dymin = WINDOWCENTERY - 1.2 * RADIUS;
  double dxmax = WINDOWCENTERX + 1.2 * RADIUS;
  double dymax = WINDOWCENTERY + 1.2 * RADIUS;

  printf("dymin %g dymax %g\n", dymin, dymax );
  Spvt *v;
  int i;
  for ( i = 0, v = ppd->spvt; v != NULL; ++i, v = v->nxt )
    {
      Vec2d uvw;
      uvw.x = WINDOWCENTERX + RADIUS * v->vec.x;
      uvw.y = WINDOWCENTERY + RADIUS * v->vec.y;
      if (i)
	{
	  if ( uvw.x > dxmax ) dxmax = uvw.x;
	  if ( uvw.x < dxmin ) dxmin = uvw.x;
	  if ( uvw.y > dymax ) dymax = uvw.y;
	  if ( uvw.y < dymin ) dymin = uvw.y;
	}
      else
	{
//  	  dxmax = dxmin = uvw.x;
//  	  dymax = dymin = uvw.y;
	}
    }

  printf("dymin %g dymax %g\n", dymin, dymax );
  *xmin = (int) (dxmin - 10.0);
  *xmax = (int) (dxmax + 10.0);
  *ymin = (int) (dymin - 10.0);
  *ymax = (int) (dymax + 10.0);
}

//
// end of file
//
