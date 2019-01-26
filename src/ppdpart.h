//
// ppdpart.h
//
// Copyright (c) 1997-1999 Takashi Kanai
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
//

#ifndef _PPDPART_H
#define _PPDPART_H

// functions
extern Sppt *create_ppdpart( Sppd * );
extern void free_ppdpart( Sppt *, Sppd * );
extern Sppt *list_ppdpart( Sppd *, int );

#endif // _PPDPART_H
