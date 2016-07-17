//
// smd.h
//
// Copyright (c) 1997-1999 Takashi Kanai; All rights reserved. 
//

#ifndef _SMD_H
#define _SMD_H

typedef	int Id;

// for BOOL
#ifndef _WINDOWS
typedef unsigned short BOOL;
#define TRUE  1
#define FALSE 0
#endif

// for BUFSIZ
// #ifndef WIN32
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>
//#endif

//
// fundermental items list
//

typedef struct _vec {
  double	x;
  double	y;
  double	z;
} Vec;

typedef struct _vec2d {
  double	x;
  double	y;
} Vec2d;

// 3-by-3 matrix
typedef struct _mat3 {		
  double   elm[3][3];
} Mat3;

// 4-by-4 matrix
typedef struct _mat4 {		
  double   elm[4][4];
} Mat4;

typedef double **Vmtx;

// "Material" structure

typedef struct _material {
  float ambient[4];
  float diffuse[4];
  float emission[4];
  float specular[4];
  float shininess[1];
} Material;

// "Light" structure

typedef struct _light {
  float   position[4];
  float   ambient[4];
  float   diffuse[4];
  float   specular[4];
  float const_attenuation;
  float linear_attenuation;
} Light;

//
// PPD file format
//  node type parameters for memory allocation routines
//

#define PPD_PPD         0
#define PPD_SOLID	1
#define PPD_PART	2
#define PPD_FACE	3
#define PPD_HEDGE       4
#define PPD_NORMAL	5
#define PPD_VERTEX	6
#define PPD_EDGE	7
#define PPD_LOOP	8
#define PPD_UVW	        9
#define PPD_TVERTEX	10
#define PPD_TEDGE	11
#define PPD_TFACE	12
#define PPD_FNODE	13
#define PPD_NONE	14
#define PPD_HEADER	22

// loop open or not
#define OPENLOOP        0
#define CLOSEDLOOP      1
#define SHORTESTPATH    2
#define HLOOP           3

#define MAXVNUM         128
#define TRIANGLE        3
#define RECTANGLE       4

// type
#define ORIGINAL        0
#define ADDED           1

// for morphing
#define SRC             1
#define TARGET          2

// create from flag
#define FROM_INTSEC     0
#define FROM_SRC        SRC
#define FROM_TARGET     TARGET
#define FROM_HVERTEX    3

// for ppd structure
typedef struct _sppd Sppd;
typedef struct _spso Spso;
typedef struct _sppt Sppt;
typedef struct _sphe Sphe;
typedef struct _spfc Spfc;
typedef struct _spnm Spnm;
typedef struct _vtfc Vtfc;
typedef struct _vted Vted;
typedef struct _vtnm Vtnm;
typedef struct _vtvt Vtvt;
typedef struct _spvt Spvt;
typedef struct _sped Sped;
typedef struct _splv Splv;
typedef struct _sple Sple;
typedef struct _splp Splp;

// shortest-path graph structure
typedef struct _sgraph SGraph;
typedef struct _sgvted SGvted;
typedef struct _sgvt   SGvt;
typedef struct _sged   SGed;
typedef struct _sgfc   SGfc;
typedef struct _sgfcvt SGfcvt;
typedef struct _sglp   SGlp;
typedef struct _sglpvt SGlpvt;
typedef struct _sglped SGlped;

// for ESPTree
typedef struct _esptree  ESPTree;
typedef struct _espenode ESPEnode;
typedef struct _espvnode ESPVnode;
typedef struct _espelist ESPEList;
typedef struct _espelnode ESPELnode;

// for quadtree
typedef struct _qtree   QTree;
typedef struct _qrange  QRange;
typedef struct _qface   QFace;
typedef struct _qedge   QEdge;
typedef struct _qvertex QVertex;

// for priority queue (used in spedprique.cxx )
typedef struct _spedpqcont SpedPQCont;
typedef struct _spedpqheap SpedPQHeap;

//
// ppd structure
//
struct _sppd {
  
  int sn; int sid; Spso *spso, *epso; // Solid
  int pn; int pid; Sppt *sppt, *eppt; // Part
  int fn; int fid; Spfc *spfc, *epfc; // Face
  int nn; int nid; Spnm *spnm, *epnm; // Normal
  int vn; int vid; Spvt *spvt, *epvt; // Vertex
  int en; int eid; Sped *sped, *eped; // Edge
  int ln; int lid; Splp *splp, *eplp; // Loop

  // original vertex num;
  int nvt_org;
  
  // Quadtree for searching
  int qn; QTree *qt;
  int maxqtlevel;
  double xmin, xmax, ymin, ymax;

  // Material
  Material matl;

  // Misc (transformation information 
  double scale;
  Vec    center;

  // for smooth shading
  double smooth_angle;
  
  // for shortest_path
  double avr_edge_length;
  double max_edge_length;

  // SGraph data
  SGraph *sg;

  // ESPTree
  ESPTree *esptree;

  // texture coordinate
  BOOL texcoord;

  char   filename[BUFSIZ];
};

// ppd solid structure
struct _spso {
  Id no;
  Spso *nxt;
  Spso *prv;
};

// ppd part structure
struct _sppt {
  Id   no;
  Id   sid;
  Spso *bpso;
  Sppt *nxt;
  Sppt *prv;
};

// ppd face structure
struct _spfc {
  
  Id    col;
  Id    no;
  Id    sid;
  Vec   nrm;                   // normal vector
  int   hen;                   // number of halfedge
  Sphe  *sphe;
  Spso  *bpso;                 // back pointer to ppdsolid
  Sppt  *bppt;                 // back pointer to ppdpart

  // for tile grouping
  int tile_id;

  // for parameterization
  double area;
  
  Spfc  *nxt, *prv;
};

// ppd halfedge structure
struct _sphe {
  
  Id    no;
  Spvt  *vt;
  Spnm  *nm;
  Sped  *ed;
  Sphe  *mate;
  Spfc  *bpfc;
  Sphe  *nxt, *prv;
  
};

// ppd normal structure
struct _spnm {
  
  Id   sid;                   // vertex Identifier for save
  Id   no;                    // vertex exist or not flag
  Id   vn;
  Vec  vec;
  Spso *bpso;
  Spnm *nxt;
  Spnm *prv;
  
};

// vertex -> edge
struct _vted {
  
  Sped  *ed;
  Spvt  *vt;
  double angle;
  Vted *nxt;
  Vted *prv;
  
};

// vertex -> normal
struct _vtnm {
  int   cnt;
  Spnm  *nm;
  Spvt  *vt;
  Vtnm *nxt;
  Vtnm *prv;
};

#define EMPTY   0
#define EXIST   1
#define DELETED 2

// constants for creating hppdlod
// vertex attributes
#define VTXINTERNAL     0
#define VTXEXTERNAL     1

// edge attributes
#define EDGEBOUNDARY    1
#define EDGEINTERNAL    2

//
// constants for shortest-path (new)
// used in sgraph structure
//

// もとのメッシュの頂点か，加えられたものかを判断
// sgvt->type
#define VERTEX_ORIGINAL 0
#define VERTEX_ADDED    1

// 最短経路上の頂点か，そうでないかを判断
// sgvt->path
#define VERTEX_NONE     0
#define VERTEX_SPATH    1

// edge
#define EDGE_ORIGINAL   0
#define EDGE_ONFACE     1
//#define EDGE_SPATH      2

// constants for shortest-path, grouping
#define SP_VERTEX_NORMAL   0   // 普通の頂点
#define SP_VERTEX_STEINER  1   // スタイナ点 （Spvt では使っていない）
#define SP_VERTEX_TEDGE    2   // TEdge に使われる頂点
#define SP_VERTEX_TEDGE_BP 3   // TEdge に使われる頂点＆経路の始点・終点
#define SP_VERTEX_TVERTEX  4   // TVertex に使われる頂点

#define SP_EDGE_NORMAL   0
#define SP_EDGE_STEINER  1
#define SP_EDGE_BOUNDARY 2

//
// ppd vertex structure
//
struct _spvt {
  
  // original
  Id    sid;                      // vertex Identifier for save 
  Id    no;                       // vertex exist or not flag
  Id    col;                      // vertex color
  Id    type;                     // vertex type
  Sphe  *sphe;                    // start neighboring half-edge
  Spso  *bpso;                    // back pointer to ppdsolid
  Spvt  *nxt, *prv;
  Vec   vec;                      // position
  Vec2d uvw;                      // texture coordinates

  // 将来的には削除
  int ven; Vted *svted, *evted;   // vertex -> edge 
  int vnn; Vtnm *svtnm, *evtnm;   // vertex -> normal 

  // 次数 (近傍の頂点の数)
  int n_degree;

  // for shape preserving mapping (obsolete in the near future)
  double *lambda;
  Vec2d *vec2d;
  // for tile_grouping
  int tile_id; // id for tile (used for temporary only tface_grouping)
  int tvec_id; // id for tvector (used for temporary only tface_spm)
  
  // boundary information
  BOOL   isBoundary;
  
  // shortest path information
  int    sp_type;
  Sped   *sp_ed;
  double sp_val;
  
  // number of passing loops
  // tedge の消去のときに必要になる
  int    spn;

  // for priority queue (used in spedprique.cxx)
  SpedPQCont *pqc;
  int    pq_type;
  double length;

  // for ESPTree
  ESPTree *tree;
  
  // for ESPVnode
  ESPVnode *vnode;

};

//
// ppd edge structure
//
struct _sped {
  
  // original
  Id    no;                      // edge exist or not flag
  Id    col;                     // edge color
  Id    sid;                     // id for stock
  Spvt  *sv, *ev;                // start end Vertex
  Sphe  *lhe, *rhe;              // left and right halfedges
  Sped  *nxt, *prv;

  // edge type (EDGE_ORIGINAL or EDGE_ADDED)
  Id    type;

  // loop check
  //int    used_loop;
  
  // boundary information
  BOOL   isBoundary;

  // edge length
  double length;

  //
  // shortest path information
  //
  //int    num_steiner_point;
  int    sp_type;
  Spfc   *sp_fc; // used for shortest path (ONFACE only)
  
  //Sped   *org_ed;
  //int    cal_subgraph;
  //Spfc   *fc; // used for shortest path (ONFACE only)
  //  SGed   *sged;
  // for grouping
  //int    group_id;

};

//
// ppd loop structure
//

#define LOOPVT_CORNER   0
#define LOOPVT_ONBOUND  1

struct _splv {
  
  Id   no;
  Id   type;
  Spvt *vt;
  Splv *nxt, *prv;
  
};

struct _sple {
  Id   no;
  Sped *ed;
  Sple *nxt, *prv;
};

struct _splp {
  
  Id   no;
  // loop type ( open or closed )
  Id   type;
  Id   col;

  // number of corner vertices
  Id   lcvn;

  // links to loop vertices
  Id   lvn;
  Splv *splv, *eplv;

  // links to loop edges
  Id   len;
  Sple *sple, *eple;

  // length
  double length;

  // calculated time
  double realtime;
  double usertime;
  double systime;

  // the number of iterations
  int num_iter;
  
  Splp *nxt, *prv;
  
};

// SGraph priority queue structure
typedef struct _sgpqcont {
  double distance;
  Id   id;
  SGvt *tgt_vt;
  SGvt *prv_vt;
} SGPQCont;
  
typedef struct _sgpqheap {
  SGPQCont *pqcont;
  int    size;
  int    last;
} SGPQHeap;

typedef struct _sgvtlist {
  SGvt *prv;
  double dis;
} SGVtList;

// shortest-path graph structure
struct _sgraph {

  int  sgvtn, sgvtid;
  SGvt *sgvt, *egvt;
  
  int  sgedn, sgedid;
  SGed *sged, *eged;

  int  sgfcn, sgfcid;
  SGfc *sgfc, *egfc;

  SGvt *src, *dist;

  SGlp *lp;

  Sppd *ppd;

  // number of SP per edge
  double avrsp;

  double sub;
};

// SGvt to SGed link
struct _sgvted {
  
  SGed *ed;

  SGvted *nxt, *prv;
  
};
  
struct _sgvt {

  int no;
  int sid;

  //
  // vertex type
  unsigned short type;

  //
  // path vertex or not
  unsigned short path;
  
  //
  // an original vertex
  //  (type = VERTEX_SPATH_ORIGINAL)
  Spvt *spvt;

  //
  // an original edge
  // (type = VERTEX_SPATH_ADDED)
  Sped *sped;
  
  // 3D coordinates
  Vec  vec;
  
  // links to the neighbor edge
  int  sgvtedn;
  SGvted *svted, *evted;

  // next vertex
  SGvt *nxt, *prv;

  // for priority queue
  SGPQCont *pqc;
  int    pq_type;
  
};

struct _sged {

  int no;
  int sid;

  //
  // edge type
  // ポリゴンのオリジナルのエッジか新たに加えられたエッジかを判断する
  unsigned short type;

  // 経路エッジかどうかの判断はこのフラッグで行なう
  // path edge or not ?
  // added by T.Kanai 99.07.08
  BOOL isPathEdge;
  
  //
  // an original edge
  // (used if the type is EDGE_ORIGINAL)
  Sped *sped;

  //
  // length parameter
  // (used if the type is EDGE_ORIGINAL)
  double eprm;
  
  //
  // an original face
  // (used if the type is EDGE_ONFACE)
  Spfc *spfc;

  //
  // neighbor SGfc (if type is EDGE_ONFACE, lf = rf = NULL)
  //
  SGfc *lf, *rf;
  
  // SGed -> SGvt link
  SGvt *sv, *ev;

  // next, previous edge
  SGed *nxt, *prv;

  // for add_vertices_edges_sgraph
  int sgvt_id;

  // length of an edge (used in sgraph)
  double length;
};

struct _sgfcvt {

  SGvt *vt;

  SGfcvt *nxt, *prv;
  
};

struct _sgfc {

  int   no;
  int   sid;
  Spfc *spfc;

  int     sgfcvtn;
  SGfcvt *sfcvt, *efcvt;

  SGfc *nxt, *prv;
};
  
struct _sglp {

  // links to loop vertices
  int  sglpvtn;
  SGlpvt *slpvt, *elpvt;

  // links to loop edges
  int  sglpedn;
  SGlped *slped, *elped;

  // length
  double length;
  
  // calculated time
  double realtime;
  double usertime;
  double systime;
};

struct _sglpvt {

  SGvt *vt;
  SGlpvt *nxt, *prv;
  
};

struct _sglped {

  SGed *ed;
  SGlped *nxt, *prv;

};

//
// general defines and macros
//

#define SMD_ON          1
#define SMD_OFF         0
#define SMDNULL         -1
#define SMDZERO         0

#define PNTBLUE         2
#define PNTGREEN        1
#define PNTRED          0

#define EDGEWHITE       2
#define EDGEBLUE        1
#define EDGERED         0

#define FACEBLUE        2
#define FACEGREEN       1
#define FACERED         0

#define LOOPBLUE        1
#define LOOPRED         0

#define NOTCALCURATED   0
#define CALCURATED      1
#define SUCCEED         1
#define FAIL            0
#define INSIDE          0
#define OUTSIDE         1
#define ONLINE          2
#define INTERSECTION    3

// misc
#define SMDEPS          1.0e-16
#define SMDZEROEPS      1.0e-05
#define SMDZEROEPS2     1.0e-12
#define SMDPI		3.14159265358979
#define PITIMES2	6.283185	// 2 * pi
#define PIOVER2		1.570796	// pi / 2
#define NE		2.718282	// the venerable e
#define SQRT2		1.414214	// sqrt(2)
#define SQRT3		1.732051	// sqrt(3)
#define GOLDEN		1.618034	// the golden ratio
#define DTOR		0.017453	// convert degrees to radians
#define RTOD		57.29578	// convert radians to degrees

#define MULTI           10000.0

//#ifndef WIN32
extern void display(char *,...);
//#endif

#endif // _SMD_H

