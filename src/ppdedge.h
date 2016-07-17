//
// ppdedge.h
//
// Copyright (c) 1997-1999 Takashi Kanai; All rights reserved. 
//

#ifndef _PPDEDGE_H
#define _PPDEDGE_H

// functions
extern Sped *create_ppdedge(Sppd *);
extern void free_ppdedge(Sped *, Sppd *);
extern void free_ppdedge_withqtree(Sped *, Sppd *);
extern void free_isolated_ppdedge(Sped *, Sppd *);
extern int num_ppdedge(Sppd *);
extern void edge_length(Sppd *);
extern Sped *find_ppdedge(Spvt *, Spvt *);
extern Sped *find_ppdedge_new(Spvt *, Spvt *);
extern double normalize_ppdedge(Sppd *);
extern void ppdedge_to_vec(Sped *, Vec *);
extern Spfc *ppdedge_another_fc(Sped *, Spfc *);

#endif // _PPDEDGE_H
