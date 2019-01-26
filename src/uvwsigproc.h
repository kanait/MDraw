//
// uvwsigproc.h
//
// Copyright (c) 1998 Takashi Kanai
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
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
