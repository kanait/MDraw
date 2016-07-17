//
// esp.h
//
// Copyright (c) 1997-2000 Takashi Kanai; All rights reserved. 
//

#ifndef _ESP_H
#define _ESP_H

// for nearest coordinate ( in esp.cxx )
#define X_COORDP  0
#define Y_COORDP  1
#define Z_COORDP  2
#define X_COORDM  3
#define Y_COORDM  4
#define Z_COORDM  5

// functions
extern Splp *esp( Sppd *, Spvt *, Spvt * );
extern ESPTree *ppd_to_esptree( Sppd *, Spvt * );
extern BOOL unfold_imagesource( Sphe *, Vec *, Vec * );
extern BOOL ProjectHalfedgeParam3D( Vec *, Vec *, Vec *, Vec *,
				    double, double, Vec *, Vec *,
				    double *, double * );
extern int esp_occupation(Spvt *, ESPTree *, ESPEnode *, Vec *, Spvt *, Spvt *, int, Sphe *, double, double, Sphe *, double, double, ESPEList *);
extern int esp_update_occupation(Spvt *, ESPVnode *, ESPTree *, ESPEnode *, Vec *, int, Sphe *, double, double, Sphe *, double, double, ESPEList *);
extern Splp *esptree_to_esp( Sppd *, Spvt *, Spvt * );
extern void esptree_path( ESPTree *, Spvt *, Splp *, Sppd * );

#endif // _ESP_H
