//
// smdsrc.h
//
// Copyright (c) 1997-1999 Takashi Kanai; All rights reserved. 
//

#ifndef _SMDSRC_H
#define _SMDSRC_H

extern Swin *create_swin(void);
extern void display3d_initialize(Disp3D *);
extern void screenatr_initialize(ScreenAtr *, int, int);
extern Material *material_initialize(void);
extern void swin_free_ppd(Swin *);
extern void free_swin(Swin *);

#endif // _SMDSRC_H
