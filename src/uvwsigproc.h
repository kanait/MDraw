//
// uvwsigproc.h
//
// Copyright (c) 2000 Takashi Kanai; All rights reserved. 
//

#ifndef _UVWSIGPROC_H
#define _UVWSIGPROC_H

extern void uvwsigproc( Sppd *, double, double, int, int );
extern void tfcparam( TFace*, int );
extern double tfcparam_membrane( TFace* );
extern double tfcparam_eweight( TFace* );
extern double tfcparam_fweight( TFace* );
extern double tfcparam_tpweight( TFace* );
extern double tfcparam_hmweight( TFace* );

#endif // _UVWSIGPROC_H
