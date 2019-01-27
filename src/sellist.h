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
#if 0
extern SelList *create_selectlist_tedge( ScreenAtr *, TEdge * );
extern SelList *create_selectlist_tface( ScreenAtr *, TFace * );
#endif
extern SelList *list_selectlist( ScreenAtr *, Id );
#if 0
extern SelList *list_selectlist_tfc( ScreenAtr*, TFace* );
#endif
extern void free_sellist( SelList*, ScreenAtr* );
extern void FreeSelectList(ScreenAtr *);

#endif // _SELLIST_H
