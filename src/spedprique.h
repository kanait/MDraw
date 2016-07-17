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

  // SpedPQHeap $B$NG[Ns$NCM(B
  Id   id;
  
  // $BI>2ACM(B
  double length;

  // $B0\F0$9$kD:E@(B
  Spvt *vt;

  // $B8r:9$9$k%(%C%8(B
  Sped *ed;

  // ed $B$H8r:9$9$kD:E@(B
  // ($BI,$:$7$b(B vt $B$HF1$8$G$"$kI,MW$O$J$$(B)
  Spvt *ivt;
  
  // $B0\F0$9$k=*E@$N%Y%/%H%k(B
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
