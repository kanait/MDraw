//
// dspr.cxx
//  
// Copyright (c) 1997-1999 Takashi Kanai; All rights reserved.
//
#include "StdAfx.h"

#if defined(_DEBUG) && defined(_WINDOWS)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "smd.h"
#include "sigproc.h"
#include "dspr.h"

SparseMatrix *init_sparsematrix( int num )
{
  SparseMatrix *sm;

  sm = (SparseMatrix *) malloc( sizeof(SparseMatrix) );
  sm->ncol = sm->nrow = num;

  return sm;
}

void free_sparsematrix( SparseMatrix *sm )
{
  free( sm->sa );
  free( sm->ija );
  free( sm );
}


