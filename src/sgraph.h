//
// sgraph.h
//
// Copyright (c) 1997-1999 Takashi Kanai; All rights reserved. 
//

#ifndef _SGRAPH_H
#define _SGRAPH_H

extern SGraph *create_sgraph(void);
extern void free_sgraph(SGraph *);
extern SGvt *create_sgvt(SGraph *);
extern SGvt *CreateSGvtfromSGvt(SGraph *, SGvt *);
extern void free_sgvt(SGvt *, SGraph *);
extern SGvt *another_sgvt(SGed *, SGvt *);
extern SGvt *find_sgvt_from_spvt(SGraph *, Spvt *);
extern SGvt *find_sgvt_from_spvt_number(SGraph *, int);
extern SGvt *find_sgvt_from_sgfc(SGfc *, SGvt *);
extern void FreeSGVertex(SGraph *);
extern SGvted *create_sgvted(SGvt *, SGed *);
extern void free_sgvted(SGvted *, SGvt *);
extern void FreeSGvted(SGvt *, SGed *);
extern void FreeSGVertexEdge(SGvt *);
extern SGed *create_sged(SGraph *);
extern SGed *CreateSGed(SGraph *, SGvt *, SGvt *, Sped *);
extern SGed *CreateSGedfromSGed(SGraph *, SGvt *, SGvt *, SGed *);
extern void free_sged(SGed *, SGraph *);
extern void FreeSGed(SGed *, SGraph *);
extern void FreeSGEdge(SGraph *);
extern SGed *find_sged(SGvt *, SGvt *);
extern SGfc *create_sgfc(SGraph *);
extern void free_sgfc(SGfc *, SGraph *);
extern void FreeSGFace(SGraph *);
extern SGfcvt *create_sgfcvt(SGfc *, SGvt *);
extern void free_sgfcvt(SGfcvt *, SGfc *);
extern void FreeSGFaceVertex(SGfc *);
extern SGlp *create_sglp(void);
extern void free_sglp(SGlp *);
extern SGlpvt *create_sglpvt(SGlp *, SGvt *);
extern void free_sglpvt(SGlpvt *, SGlp *);
extern void FreeSGLoopVertex(SGlp *);
extern SGlpvt *find_sglv( SGlp *, SGvt * );
extern SGlped *create_sglped(SGlp *, SGed *);
extern void free_sglped(SGlped *, SGlp *);
extern void FreeSGLoopEdge(SGlp *);
extern SGraph *initialize_sgraph( Sppd *, double );
extern void SGvtnrm( SGvt *, Vec * );

// usd for add_vertices_edges_sgraph
#define PRIPROCESS	0
#define ALGORITHM	1

// 分割数を決めるためのパラメータ
//#define GAMMA 0.1
//#define GAMMA 0.04
//#define GAMMA 0.01

// bunny_f1000.ppd で Avrsp がおおよそ6.0 になるときの値
#define GAMMA 0.018

#endif // _SGRAPH_H
