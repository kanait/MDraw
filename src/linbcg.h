//
// linbcg.h
// from Numerical Recipes in C 
// Copyright (c) 1997-1999 Takashi Kanai; All rights reserved. 
//

#ifndef _LINBCG_H
#define _LINBCG_H

// matrix structure for linbcg
typedef struct _semat {

  // matrix row
  int    rnum;
  
  // matrix column
  int    cnum;

  // the dimension of vectors ( = cnum )
  int    num;

  // index
  int    *ija;

  // value
  double *sa;
  
  // constant vector array
  double *bx, *by, *bz;
  
  // solution vector array
  double *xx, *yy, *zz;
  
} Semat;

extern int linbcg(Semat *, double *, double *, int, double, int, int *, double *);
extern double snrm(int, double *, int);
extern void atimes(Semat *, int, double *, double *, int);
extern void asolve(Semat *, int, double *, double *, int);
// extern void dsprsin(HGfc *, Semat *); 
extern void dsprsax(double *, int *, double *, double *, int);
extern void dsprstx(double *, int *, double *, double *, int);
extern double *dvector(long, long);
extern void free_dvector(double *, long, long);

extern Semat *init_emat(int);
extern void free_emat(Semat *);

extern Semat *init_emat3(int);
extern void free_emat3(Semat *);

extern void printemat(Semat *);

#endif // _LINBCG_H
