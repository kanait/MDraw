//
// fwarp.cxx
// foldover-free mesh warping functions header
//
// Copyright (c) 2000 Takashi Kanai; All rights reserved. 
//

#ifndef _FFWARP_H
#define _FFWARP_H

//
// Point location data structure
//
// 頂点一つにつき一つ作られる
typedef struct _ploc PLoc;

struct _ploc {

  // 頂点
  Spvt *vt;

  // この頂点を含む包含面
  Spfc *fc;
  
};

//
// エッジを格納しておくための待ち行列 データ構造
//
typedef struct _spedfifo SpedFIFO;
typedef struct _spedfifonode SpedFIFONode;

struct _spedfifo {

  int en;
  SpedFIFONode *senode, *eenode;
  
};

struct _spedfifonode {
  
  Sped *ed;
  SpedFIFONode *nxt, *prv;
  
};

// functions
extern void ppdffwarp(Sppd *, Sppd *);
extern PLoc *ppdpointlocation(Sppd *, Sppd *);
extern Spfc *FindInFace(Sppd *, Vec2d *);
extern SpedPQHeap *initialize_spedpqheap(Sppd *);
extern void update_spedpqcont(Spvt *, SpedPQHeap *);
extern Sped *find_cross_1ring_edge(Spvt *, Vec2d *, Vec2d *);
extern double facearea2d(Spfc *);
extern Spfc *find_areazero_1ring_face(Spvt *, Vec2d *, Vec2d *, double *);
extern Spvt *find_180_vertex(Spfc *, Spvt *);
extern Spvt *find_180_vertex_param(Spfc *, Spvt *, Vec2d *, Vec2d *, double);
extern Sped *edge_swap(Sped *, Sppd *);
extern Spvt *get_lm_ppdvertex(Spfc *, Spvt *, Spvt *);
extern Sped *get_lm_ppdedge(Spfc *, Spvt *, Spvt *);
extern Sphe *get_lm_ppdhalfedge_rev(Spfc *, Sped *);
extern SpedFIFO *create_spedfifo(void);
extern void free_spedfifo(SpedFIFO *);
extern void spedfifo_deletefirstnode(SpedFIFO *);
extern SpedFIFONode *spedfifo_getstartnode(SpedFIFO *);
extern SpedFIFONode *create_spedfifonode(Sped *, SpedFIFO *);
extern void free_spedfifonode(SpedFIFONode *, SpedFIFO *);

#endif // _FFWARP_H
