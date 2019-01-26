//
// tile.h
//
// Copyright (c) 1997-1999 Takashi Kanai
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
//

#ifndef _TILE_H
#define _TILE_H

typedef struct _tile Tile;
typedef struct _tvector TVector;
typedef struct _tvertex TVertex;
typedef struct _tedge TEdge;
typedef struct _tface TFace;

struct _tile {

  // ppd
  Sppd *ppd;
  
  // Vertex
  int n_tvertex;
  int n_tvtid;
  TVertex *stvt, *etvt;

  // Edge
  int n_tedge;
  int n_tedid;
  TEdge *sted, *eted;

  // Face
  int n_tface;
  int n_tfcid;
  TFace *stfc, *etfc;

  // filename
  char filename[BUFSIZ];
  
};

// ppd vertex and parameter values
struct _tvector {

  // vertex
  Spvt *vt;

  // 境界の頂点かどうか
  //BOOL isBoundary;

  // for shape preserving mapping
  int n_degree;
  Vec2d *vec2d; // parameters for conformal mapping
  double *lambda;
  Vec2d uv; // parameter for shape preserving mapping

};
  
struct _tvertex {

  int no;
  int sid;
  Spvt *vt;

  TVertex *nxt, *prv;
  
};

struct _tedge {

  int no;
  int sid;
  TVertex *sv, *ev;

  // ppd open loop
  Splp *lp;

  // 再計算フラッグ
  BOOL isRecalc;

  // 経路を計算したときの gamma
  double sp_gamma;
  
  TEdge *nxt, *prv;
};

struct _tface {

  int no;
  int sid;
  int col;

  // パラメータ化を行っているかどうか
  BOOL isParameterized;
  
  // ppd part
  Sppt *pt;
  
  // edge link
  int n_tedge;
  TEdge **ed;

  // ppd closed loop
  Splp *clp;

  // TVector
  int n_tvector;
  TVector *tvec;

  // pattern type
  int tptype;

  // number of subdivision
  int ndiv;

  // center vertex ( only if tfc->tptype == TILE_PATTERN_RADIAL)
  Spvt *cv;

  // cylindrical face
  int num_edges_per_loop;
  Splp *(cyl_lp[2]);

  // remesh ppd
  Sppd *rppd;
  
  TFace *nxt, *prv;

};

// definitions
#define TILE_PATTERN_NONE     0
#define TILE_PATTERN_LATTICE  1
#define TILE_PATTERN_RADIAL   2

// functions
extern Tile *create_tile(void);
extern void free_tile(Tile *);
extern TVertex *create_tvertex(Tile *);
extern TVertex *list_tvertex(Tile *, Id);
extern TVertex *find_tvertex(Tile *, Spvt *);
extern void free_tvertex(TVertex *, Tile *);
extern TEdge *create_tedge(Tile *);
extern void free_tedge(TEdge *, Tile *);
extern TFace *create_tface(Tile *);
extern void free_tface(TFace *, Tile *);
extern TEdge *list_tedge( Tile *, Id );
extern BOOL isusedtedge( TEdge *, Tile * );
extern BOOL istedge_in_tface( TEdge* , TFace* );
extern void delete_tedge( TEdge *, Tile * );
extern void delete_tedge_lp( Splp*, Sppd* );
extern TFace *find_tface( Tile *, int );
extern void initialize_tvector( TVector * );
extern void tface_subdivide_ppdface(TFace *, Tile*, Sppd *);
extern void cyl_loop_subdivide_ppdface(TFace *, Sppd *);
extern void order_ppdedge(Sped *, Spvt *, Spvt *);
extern void tface_grouping(TFace *, Sppd *);
extern void cyl_tface_grouping(TFace *, Sppd *);
extern void ppdface_grouping( TFace*, Spfc*, int );
extern void rename_tvector( TFace* );
extern int tfc_num_neighbor( Spvt* );
extern void tfc_calc_boundary( TFace* );

#endif // _TILE_H  
