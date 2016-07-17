//
// oppd.h
//  
//  Copyright (c) 1997-1999 by Takashi Kanai; All rights researved.
//

#ifndef _OPPD_H
#define _OPPD_H

//
// for open ppd structure
//
typedef struct _oppd Oppd;
typedef struct _oso  Oso;
typedef struct _opt  Opt;
typedef struct _ovt  Ovt;
typedef struct _onm  Onm;
typedef struct _ofc  Ofc;

struct _oppd {
  Oso *oso;
  Opt *opt;
  Onm *onm;
  Ovt *ovt;
  Ofc *ofc;
};

struct _oso {
  Spso *so;
};

struct _opt {
  Sppt *pt;
};

struct _onm {
  Spnm *nm;
  Spso *so;
};

struct _ovt {
  Spvt *vt;
  Spso *so;
};

struct _ofc {
  Spfc *fc;
  Spso *so;
  Sppt *pt;
};

extern Oppd *create_oppd(void);
extern void free_oppd(Oppd *);
extern Oso *create_osolid( int );
extern Opt *create_opart( int );
extern Ofc *create_oface( int );
extern Onm *create_onormal( int );
extern Ovt *create_overtex( int );

#endif // _OPPD_H
