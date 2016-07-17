//
// esptree.h
//
// Copyright (c) 1997-1999 Takashi Kanai; All rights reserved. 
//

#ifndef _ESPTREE_H
#define _ESPTREE_H

// ESPTree 
struct _esptree {

  // ID
  int id;
  
  // root espenode child
  // (number of faces neighbor to source vertex)
  int      n_child;
  // child espenodes
  ESPEnode **child;

  // root espvnode child
  // (number of edges neighbor to source vertex)
  int      n_vchild;
  // child espvnodes
  ESPVnode **vchild;
  
  // number of enodes
  int      n_enode;
  int      id_enode;
  // number of vnodes
  int      n_vnode;
  int      id_vnode;

  // root vertex
  Spvt *root_vt;

  // parent tree (for multiple modes)
  ESPTree *parent;
  
  // gamma (distance between src to root vertex)
  double gamma;
  
};

// ESPEnode
struct _espenode {

  // ID
  int id;
  
  // ppd halfedge
  Sphe *he;
  
  // unfolded position of source vertex
  Vec  unfold_vec;
  
  // start and end parameter values of projection 
  // both are values between 0 to 1
  double sproj, eproj;

  // left child, right child
  int n_child;
  ESPEnode *lchild;
  ESPEnode *rchild;
  
  // parent
  ESPEnode *parent;

  // vnode (back pointer)
  ESPVnode *vnode;

  // vt (for occupy)
  Spvt *vt;
  
  // my tree
  ESPTree *tree;

  // for setting current enodelist;
  ESPELnode *elnode;

};

// ESPVnode
struct _espvnode {

  // ID
  int id;
  
  // ESPEnode 
  ESPEnode *enode;

  // unfolded position of source vertex
  Vec  unfold_vec;
  
  // my tree
  ESPTree *tree;
  
  // link to vertex 
  Spvt *vt;
  
};

// ESPEList
struct _espelist {

  int n_elnode;
  int id_elnode;
  ESPELnode *selnode, *eelnode;
  
};

// ESPELnode
struct _espelnode {

  int id;
  ESPEnode *enode;
  ESPELnode *nxt, *prv;

};
  
// defines
#define ONE_CHILD    1
#define TWO_CHILDREN 2

// functions
extern ESPTree *create_esptree(Spvt *, double);
extern void delete_espall(Sppd *);
extern void delete_esptree(ESPTree *);
extern ESPTree *initialize_esptree(Spvt *, double, ESPEnode *, ESPTree *, ESPEList *);
extern void update_esptree(Spvt *, ESPEnode *, ESPTree *, double, ESPEList *);
extern void update_esptree_leafs(ESPTree *, ESPEList *);
extern void update_espenode_leafs(ESPEnode *, ESPEList *);
extern void esptree_parent_route( ESPTree * );
extern void esptree_child_route( ESPTree * );
extern void esptree_child_vnode_route( ESPVnode * );
extern void esptree_child_enode_route( ESPEnode * );
extern double esptree_path_distance(ESPTree *, Vec *, Vec *);
extern void print_esp_path_route(Spvt *);
extern double tmp_path_distance( ESPTree *, Vec *, Vec * );
extern void tmp_path_route(ESPTree *, ESPEnode *, Vec *, Vec * );
extern void print_esptree_leafs(ESPTree *);
extern void print_espenode_leafs(ESPEnode *);
extern ESPEnode *create_espenode(void);
extern ESPEnode *insert_espenode(Sphe *, Vec *, double, double, ESPEnode *, ESPTree *, ESPEList *);
extern int cutoff_enode(ESPEnode *);
extern int delete_espenode(ESPEnode *);
extern int espenode_update_child(ESPTree *, ESPEnode *, int, Vec *, Sphe *, double, double, Sphe *, double, double, ESPEList *, BOOL );
extern int replace_espenode(ESPEnode *, ESPEnode *);
extern void espenode_to_proj( ESPEnode *, Vec *, BOOL *, double *, double *, BOOL *, double *, double * );
extern ESPVnode *create_espvnode(void);
extern ESPVnode *insert_espvnode(ESPEnode *, ESPTree *, Vec *, Spvt *);
extern void delete_espvnode(ESPVnode *);
extern ESPVnode *update_espvnode(Spvt *, ESPTree *, ESPEnode *, Vec *);
extern ESPEList *create_espelist(void);
extern void free_espelist(ESPEList *);
extern ESPELnode *create_espelnode(ESPEList *);
extern void free_espelnode(ESPELnode *, ESPEList *);

#endif // _ESPTREE_H
