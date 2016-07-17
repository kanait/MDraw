//
// hestack.h
//
// Copyright (c) 1999-2000 Takashi Kanai; All rights reserved. 
//

#ifndef _HESTACK_H
#define _HESTACK_H

//
// data structure for vertices on halfedges
// (これはかなり特殊なデータ構造群です)

// used in tface_subdivide_ppdface
// 分割する前の面を格納するためのデータ構造
typedef struct _subdivface {

  // 分割の対象となる面
  Spfc *fc;

  // 分割につかうエッジ(最大で 4 本)
  int en;
  Sped *ed[4]; // 最大で 4 本

  // 分割された面 (最大で 5 つ)
  //int fn;
  //Spfc *new_fc[5];

  // 分割する前の面
  Spfc* prev_fc;
  
} SubdivFace;

// used in tface_subdivide_ppdface
// 分割する前のエッジを格納するためのデータ構造
typedef struct _subdivedge {

  // 分割の対象となるエッジ
  Sped *ed;

  // 分割に使う頂点 (最大で 4 つ)
  int vn;
  Spvt *vt[4]; // 最大で 4 つ

  // 分割されたエッジ (最大で 5 本)
  //int en;
  //Sped *new_ed[5];

} SubdivEdge;

typedef struct _hestack Hestack;
typedef struct _hevt Hevt;
typedef struct _hevted Hevted;

struct _hestack {
  
  // 面のハーフエッジの数だけの Hevt (ポインタ配列)
  Sphe *he;
  Hevt *hevt;
  
  int hevtn;
};
  
struct _hevt {
  // ID
  int hsid;
  int hvid;

  // 頂点
  Spvt *vt;

  // 分割したエッジへのリンク（いらない）
  //Sped *ed;
  // ソートに必要な値
  double val;
  // 分割において使われたかどうか
  //  BOOL isUsed;

  // エッジへのリンク（双方向巡回リスト）
  int nhve;
  Hevted *svted, *evted;
};

// 頂点からエッジへのリンクとなる双方向巡回リストノード
// 半時計周り
struct _hevted {

  // 値 (一応)
  double val;
  
  // エッジ
  Sped *ed;

  // バックポインタ
  Hevt *hevt;
  
  // メイト
  Hevted *mate;

  // 分割において使われたかどうか
  BOOL isVisited;
  
  // 前後のノード
  Hevted *nxt, *prv;
};
  
// hestack.cxx
extern void subdivide_ppdface( SubdivFace *, Sppd *, int*, SubdivFace*, int*, SubdivEdge* );
extern void initialize_hestack( SubdivFace *, Hestack *, Sppd *, int*, SubdivEdge* );
extern int find_henum_hestack( Sped *, int, Hestack * );
extern Hevt *next_hevt_triloop( Hevt *, int, Hestack * );
extern void subdivide_ppdface_by_hestack( Spfc *, Hestack *, Sppd *, int*, SubdivFace* );
extern BOOL isvalid_create_ppdface( Hevted *, Hevt * );
extern void initialize_hevt( Hevt * );
extern void insert_hevt_in_hestack( Spvt *, Sped *, Hestack * );
extern Hevt *find_hevt_from_hestack( Spvt *, int, Hestack * );
extern Hevted *create_hevted( Sped *, Hevt * );
extern Hevted *find_hevted( Sped *, Hevt * );
extern Hevted *insert_hevted( double, Sped *, Hevt * );
extern void free_halfedgevertexedge( Hevt * );
extern void free_hevted( Hevted *, Hevt * );
extern void hevted_makemate( int, Hestack * );

extern void init_subdivface( SubdivFace* );
extern SubdivFace* find_subdivface( int, SubdivFace*, Spfc* );
extern void init_subdivedge( SubdivEdge* );
extern SubdivEdge* find_subdivedge( int, SubdivEdge*, Sped* );


#endif // _HESTACK_H
