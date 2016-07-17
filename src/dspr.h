//
// dspr.h
//
// Copyright (c) 1997-1999 Takashi Kanai; All rights reserved. 
//

#ifndef _DSPR_H
#define _DSPR_H

extern SparseMatrix *init_sparsematrix(int);
extern void free_sparsematrix(SparseMatrix *);
extern void dsprsax(double *, int *, double *, double *, int);

#endif // _DSPR_H
