//
// spm.h
//
// Copyright (c) 1997-1999 Takashi Kanai; All rights reserved. 
//

#ifndef _FLOATER_H
#define _FLOATER_H

extern void ppdspm(Sppd *);
extern void ppdspm_initialize(Sppd *);
extern void ppdspm_finish(Sppd *);
extern Semat *create_spm_emat(Sppd *);
extern void create_spm_vector(Sppd *, Semat *);
extern void spm_lambda(Sppd *);
extern void ppdvertex_gpm(Spvt *);
extern void ppdvertex_spm_lambda(Spvt *);
extern void solvec_ppdvertex(Sppd *, Semat *);
extern void tfcspm(TFace *);
extern void tfcspm_initialize(TFace *);
extern Semat *create_tfcspm_emat(TFace *);
extern void tfcspm_lambda(TFace *);
extern void tvector_gpm(TVector *);
extern void tvector_spm_lambda(TVector *);
extern void create_tfcspm_vector(TFace *, Semat *);
extern void solvec_tvector(TFace *, Semat *);
extern void tfcspm_finish(TFace *);

#endif // _FLOATER_H
