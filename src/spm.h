//
// spm.h
//
// Copyright (c) 1999-2000 Takashi Kanai
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
//

#ifndef _SPM_H
#define _SPM_H

extern void ppdparam(Sppd *, int);
extern void ppdparam_initialize(Sppd *);
extern void boundary_mapping(Sppd *);
extern void internal_mapping(Sppd *, int);
extern double param_membrane(Sppd *);
extern double param_eweight(Sppd *);
extern double param_fweight(Sppd *);
extern double param_tpweight(Sppd *);
extern double param_hmweight(Sppd *);
extern double param_hmw(Sped *);

extern void write_ppd_uvw(char *, Sppd *);
extern void write_ppd_tfc(char *, TFace *, Sppd *);
extern void write_tfc_param_file(char *, TFace *, Sppd *);
extern void write_tfc_param_ps_file(char *, TFace *, Sppd *);

// defines
#define SP_NONE     0
#define SP_CONVEX   1
#define SP_MEMBRANE 2
#define SP_EWEIGHT  3
#define SP_FWEIGHT  4
#define SP_TPWEIGHT 5
#define SP_HMWEIGHT 6

#endif // _SPM_H
