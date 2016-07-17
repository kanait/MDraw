//
// ppd.h
//
// Copyright (c) 1997-1999 Takashi Kanai; All rights reserved. 
//

#ifndef _PPD_H
#define _PPD_H

// functions
extern Sppd *create_ppd(void);
extern void free_ppd(Sppd *);
extern void reset_ppd(Sppd *);
extern Sppd *copy_ppd(Sppd *);
extern void clear_newppd_links(Sppd *);
extern void calc_ppd_boundary(Sppd *);
extern void normalize_ppd(Sppd *);
extern void unnormalize_ppd(Sppd *);
extern void ppd_make_edges( Sppd* );

#endif // _PPD_H
