//
// sigproc.h
//
// Copyright (c) 1997-2000 Takashi Kanai; All rights reserved. 
//

#ifndef _SIGPROC_H
#define _SIGPROC_H

// matrix structure for linbcg
typedef struct _sparsematrix {
  int nrow;	// matrix row
  int ncol;	// matrix column
  int narray;   // number of array
  int *ija;
  double *sa;
} SparseMatrix;

extern void ppd_signal_processing(Sppd *, double, double, int);
extern SparseMatrix *create_spmatrix_dspr(Sppd *, double *, double);
extern double *edge_weight(Sppd *);

extern void uvw_signal_processing(Sppd *, double, double, int);
extern SparseMatrix *create_spmatrix_dspr_const(Sppd *, double *, double);

#endif // _SIGPROC_H
