//
// cylspm.h
//
// Copyright (c) 2000 Takashi Kanai; All rights reserved. 
//

#ifndef _CYLSPM_H
#define _CYLSPM_H

extern void cylspm(Sppd *);
extern void cylspm_initialize(Sppd *);
extern void cylspm_finish(Sppd *);
extern Semat *cylspm_create_emat(Sppd *);
extern void cylspm_create_vector(Sppd *, Semat *);
extern void cylspm_solvec_ppdvertex(Sppd *, Semat *);

#endif // _CYLSPM_H