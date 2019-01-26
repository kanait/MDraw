//
// params.h
//
// Copyright (c) 1997-1999 Takashi Kanai
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
//

#ifndef _PARAMS_H
#define _PARAMS_H

// structures for options
typedef struct _params {

  // input, output filename
  int  nfile;
  char filename[BUFSIZ];

  // TRUE: esp, FALSE: asp 
  BOOL esp; 

  // source, dist vertex no.
  int src_id, dist_id;

  // save a path
  BOOL save_path;
  char pathfilename[BUFSIZ];
  
  // print
  BOOL print;

  // ASP parameters
  double gamma;
  int midp;
  
} Params;

extern void params_init( Params * );

extern Params params;

#endif // _PARAMS_H
