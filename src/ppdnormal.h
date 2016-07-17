//
// ppdnormal.h
//
// Copyright (c) 1997-1999 Takashi Kanai; All rights reserved. 
//

#ifndef _PPDNORMAL_H
#define _PPDNORMAL_H

extern Spnm *create_ppdnormal(Sppd *);
extern void free_ppdnormal(Spnm *, Sppd *);
extern void ppdnorm( Sppd *, double );
extern void freeppdnorm( Sppd * );

#endif // _PPDNORMAL_H
