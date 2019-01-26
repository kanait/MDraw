//
// mdw.h
//
// Copyright (c) 2000 Takashi Kanai
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
//

#ifndef _MDW_H
#define _MDW_H

extern void write_mdw_v10( char *, Sppd *, Tile * );
extern Tile *open_mdw( char *, BOOL );
extern Spfc *find_edgeonface( Sped * );
extern BOOL find_vertex_in_face( Spfc *, Spvt * );
extern Splp *create_closedloop_mdw( TFace *, Spvt * );

#endif // _MDW_H
