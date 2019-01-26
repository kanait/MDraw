//
// intsec.cxx
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
#include "veclib.h"

#include "quadtree.h"
#include "intsec.h"

//#define SMDZEROEPS2 1.0e-06

//
// point square in-out check
//
int vertex_square( Vec2d *v, QRange *qr )
{
  if ((v->x > qr->xmin) && (v->x < qr->xmax) &&
      (v->y > qr->ymin) && (v->y < qr->ymax))
    return INSIDE;
  if ((fabs(v->x - qr->xmin) < SMDZEROEPS) && (v->x < qr->xmax) &&
      (v->y > qr->ymin) && (v->y < qr->ymax))
    return INSIDE;
  if ((v->x > qr->xmin) && (fabs(v->x - qr->xmax) < SMDZEROEPS) &&
      (v->y > qr->ymin) && (v->y < qr->ymax))
    return INSIDE;
  if ((v->x > qr->xmin) && (v->x < qr->xmax) &&
      (fabs(v->y - qr->ymin) < SMDZEROEPS) && (v->y < qr->ymax))
    return INSIDE;
  if ((v->x > qr->xmin) && (v->x < qr->xmax) &&
      (v->y > qr->ymin) && (fabs(v->y - qr->ymax) < SMDZEROEPS))
    return INSIDE;
  return OUTSIDE;
}

// Last modified: 97/10/28
//
//         o v2
//         |
//TRUE     |     FALSE
//         |
//         o v1
//
BOOL isLeftSide( Vec2d *v1, Vec2d *v2, Vec2d *v )
{
//    display("\tval %g\n",
//  	  (v2->x - v1->x) * (v->y  - v1->y) - 
//  	  (v->x  - v1->x) * (v2->y - v1->y) );
  if ( ((v2->x - v1->x) * (v->y  - v1->y) -
	(v->x  - v1->x) * (v2->y - v1->y)) > 0.0 ) {
    return TRUE;
  } else return FALSE;
}

BOOL isLeftSidePivot( Vec2d *vi, Vec2d *vj, Vec2d *vk )
{
  double val1, val2, val3, val4;
  
  val1 = (vj->x - vi->x) * (vk->y  - vi->y) - (vk->x  - vi->x) * (vj->y - vi->y);
  if ( val1 != 0.0 ) {
    if ( val1 > 0.0 ) return TRUE; else return FALSE;
  }
  //	display("taika! 退化! \n");
  val2 = vj->y - vk->y;
  if ( val2 != 0.0 ) {
    if ( val2 > 0.0 ) return TRUE; else return FALSE;
  }
  val3 = vk->y - vi->y;
  if ( val3 != 0.0 ) {
    if ( val3 > 0.0 ) return TRUE; else return FALSE;
  }
  val4 = vk->x - vj->x;
  if ( val4 != 0.0 ) {
    if ( val4 > 0.0 ) return TRUE; else return FALSE;
  }
	
  return FALSE;
}

// Created: 99/03/18
//
//         o v2
//         |
//TRUE     |     FALSE
//         |
//         o v1
// 三次元平面内の直線に対する，左側判定
BOOL isLeftSide3D( Vec *v1, Vec *v2, Vec *nrm, Vec *v )
{
  Vec l;
  l.x = v2->x - v1->x;
  l.y = v2->y - v1->y;
  l.z = v2->z - v1->z;

  double val = ( (nrm->y * l.z - nrm->z * l.y) * (v->x - v1->x) + 
		 (nrm->z * l.x - nrm->x * l.z) * (v->y - v1->y) + 
		 (nrm->x * l.y - nrm->y * l.x) * (v->z - v1->z) );
//   display("val %g\n", val );
  if ( val > 0.0 ) {
    return TRUE;
  } else return FALSE;
}

// 点が線上にある場合
BOOL isOnLine3D( Vec *v1, Vec *v2, Vec *nrm, Vec *v )
{
  Vec l;
  l.x = v2->x - v1->x;
  l.y = v2->y - v1->y;
  l.z = v2->z - v1->z;

  double val = ( (nrm->y * l.z - nrm->z * l.y) * (v->x - v1->x) + 
		 (nrm->z * l.x - nrm->x * l.z) * (v->y - v1->y) + 
		 (nrm->x * l.y - nrm->y * l.x) * (v->z - v1->z) );
  if ( fabs(val) < SMDZEROEPS ) {
    return TRUE;
  } else return FALSE;
}
  
void swap_sign( int *a, int *b )
{
  int tmp;
  tmp = *a; *a = *b; *b = tmp;
}

void swap_vec2d( Vec2d *vec1, Vec2d *vec2 )
{
  Vec2d tmp;
	
  tmp.x   = vec1->x; tmp.y   = vec1->y;
  vec1->x = vec2->x; vec1->y = vec2->y;
  vec2->x = tmp.x;   vec2->y = tmp.y;
	
}

// BOOL isHGvtLeftSide( HGvt *vi, HGvt *vj, HGvt *vk )
// {
// 	BOOL a;
// 	int cnt = 0, ni, nj, nk, swap = SMD_OFF;
// 	Vec2d uvwi, uvwj, uvwk;
	
// 	ni = vi->sign; nj = vj->sign; nk = vk->sign;
// 	uvwi.x = vi->uvw.x; uvwi.y = vi->uvw.y; 
// 	uvwj.x = vj->uvw.x; uvwj.y = vj->uvw.y; 
// 	uvwk.x = vk->uvw.x; uvwk.y = vk->uvw.y;
	
// 	if ( ni > nj ) {
// 		swap_sign( &ni, &nj );  swap_vec2d( &(uvwi), &(uvwj) );
// 		cnt++;
// 	}
// 	if ( ni > nk ) {
// 		swap_sign( &ni, &nk );  swap_vec2d( &(uvwi), &(uvwk) );
// 		cnt++;
// 	}
// 	if ( nj > nk ) {
// 		swap_sign( &nj, &nk );  swap_vec2d( &(uvwj), &(uvwk) );
// 		cnt++;
// 	}
// 	if ( !(cnt % 2) ) swap = SMD_OFF; else swap = SMD_ON;
	
// 	a = isLeftSidePivot( &(uvwi), &(uvwj), &(uvwk) );
	
// 	if ( a == TRUE ) {
		
// 		if ( swap ) return FALSE;
// 		else        return TRUE;
		
// 	} else {
		
// 		if ( swap ) return TRUE;
// 		else        return FALSE;
		
// 	}
// }

double isLeftVal( Vec2d *v1, Vec2d *v2, Vec2d *v )
{
  return (v2->x - v1->x) * (v->y  - v1->y) -
    (v->x  - v1->x) * (v2->y - v1->y) ;
}

#define Bigger(x, y) ((x > y) ? x : y)
#define Smaller(x, y) ((x < y) ? x : y)
/*
* check the intersection of line segments
*/
// BOOL isHGvtLineSegmentCrossing( HGvt *v1_s, HGvt *v1_e, HGvt *v2_s, HGvt *v2_e )
// {
// 	BOOL s1, e1, s2, e2;
	
// 	s1 = isHGvtLeftSide( v2_s, v2_e, v1_s );
// 	e1 = isHGvtLeftSide( v2_s, v2_e, v1_e );
// 	/*   display("1: %d %d %g %g\n", s1, e1, */
// 	/* 	  isLeftVal( v2_s, v2_e, v1_s ), */
// 	/* 	  isLeftVal( v2_s, v2_e, v1_e ) */
// 	/* 	  ); */
// 	if ( s1 == e1 ) return FALSE;
// 	s2 = isHGvtLeftSide( v1_s, v1_e, v2_s );
// 	e2 = isHGvtLeftSide( v1_s, v1_e, v2_e );
// 	/*   display("2: %d %d\n", s2, e2 ); */
// 	if ( s2 == e2 ) return FALSE;
	
// 	return TRUE;
// }

BOOL isLineSegmentCrossing( Vec2d *v1_s, Vec2d *v1_e, Vec2d *v2_s, Vec2d *v2_e )
{
  BOOL s1 = isLeftSide( v2_s, v2_e, v1_s );
  BOOL e1 = isLeftSide( v2_s, v2_e, v1_e );
//    display("\t--1 %d %d\n", s1, e1 );
  if ( s1 == e1 ) return FALSE;
  BOOL s2 = isLeftSide( v1_s, v1_e, v2_s );
  BOOL e2 = isLeftSide( v1_s, v1_e, v2_e );
//    display("\t--2 %d %d\n", s2, e2 );
  if ( s2 == e2 ) return FALSE;
  return TRUE;
}

void LineIntersectionPoint( Vec2d *v1_s, Vec2d *v1_e,
			    Vec2d *v2_s, Vec2d *v2_e,
			    Vec2d *intsec )
{
  double den, a1, b1, c1, a2, b2, c2;
	
  a1 = v1_s->y - v1_e->y;
  b1 = v1_e->x - v1_s->x;
  c1 = v1_s->x * v1_e->y - v1_s->y * v1_e->x;
  a2 = v2_s->y - v2_e->y;
  b2 = v2_e->x - v2_s->x;
  c2 = v2_s->x * v2_e->y - v2_s->y * v2_e->x;
	
  den = a1 * b2 - b1 * a2;
  intsec->x = (c2 * b1 - c1 * b2 ) / den;
  intsec->y = (c1 * a2 - c2 * a1 ) / den;
	
}

int IntsecLines( Vec2d *v1_s, Vec2d *v1_e, Vec2d *v2_s, Vec2d *v2_e )
{
  if ( FALSE == isLineSegmentCrossing( v1_s, v1_e, v2_s, v2_e ) ) {
    return OUTSIDE;
  }
	
  return INSIDE;
}

//
// v1_s + t (v1_e - v1_s) = v2_s + s (v2_e - v2_s)
//                 ↓
// (v1_e - v1_s) t + (v2_s - v2_e) s = v2_s - v1_s
//                 ↓
// | a    c || t | = | x |
// | b    d || s |   | y |
//
BOOL IntsecLinesParam( Vec2d *v1_s, Vec2d *v1_e, Vec2d *v2_s, Vec2d *v2_e,
		       double *t, double *s )
{
  double a = v1_e->x - v1_s->x;
  double b = v1_e->y - v1_s->y;
  double c = v2_s->x - v2_e->x;
  double d = v2_s->y - v2_e->y;
  double x = v2_s->x - v1_s->x;
  double y = v2_s->y - v1_s->y;
  
  // parallel condition
  double den = a * d - b * c;
  if ( fabs(den) < SMDZEROEPS ) return FALSE;

  // parameter
  *t = (d * x - c * y) / den;
  *s = (-b * x + a * y) / den;
  
  return TRUE;
}

//
// line-line intersection in 3D space
//
// Reference:
// "Intersection of Two Lines in Three-Space", 
// by Ronald Goldman, Graphics Gems I, Academic Press, p.304.
//
// This function solves the following eqation:
// v1_s + t (v1_e - v1_s) = v2_s + s (v2_e - v2_s)
//                 
// - If the lines are parallel, then |(v1_e - v1_s) x (v2_e - v2_s)|^2 = 0
//   and the function returns FALSE, and for convenience, t = s = 0.0;
// - If the lines are skew, s and t represent the parameters of the points 
//   of closet approach.
//
BOOL IntsecLinesParam3D( Vec *v1_s, Vec *v1_e, Vec *v2_s, Vec *v2_e,
			 double *t, double *s )
{
  Vec v1, v2, vc;
  V3Sub( v1_e, v1_s, &v1 );
  V3Sub( v2_e, v2_s, &v2 );
  V3Cross( &v1, &v2, &vc );
  
  double den = vc.x * vc.x + vc.y * vc.y + vc.z * vc.z;
  //display("den %g\n", den );
  // parallel condition
  if ( den < 1.0e-10 ) {
    *t = 0; *s = 0;
    return FALSE;
  }

  Vec v;
  V3Sub( v2_s, v1_s, &v );
  *t = V3Triple( &v, &v2, &vc ) / den;
  *s = V3Triple( &v, &v1, &vc ) / den;
  return TRUE;
}

BOOL isParamsInFace( Spfc *fc, double xparam, double yparam )
{
  Vec2d *uvw0, *uvw1, uvw;
  Sphe *he = fc->sphe;
  do {
    uvw0 = &(he->vt->uvw);
    uvw1 = &(he->nxt->vt->uvw);
    uvw.x = xparam; uvw.y = yparam;
    if ( isLeftSide( uvw0, uvw1, &uvw ) == FALSE ) return FALSE;
  } while ( (he = he->nxt) != fc->sphe );
  
  return TRUE;
}

double point_line_distance_param( Vec2d *v, Vec2d *v0, Vec2d *v1, double *t )
{
  double len;
  Vec2d  sub0, sub1, sub, sub0b;
	
  len = V2DistanceBetween2Points( v0, v1 );
  V2Sub( v0, v, &(sub0) );
  V2Sub( v1, v, &(sub1) );
	
  V2Sub( v, v0, &(sub0b) );
  V2Sub( v1, v0, &(sub) );
	
  *t = V2Dot( &(sub), &(sub0b) ) / V2Dot( &(sub), &(sub) );
	
  return fabs( (sub0.x * sub1.y) - (sub1.x * sub0.y) ) / len;
}

double point_line_distance( Vec2d *v, Vec2d *v0, Vec2d *v1 )
{
  double len;
  Vec2d  sub0, sub1;
	
  len = V2DistanceBetween2Points( v0, v1 );
  V2Sub(v0, v, &(sub0));
  V2Sub(v1, v, &(sub1));
	
  return fabs( sub0.x * sub1.y - sub1.x * sub0.y ) / len;
}

//
// Line-Square Intersection Test
//
int line_square_intersection(QRange *qr, Vec2d *svec, Vec2d *evec)
{
  int    icnt, a, b, c;
  Vec2d  q1, q2;
	
  b = vertex_square(svec, qr);
  c = vertex_square(evec, qr);
  if ((b == INSIDE) && (c == INSIDE)) return INSIDE;
	
  icnt = 0;
	
  if ((svec->x - qr->xmin) * (evec->x - qr->xmin) < 0.0) {
    q1.x = qr->xmin;
    q1.y = qr->ymin;
    q2.x = qr->xmin;
    q2.y = qr->ymax;
    a = IntsecLines(svec, evec, &(q1), &(q2));
    if (a != OUTSIDE) ++icnt;
  }
  if ((svec->x - qr->xmax) * (evec->x - qr->xmax) < 0.0) {
    q1.x = qr->xmax;
    q1.y = qr->ymin;
    q2.x = qr->xmax;
    q2.y = qr->ymax;
    a = IntsecLines(svec, evec, &(q1), &(q2));
    if (a != OUTSIDE) ++icnt;
		
  }
  if ((svec->y - qr->ymin) * (evec->y - qr->ymin) < 0.0) {
    q1.x = qr->xmin;
    q1.y = qr->ymin;
    q2.x = qr->xmax;
    q2.y = qr->ymin;
    a = IntsecLines(svec, evec, &(q1), &(q2));
    if (a != OUTSIDE) ++icnt;
  }
  if ((svec->y - qr->ymax) * (evec->y - qr->ymax) < 0.0) {
    q1.x = qr->xmin;
    q1.y = qr->ymax;
    q2.x = qr->xmax;
    q2.y = qr->ymax;
    a = IntsecLines(svec, evec, &(q1), &(q2));
    if (a != OUTSIDE) ++icnt;
  }
	
  if (icnt == 0) return OUTSIDE;
  else if (icnt == 1) {
    if (b == INSIDE) return INTERSECTION;
    if (c == INSIDE) return INTERSECTION;
    return OUTSIDE;
  } else 
    return INTERSECTION;
}

int vertex_line(Vec2d *v, Vec2d *ev1, Vec2d *ev2)
{
  double t;
  double point_line_distance(Vec2d *, Vec2d *, Vec2d *);
	
  if (point_line_distance(v, ev1, ev2) < SMDZEROEPS) {
    t = (v->x - ev1->x) / (ev2->x - ev1->x);
    if ( ( t > 0.0 ) && ( t < 1.0) ) return INSIDE;
  }
  return OUTSIDE;
}

// 二つのベクトルが平行かどうかをチェック
BOOL isParallelLine( Vec *v1s, Vec *v1e, Vec *v2s, Vec *v2e )
{
  Vec sub1, sub2, c;
  V3Sub( v1s, v1e, &(sub1) );
  V3Sub( v2s, v2e, &(sub2) );
  
  V3Cross( &(sub1), &(sub2), &(c) );
  //double aval = V3Cosign( &(sub1), &(sub2) );

  double val = c.x*c.x+c.y*c.y+c.z*c.z;
//    display("v1s %g %g %g, v1e %g %g %g\n",
//  	  v1s->x, v1s->y, v1s->z, v1e->x, v1e->y, v1e->z );
//    display("v2s %g %g %g, v2e %g %g %g\n", 
//  	  v2s->x, v2s->y, v2s->z, v2e->x, v2e->y, v2e->z );
//    display(" c %g %g %g val %g\n", c.x, c.y, c.z, val );
//    display(" cosign %g\n", aval );
  if ( val < SMDZEROEPS ) return TRUE;
  else return FALSE;
  
}
  
/*
 * given a line through P:(px,py) Q:(qx,qy) and T:(tx,ty)
 * return 0 if T is not on the line through      <--P--Q-->
 *        1 if T is on the open ray ending at P: <--P
 *        2 if T is on the closed interior along:   P--Q
 *        3 if T is on the open ray beginning at Q:    Q-->
 *
 * Example: consider the line P = (3,2), Q = (17,7). A plot
 * of the test points T(x,y) (with 0 mapped onto '.') yields:
 *
 *     8| . . . . . . . . . . . . . . . . . 3 3
 *  Y  7| . . . . . . . . . . . . . . 2 2 Q 3 3    Q = 2
 *     6| . . . . . . . . . . . 2 2 2 2 2 . . .
 *  a  5| . . . . . . . . 2 2 2 2 2 2 . . . . .
 *  x  4| . . . . . 2 2 2 2 2 2 . . . . . . . .
 *  i  3| . . . 2 2 2 2 2 . . . . . . . . . . .
 *  s  2| 1 1 P 2 2 . . . . . . . . . . . . . .    P = 2
 *     1| 1 1 . . . . . . . . . . . . . . . . .
 *      +--------------------------------------
 *        1 2 3 4 5 X-axis 10        15      19
 *
 * Point-Line distance is normalized with the Infinity Norm
 * avoiding square-root code and tightening the test vs the
 * Manhattan Norm. All math is done on the field of integers.
 * The latter replaces the initial ">= MAX(...)" test with
 * "> (ABS(qx-px) + ABS(qy-py))" loosening both inequality
 * and norm, yielding a broader target line for selection.
 * The tightest test is employed here for best discrimination
 * in merging collinear (to grid coordinates) vertex chains
 * into a larger, spanning vectors within the Lemming editor.
 */
#define MAX(a,b)	(((a)>(b))?(a):(b))

BOOL isParamsOnLine( Vec2d *x, Vec2d *v1, Vec2d *v2 )
  //		    int PntOnLine(px,py,qx,qy,tx,ty)
{
  if ( fabs( (v2->y-v1->y)*(x->x-v1->x) - (x->y-v1->y)*(v2->x-v1->x) ) >
       ( MAX(fabs(v2->x-v1->x), fabs(v2->y-v1->y))) )
    return FALSE;
  if (((v2->x<v1->x)&&(v1->x<x->x)) || ((v2->y<v1->y)&&(v1->y<x->y))) return FALSE;
  if (((x->x<v1->x)&&(v1->x<v2->x)) || ((x->y<v1->y)&&(v1->y<v2->y))) return FALSE;
  if (((v1->x<v2->x)&&(v2->x<x->x)) || ((v1->y<v2->y)&&(v2->y<x->y))) return FALSE;
  if (((x->x<v2->x)&&(v2->x<v1->x)) || ((x->y<v2->y)&&(v2->y<v1->y))) return FALSE;
  
  return TRUE;
}

