//
// spedprique.h
//
// Copyright (c) 2000 Takashi Kanai; All rights reserved. 
//
//
// ppd edge priority queue (heap) structure
// (used in ffwarp.cxx)
//

#ifndef _SPEDPRIQUE_H
#define _SPEDPRIQUE_H

struct _spedpqcont {

  // SpedPQHeap の配列の値
  Id   id;
  
  // 評価値
  double length;

  // 移動する頂点
  Spvt *vt;

  // 交差するエッジ
  Sped *ed;

  // ed と交差する頂点
  // (必ずしも vt と同じである必要はない)
  Spvt *ivt;
  
  // 移動する終点のベクトル
  Vec2d evec;
  
};
  
struct _spedpqheap {
  
  SpedPQCont *pqcont;
  int    size;
  int    last;
  
};

// functions
extern SpedPQHeap *create_spedpqheap(int);
extern void swap_spedpqcont(SpedPQCont *, SpedPQCont *);
extern SpedPQCont *insert_spedpqcont(SpedPQHeap *, double, Spvt *);
extern void spedpqcont_to_root(int, SpedPQHeap *);
extern int  spedpqcont_adjust_to_parent(int, SpedPQHeap *);
extern int  spedpqcont_adjust_to_child(int, SpedPQHeap *);
extern int  adjust_spedpqcont(Id, SpedPQHeap *);
extern void deletemin_spedpqcont(SpedPQHeap *);
extern void delete_spedpqcont(Id, SpedPQHeap *);
extern void free_spedpqheap(SpedPQHeap *);
extern int parent_num(int);

#endif // _SPEDPRIQUE_H
