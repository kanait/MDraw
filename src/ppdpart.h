//
// ppdpart.h
//
// Copyright (c) 1997-2000 Takashi Kanai; All rights reserved. 
//

#ifndef _PPDPART_H
#define _PPDPART_H

// functions
extern Sppt *create_ppdpart( Sppd * );
extern void free_ppdpart( Sppt *, Sppd * );
extern Sppt *list_ppdpart( Sppd *, int );

#endif // _PPDPART_H
