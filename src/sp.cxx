//
// sp.cxx
// main routine for calculating a shortest path.:
//
// Copyright (c) 1998-1999 Takashi Kanai
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
//

#ifdef _AFXDLL
#include "StdAfx.h"
#endif

#if defined(_DEBUG) && defined(_WINDOWS)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "smd.h"
#include "screen.h"
#include "ppd.h"
#include "ppdlist.h"
#include "ppdvertex.h"
#include "ppdloop.h"
#include "esptree.h"
#include "asp.h"
#include "sgraph.h"
#include "esp.h"
#include "mytime.h"

#include "params.h"

Params params;

Splp *asp_run( Sppd *ppd, int IDsrc, int IDdist, double gam, int midp )
{
  if ( ppd == NULL ) return NULL;

  Spvt *src = list_ppdvertex( ppd, IDsrc );
  if ( src == NULL ) {
    fprintf( stderr, "asp_run can't find vertices id %d\n", IDsrc );
    return NULL;
  }
  
  Spvt *dist = list_ppdvertex( ppd, IDdist );
  if ( dist == NULL ) {
    fprintf( stderr, "asp_run can't find vertices id %d\n", IDdist );
    return NULL;
  }

  // sgraph initialize
  double gamma = gam;
  SGraph *sg = ppd->sg = initialize_sgraph( ppd, gamma );
  if ( sg == NULL ) return FALSE;
  
  sg->src  = find_sgvt_from_spvt( sg, src );
  sg->dist = find_sgvt_from_spvt( sg, dist );
  if ( (sg->src == NULL) || (sg->dist == NULL) ) return FALSE;

  // create a exact path
  Splp *lp = NULL;
  lp = asp( sg, ppd, midp );

  //display("sg avr. %g\n", sg->avrsp);
  
  free_sgraph( sg );
  
  return lp;
}

Splp *asp_all_run( Sppd *ppd, int IDsrc, int IDdist, double gam, int midp )
{
  if ( ppd == NULL ) return NULL;

  Spvt *src = list_ppdvertex( ppd, IDsrc );
  if ( src == NULL ) {
    fprintf( stderr, "asp_run can't find vertices id %d\n", IDsrc );
    return NULL;
  }

  // start vertex
  Spvt *dist = list_ppdvertex( ppd, IDdist );
  if ( dist == NULL ) {
    fprintf( stderr, "asp_run can't find vertices id %d\n", IDdist );
    return NULL;
  }

  Spvt *vt = NULL;
//   for ( vt = dist; vt != NULL; vt = vt->nxt ) {
//     if( src == vt ) continue;
//     display("./sp.exe bunny.ppd -s %d -d %d -g 0.01 -mp 1 >> alogf69451g001mp1\n", src->no, vt->no );
//   }

  // sgraph initialize
  double gamma = gam;
  SGraph *sg = ppd->sg = initialize_sgraph( ppd, gamma );
  if ( sg == NULL ) return FALSE;
  display("gamma %g SP avr. %g\n", gam, sg->avrsp);

  Splp *lp = NULL;
  int i = 0;
  for ( vt = dist; vt != NULL; vt = vt->nxt ) {
    
    ++i;
    if ( src == vt ) continue;
    sg->src  = find_sgvt_from_spvt( sg, src );
    sg->dist = find_sgvt_from_spvt( sg, vt );
    if ( (sg->src == NULL) || (sg->dist == NULL) ) continue;

    // create a exact path
    Splp *lp = NULL;
    lp = asp( sg, ppd, midp );
    if ( lp != NULL ) {
      display("src: %d dist: %d distance: %g time: %g\n",
	      src->no, vt->no, lp->length, lp->usertime );
      free_ppdvertex_splp( lp, ppd );
      free_splp( lp );
      lp = NULL;
    } else {
      display("src: %d dist: %d distance: error. time: error.\n",
	      src->no, vt->no );
    }
    if ( i == 100 ) break;
  }
  
  free_sgraph( sg );
  
  return lp;
}

Splp *esp_run( Sppd *ppd, int IDsrc, int IDdist )
{
  if ( ppd == NULL ) return NULL;

  Spvt *src = list_ppdvertex( ppd, IDsrc );
  if ( src == NULL ) {
    fprintf( stderr, "esp_run can't find vertices id %d\n", IDsrc );
    return NULL;
  }
  
  Spvt *dist = list_ppdvertex( ppd, IDdist );
  if ( dist == NULL ) {
    fprintf( stderr, "esp_run can't find vertices id %d\n", IDdist );
    return NULL;
  }

  //printf("src %d dist %d\n", src->no, dist->no );
  
  // create a exact path
  Splp *lp = NULL;
  lp = esp( ppd, src, dist );

  return lp;
}

//#ifdef WIN32
//void main( int argc, char *argv[] )
  //#else
int main( int argc, char *argv[] )
  //#endif  
{
  params_init( &params );
  
  int i = 1;
  int j = 0;
  while( i < argc ) {
    
    if ( !strcmp(argv[i], "-savepath") ) {

      params.save_path = TRUE;
      ++i;
      if ( argv[i] ) strcpy( params.pathfilename, argv[i] );
      
    } else if ( !strcmp(argv[i], "-s") ) {
      
      ++i;
      if ( argv[i] ) params.src_id = atoi(argv[i]);
      
    } else if ( !strcmp(argv[i], "-d") ) {
      
      ++i;
      if ( argv[i] ) params.dist_id = atoi(argv[i]);
      
    } else if ( !strcmp(argv[i], "-esp") ) {
      
      params.esp = TRUE;
      
    } else if ( !strcmp(argv[i], "-g") ) {
      
      ++i;
      if ( argv[i] ) params.gamma = atof(argv[i]);
      
    } else if ( !strcmp(argv[i], "-mp") ) {

      ++i;
      if ( argv[i] ) params.midp = atoi(argv[i]);
      
    } else if ( !strcmp(argv[i], "-print") ) {

      params.print = TRUE;
      
    } else {
      
      if ( j < 1 ) {
	strcpy( params.filename, argv[i] );
      }
      ++j;
      ++(params.nfile);

    }
    ++i;     
  }

  if ( (params.nfile != 1) ||
       (params.src_id == SMDNULL) ||
       (params.dist_id == SMDNULL) ) {
    
    fprintf( stderr, "SP (Shortest-Path generator) Ver.0.8.01 \n");
    fprintf( stderr, "Copyright (c) 1999 Takashi Kanai. All rights reserved.\n");
    fprintf( stderr, "Usage: sp(.exe) in.ppd -s num -d num [-esp] [-savepath pathfile] [-g val] [-mp val] [-print]\n");
    fprintf( stderr, "Options: \n");
    fprintf( stderr, "  -s:\tsource vertex no.\n");
    fprintf( stderr, "  -d:\tdistination vertex no.\n");
    fprintf( stderr, "  -esp:\texact shortest paths.\n");
    fprintf( stderr, "  -savepath:\tsave path file (.sp)\n");
    fprintf( stderr, "  -g:\t gamma (ASP only). \n");
    fprintf( stderr, "  -mp:\tnumber of mid-point (ASP only)\n");
    fprintf( stderr, "  -print:\tprint status.\n");
    exit( 1 );
  }

  Sppd *ppd = open_ppd( params.filename, FALSE );
  if ( ppd == NULL ) {
    fprintf( stderr, "can't find %s.\n", params.filename );
    exit( 1 );
  }
  
  // calculate exact shortest path
  Splp *lp = NULL;

  if ( params.esp == TRUE ) {

    // exact shortest path
    display("esp...\n");
    lp = esp_run( ppd, params.src_id, params.dist_id );

  } else {

    // approximate shortest path
//     display("asp...\n");
    lp = asp_run( ppd, params.src_id, params.dist_id,
		  params.gamma, params.midp );
    
//     lp = asp_all_run( ppd, params.src_id, params.dist_id,
// 		      params.gamma, params.midp );
    
  }
  
  if ( lp == NULL ) {
    fprintf(stdout, "src: %d dist: %d distance: error. time: error. iter: error.\n",
	    params.src_id, params.dist_id );
    free_ppd( ppd );
    exit(1);
  }

  if ( params.save_path == TRUE )
    write_sp_file( lp, params.pathfilename, params.filename );

  fprintf(stdout, "src: %d dist: %d distance: %g time: %g iter: %d\n",
	  params.src_id, params.dist_id, lp->length, lp->usertime, lp->num_iter );
  
  free_splp( lp );
  free_ppd( ppd );
  
  exit( 0 );
}
