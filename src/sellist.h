//
// sellist.h
//
// Copyright (c) 1997-1999 Takashi Kanai
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
//

#ifndef _SELLIST_H
#define _SELLIST_H

extern SelList *create_selectlist_vt( ScreenAtr*, Spvt* );
extern SelList *create_selectlist_ppdface( ScreenAtr*, Spfc * );
extern SelList *create_selectlist_lp( ScreenAtr *, Splp * );
extern SelList *create_selectlist_tedge( ScreenAtr *, TEdge * );
extern SelList *create_selectlist_tface( ScreenAtr *, TFace * );
extern SelList *list_selectlist( ScreenAtr *, Id );
extern SelList *list_selectlist_tfc( ScreenAtr*, TFace* );
extern void free_sellist( SelList*, ScreenAtr* );
extern void FreeSelectList(ScreenAtr *);

#endif // _SELLIST_H
