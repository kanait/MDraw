//
// ppdloop.h
//
// Copyright (c) 1997-1999 Takashi Kanai
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
//

#ifndef _PPDLOOP_H
#define _PPDLOOP_H

// functions
extern Splp *create_splp(void);
extern Splp *create_ppdloop(Sppd *);
extern Splp *find_ppdloop(Sppd *, Spvt *);
extern void free_splp(Splp *);
extern void free_ppdloop(Splp *, Sppd *);
extern Splv *create_ppdloopvertex(Splp *);
extern Splv *find_ppdloopvertex(Splp *, Spvt *);
extern void free_ppdvertex_splp( Splp *, Sppd * );
extern void free_ppdloopvertex(Splv *, Splp *);
extern Sple *create_ppdloopedge(Splp *);
extern void free_ppdloopedge(Sple *, Splp *);
extern Splp *reverse_ppdloop( Splp * );
extern void add_splp( Splp *, Splp * );
extern void write_sp_file( Splp *, char *, char * );
extern Splp *open_sp( char *, Sppd * );

#endif // _PPDLOOP_H
