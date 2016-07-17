//
// matlib.h
//
// Copyright (c) 1997-1999 Takashi Kanai; All rights reserved. 
//

#ifndef _MATLIB_H
#define _MATLIB_H

extern double InvertMatrix(Vmtx, int);
extern Vmtx NewMatrix(int, int);
extern void FreeMatrix(Vmtx, int);
extern void TransposeMatrix(Vmtx, Vmtx, int, int);
extern void MultMatrix(Vmtx, Vmtx, Vmtx, int, int, int);
extern void InvertMatrix4(double *, double *);
extern void ludcmp(Vmtx, int, int *, double *);
extern void lubksb(Vmtx, int, int *, double *);
extern void rot_x(double, Vec *);
extern void rot_y(double, Vec *);
extern void rot_z(double, Vec *);
extern void rot_inv_x(double, Vec *);
extern void rot_inv_y(double, Vec *);
extern void rot_inv_z(double, Vec *);
extern void rot_all(double, double, double, Vec *);
extern void rot_vec(double, Vec *, Vec *);
extern void MultMatrix4(double *, double *, double *);
extern void MakeRotXCoSi(double, double, double *);
extern void MakeRotX(double, double *);
extern void MakeRotY(double, double *);
extern void MakeRotZ(double, double *);
extern void MakeRotVec( double, double, Vec *, double * );
extern void MakeRotInvVec(double, double, Vec *, double *);
extern void MultMatrixVec(double *, Vec *, Vec *);
extern void MultMatrixVecUpdate(double *, Vec *);
extern void EqualMatrix4(double *, double *);
extern void iMakeRotVec(double, Vec *, double * );
extern void iMakeRotInvVec(double, Vec *, double *);
extern void iMultMatrixVec(double *, Vec *);

#endif // _MATLIB_H
