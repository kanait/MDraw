//
// ppdlist.h
//
// Copyright (c) 1997-1999 Takashi Kanai; All rights reserved. 
//

#ifndef _PPDLIST_H
#define _PPDLIST_H

extern void write_ppd_file( char *, Sppd *, BOOL );
extern Sppd *open_ppd( char *, BOOL );
extern int gppd_ok(char *);
extern int ppd_ok(char *, int);
extern void printppd(Sppd *);

#endif // _PPDLIST_H
