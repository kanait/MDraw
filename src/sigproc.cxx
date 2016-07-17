//
// sigproc.cxx
//
// Copyright (c) 1997-1999 Takashi Kanai; All rights reserved. 
//

#include "StdAfx.h"

#if defined(_DEBUG) && defined(_WINDOWS)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include <math.h>
#include "smd.h"
#include "linbcg.h"
#include "ppdvertex.h"
#include "ppdface.h"
#include "veclib.h"
#include "sigproc.h"
#include "dspr.h"

//
// フェアリング (signal processing) 制約なし
//
void ppd_signal_processing( Sppd *ppd, 
			    double lambda,
			    double mu,
			    int nrepeat )
{
  if ( ppd == NULL ) return;
  // ed->ewt
  double *ewt = edge_weight( ppd );

  // initialize
  int j = 0;
  Spvt *vt;
  for ( vt = ppd->spvt; vt != NULL; vt = vt->nxt ) {
    vt->sid = j; ++j;
  }
    

  //   create matrix I - lK... 
  SparseMatrix *sml = create_spmatrix_dspr( ppd, ewt, lambda );
  //   create matrix I - mK...
  SparseMatrix *smm = create_spmatrix_dspr( ppd, ewt, mu );
  
  double *vecx  = (double *) malloc( ppd->vn * sizeof(double) );
  double *vecy  = (double *) malloc( ppd->vn * sizeof(double) );
  double *vecz  = (double *) malloc( ppd->vn * sizeof(double) );
  double *dvecx = (double *) malloc( ppd->vn * sizeof(double) );
  double *dvecy = (double *) malloc( ppd->vn * sizeof(double) );
  double *dvecz = (double *) malloc( ppd->vn * sizeof(double) );

  for ( j = 0, vt = ppd->spvt; j < ppd->vn; ++j, vt = vt->nxt ) {
    vecx[j] = vt->vec.x; vecy[j] = vt->vec.y; vecz[j] = vt->vec.z;
  }
  
  for ( int i = 0; i < nrepeat; ++i ) {
    dsprsax( smm->sa, smm->ija, vecx, dvecx, smm->ncol );
    dsprsax( smm->sa, smm->ija, vecy, dvecy, smm->ncol );
    dsprsax( smm->sa, smm->ija, vecz, dvecz, smm->ncol );
    dsprsax( sml->sa, sml->ija, dvecx, vecx, sml->ncol );
    dsprsax( sml->sa, sml->ija, dvecy, vecy, sml->ncol );
    dsprsax( sml->sa, sml->ija, dvecz, vecz, sml->ncol );
  }
  
  for ( j = 0, vt = ppd->spvt; j < ppd->vn; ++j, vt = vt->nxt ) {
    vt->vec.x = vecx[j]; vt->vec.y = vecy[j]; vt->vec.z = vecz[j];
  }

  // normal の再計算
  for ( Spfc *fc = ppd->spfc; fc != NULL; fc = fc->nxt ) {
    calc_fnorm( fc );
  }
  
  free( ewt );
  free( vecx ); free( vecy ); free( vecz ); 
  free( dvecx ); free( dvecy ); free( dvecz );
  free_sparsematrix( sml );
  free_sparsematrix( smm );

}

SparseMatrix *create_spmatrix_dspr( Sppd *ppd,
				    double *ewt, // エッジの重み
				    double coefs // 係数
				    )
{
  SparseMatrix *sm = init_sparsematrix( ppd->vn );
  
  // decide number of array
  Spvt *vt;
  sm->narray = ppd->vn + 1; 
  for ( vt = ppd->spvt; vt != NULL; vt = vt->nxt ) {
    sm->narray += vt->ven;
  }

  sm->sa  = (double *) malloc( sm->narray * sizeof(double) );
  int i;
  for ( i = 0; i < sm->narray; ++i ) sm->sa[i] = 0.0;
  sm->ija = (int *) malloc( sm->narray * sizeof(int) );

  // store values
  sm->ija[0] = sm->ncol + 1;
  int j = sm->ncol;

  Spvt *avt;
  for ( i = 0, vt = ppd->spvt; vt != NULL; vt = vt->nxt, ++i ) {

    sm->sa[i] = 1.0 - coefs;
      
    double *vtwt = (double *) malloc( vt->ven * sizeof(double) );

    // sum weights
    double sum_wt = 0.0;
    int k = 0;
    Vted *vted;
    for ( vted = vt->svted; vted != NULL; vted = vted->nxt ) {
      avt = another_vt( vted->ed, vt );
      vtwt[k] = ewt[vted->ed->sid];
      sum_wt += vtwt[k];
      ++k;
    }


    // normalize weights and store into matrix
    k = 0;
    for ( vted = vt->svted; vted != NULL; vted = vted->nxt ) {
      avt = another_vt( vted->ed, vt );
      vtwt[k] /= sum_wt;
      ++j;
      sm->ija[j] = avt->sid;
      sm->sa[j]  = coefs * vtwt[k];
      display("vt %d ed %d ija %d sa %g\n", i, vted->ed->sid, sm->ija[j], sm->sa[j] );
      ++k;
    }

    free( vtwt );

    sm->ija[i+1] = j+1;

  }

  return sm;
}

double *edge_weight( Sppd *ppd )
{
  if ( ppd == NULL ) return NULL;
  
  double *ewt = (double *) malloc( ppd->en * sizeof( double ) );
  int i = 0;
  for ( Sped *ed = ppd->sped; ed != NULL; ed = ed->nxt ) {
    ed->sid = i;
    double len = V3DistanceBetween2Points( &(ed->sv->vec), &(ed->ev->vec) );
    if ( fabs(len) > SMDZEROEPS ) {
      ewt[ed->sid] = 1.0 / len;
    } else {
      ewt[ed->sid] = 0.0;
    }
    display("ed %d ewt %g\n", ed->sid, ewt[ed->sid]);
    ++i;
  }
  return ewt;
}

#if 0
//
// uvw フェアリング (signal processing) 制約あり
//
void uvw_signal_processing( Sppd *ppd, 
			    double lambda,
			    double mu,
			    int nrepeat )
{
  if ( ppd == NULL ) return;

  // initialize
  int j = 0;
  Spvt *vt;
  for ( vt = ppd->spvt; vt != NULL; vt = vt->nxt ) {
    vt->sid = j; ++j;
  }
    
  // ed->ewt
  double *ewt = uvw_edge_weight( ppd );

  //   create matrix I - lK... 
  SparseMatrix *sml = create_spmatrix_dspr_const( ppd, ewt, lambda );
  //   create matrix I - mK...
  SparseMatrix *smm = create_spmatrix_dspr_const( ppd, ewt, mu );
  
  double *vecx  = (double *) malloc( ppd->vn * sizeof(double) );
  double *vecy  = (double *) malloc( ppd->vn * sizeof(double) );
  double *dvecx = (double *) malloc( ppd->vn * sizeof(double) );
  double *dvecy = (double *) malloc( ppd->vn * sizeof(double) );

  for ( j = 0, vt = ppd->spvt; j < ppd->vn; ++j, vt = vt->nxt ) {
    vecx[j] = vt->uvw.x;
    vecy[j] = vt->uvw.y;
  }
  
  for ( int i = 0; i < nrepeat; ++i ) {
//      dsprsax( sml->sa, sml->ija, vecx, dvecx, sml->ncol );
//      dsprsax( sml->sa, sml->ija, vecy, dvecy, sml->ncol );
    dsprsax( smm->sa, smm->ija, vecx, dvecx, smm->ncol );
    dsprsax( smm->sa, smm->ija, vecy, dvecy, smm->ncol );
    dsprsax( sml->sa, sml->ija, dvecx, vecx, sml->ncol );
    dsprsax( sml->sa, sml->ija, dvecy, vecy, sml->ncol );
//      for ( j = 0; j < ppd->vn; ++j) {
//        vecx[j] = dvecx[j];
//        vecy[j] = dvecy[j];
//      }
      
  }
  
  for ( j = 0, vt = ppd->spvt; j < ppd->vn; ++j, vt = vt->nxt ) {
    vt->uvw.x = vecx[j];
    vt->uvw.y = vecy[j];
  }

  free( ewt );
  free( vecx ); free( vecy );
  free( dvecx ); free( dvecy );
  free_sparsematrix( sml );
  free_sparsematrix( smm );

}

//
// 制約付きマトリクスの生成
//
SparseMatrix *create_spmatrix_dspr_const( Sppd *ppd,
					  double *ewt, // エッジの重み
					  double coefs // 係数
					  )
{
  SparseMatrix *sm = init_sparsematrix( ppd->vn );
  
  // decide number of array
  Spvt *vt;
  sm->narray = ppd->vn + 1; 
  for ( vt = ppd->spvt; vt != NULL; vt = vt->nxt ) {
    sm->narray += vt->ven;
  }

  sm->sa  = (double *) malloc( sm->narray * sizeof(double) );
  int i;
  for ( i = 0; i < sm->narray; ++i ) sm->sa[i] = 0.0;
  sm->ija = (int *) malloc( sm->narray * sizeof(int) );
  
  // store values
  sm->ija[0] = sm->ncol + 1;
  int j = sm->ncol;

  Spvt *avt;
  for ( i = 0, vt = ppd->spvt; vt != NULL; vt = vt->nxt, ++i ) {

    if ( vt->isBoundary != TRUE )
      {
      
	sm->sa[i] = 1.0 - coefs;
      
	double *vtwt = (double *) malloc( vt->ven * sizeof(double) );

	// sum weights
	double sum_wt = 0.0;
	int k = 0;
	Vted *vted;
	for ( vted = vt->svted; vted != NULL; vted = vted->nxt ) {
	  avt = another_vt( vted->ed, vt );
	  if ( avt->isBoundary != TRUE ) {
	    //vtwt[k] = ewt[vted->ed->sid];
	    vtwt[k] = 1.0;
	    sum_wt += vtwt[k];
	    ++k;
	  }
	}

	// normalize weights and store into matrix
	k = 0;
	for ( vted = vt->svted; vted != NULL; vted = vted->nxt )
	  {
	    avt = another_vt( vted->ed, vt );
	    if ( avt->isBoundary != TRUE ) {
	      vtwt[k] /= sum_wt;
	      ++j;
	      sm->ija[j] = avt->sid;
	      sm->sa[j]  = coefs * vtwt[k];
	      ++k;
	    }
	  }
	
	free( vtwt );
      }
    else // constraint vertex
      {
	sm->sa[i] = 1.0;
      }
    
    sm->ija[i+1] = j+1;

  }

  return sm;
}

#endif
