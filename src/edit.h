//
// edit.h
//
// Copyright (c) 1997-1999 Takashi Kanai
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
//

#ifndef _EDIT_H
#define _EDIT_H

extern BOOL esp_create_spath( ScreenAtr* );
extern BOOL asp_create_spath( ScreenAtr* );

extern BOOL create_tile_face( ScreenAtr* );
extern void tedge_lp_to_screen( Splp*, ScreenAtr* );
extern BOOL loop_connect_check( ScreenAtr* );
extern Splp *create_closedloop( ScreenAtr* );

extern BOOL create_tile_cylindrical_face( ScreenAtr* );
extern BOOL two_loop_connect_check( ScreenAtr*, int* );
extern void create_two_closedloop( ScreenAtr*, TFace* );

#endif // _EDIT_H
