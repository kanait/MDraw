//
// esptree.cxx
// exact shortest path tree functions
// based on Chen & Han algorithms
//
// Copyright (c) 1997-1999 Takashi Kanai; All rights reserved. 
//

#include "StdAfx.h"

#if defined(_DEBUG) && defined(_WINDOWS)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include <math.h>
#include <assert.h>
#include "smd.h"

#include "ppdvertex.h"
#include "ppdface.h"
#include "veclib.h"

#include "esp.h"
#include "esptree.h"

#include "params.h"

/////////////////////////////////////////////////////////////////////
// ESPTree: おおもと(src を root とする) のTree の生成，削除
/////////////////////////////////////////////////////////////////////

extern int n_esptree;

// create ESPTree
ESPTree *create_esptree( Spvt *vt, double gamma )
{
  ESPTree *tree;

  tree = (ESPTree *) malloc( sizeof(ESPTree) );
  tree->n_child = 0;
  tree->child = (ESPEnode **) NULL;
  tree->n_vchild = 0;
  tree->vchild = (ESPVnode **) NULL;
  tree->n_enode = 0;
  tree->id_enode = 0;
  tree->n_vnode = 0;
  tree->id_vnode = 0;

  tree->root_vt = vt;
  tree->gamma = gamma;

  tree->id = n_esptree;
  ++(n_esptree);

  return tree;
}

// delete all
void delete_espall( Sppd *ppd )
{
  // delete espvnode
  for ( Spvt *vt = ppd->spvt; vt != NULL; vt = vt->nxt ) {
    
    if ( vt->vnode != NULL ) {
      delete_espvnode( vt->vnode );
    }
    vt->vnode = NULL;
    
    if ( vt->tree != NULL ) {
      delete_esptree( vt->tree );
    }
    vt->tree = NULL;
    
  }

  // delete esptree
  ppd->esptree = NULL;
}

// delete esptree
void delete_esptree( ESPTree *esptree )
{
  if ( esptree == NULL ) return;

  // enode 
  int n_deleted = 0;
  if ( esptree->child != NULL ) {
    for ( int i = 0; i < esptree->n_child; ++i ) {
      n_deleted += delete_espenode( esptree->child[i] );
    }
    free( esptree->child );
  }

  // vnodes
  if ( esptree->vchild != NULL ) {
    for ( int i = 0; i < esptree->n_vchild; ++i ) {
      delete_espvnode( esptree->vchild[i] );
    }
    esptree->vchild = NULL;
  }

  free( esptree );
}

// tree の生成（初期動作すべて）
ESPTree *initialize_esptree( Spvt *root, double gamma, ESPEnode *enode,
			     ESPTree *parent, ESPEList *elist )
  // root: root vertex
  // gamma: root vertex から，その親の root vertex までの測地線距離
  // enode: 進入してきた enode (root が src の場合は NULL)
  // parent: tree の parent tree
  // elist: 現在の elist (root->child[i] の格納のため)
{
  ESPTree *tree;

  tree = create_esptree( root, gamma );
  tree->parent = parent;
  if ( enode == NULL ) {
    tree->n_child = ppdvertex_num_face( root );
  } else {
    tree->n_child = ppdvertex_num_face( root ) - 1;
  }
  tree->child = (ESPEnode **) malloc( tree->n_child * sizeof( ESPEnode * ) );
//   display("tree %d parent %d created.\n", tree->id,
// 	  (parent != NULL ) ? parent->id : -1 );
//   esptree_parent_route( tree );
  
  // insert child espenodes
  Sphe *rev_he;
  Sphe* he = root->sphe; 
  Spfc* fc = he->bpfc;
  int i = 0;
  while ( i < tree->n_child ) {
    //for ( int i = 0; i < tree->n_child; ++i ) {
    rev_he = ppdface_opposite_halfedge( fc, root );
    if ( enode != NULL ) {
      if ( enode->he->mate != rev_he ) {
	tree->child[i] = insert_espenode( rev_he, &(root->vec), 0.0, 1.0,
					  NULL, tree, elist );
	++i;
      }
    } else {
      tree->child[i] = insert_espenode( rev_he,  &(root->vec), 0.0, 1.0,
					NULL, tree, elist );
      ++i;
    }
    he = ppdvertex_ccwnext_halfedge( he ); 
    if (!he) break;
    fc = he->bpfc;
  }

  // insert vspvnodes at 1-ring neighborhood vertices
  int num = 0;
  Sped *ed = ppdvertex_first_edge( root );
  do {
    ++num;
    ed = ppdvertex_next_edge( ed, root );
  } while ( (ed != ppdvertex_first_edge( root )) && (ed != NULL) );

  tree->vchild = (ESPVnode **) malloc( num * sizeof(ESPVnode *) );
  //tree->n_vchild = num;

  i = 0;
  ed = ppdvertex_first_edge( root );
  do {
    Spvt *vt = another_vt( ed, root );
    // cur_node がない場合は，root までの距離
    if ( vt->vnode != NULL ) {

      Vec vec;
      ESPTree *ptree, *otree;

      // dis0 の tree
      otree = ( tree != NULL ) ? tree : NULL;
      double dis0 = tmp_path_distance( otree, &(root->vec), &(vt->vec) );
      
      ESPEnode *prev_node = vt->vnode->enode;
      if ( prev_node != NULL ) {
	// prev_node の bar_i = その子供の unfold_vec
	// prev_node の子供は二つある(はず)
// 	display("prev_node %d\n", prev_node->id );
	ptree = vt->vnode->tree;
	ptree = ptree->parent;
	unfold_imagesource( prev_node->he, &(prev_node->unfold_vec), &(vec) );
      } else {
	// vt->vnode->enode == NULL
	// -> a は vt->vnode->tree->root_vt の 1-ring 近傍
	ptree = vt->vnode->tree;
	Spvt *root_vt = ptree->root_vt;
	vec.x = root_vt->vec.x;
	vec.y = root_vt->vec.y;
	vec.z = root_vt->vec.z;
	ptree = ptree->parent;
      }
      // prev_node (n')
      double dis1 = esptree_path_distance( ptree, &(vec), &(vt->vec) );
      // if ( n が n' に対し角CAB を占有する ) {
      if ( dis0 < dis1 ) {
	tree->vchild[i] = update_espvnode( vt, tree, NULL, &(root->vec) );
// 	update_esptree( vt, NULL, tree, V3DistanceBetween2Points(&(root->vec), &(vt->vec)),
// 			elist );
	if ( vt->tree != NULL ) {
 	  delete_esptree( vt->tree );
 	  vt->tree = NULL;
	}
        ++i;
      }
      
    } else { // vnode == NULL
      tree->vchild[i] = update_espvnode( vt, tree, NULL, &(root->vec) );
//       update_esptree( vt, NULL, tree, V3DistanceBetween2Points(&(root->vec), &(vt->vec)),
// 		      elist );
      if ( vt->tree != NULL ) {
	delete_esptree( vt->tree );
	vt->tree = NULL;
      }
      ++i;
    }
    ed = ppdvertex_next_edge( ed, root );
  } while ( (ed != ppdvertex_first_edge( root )) && (ed != NULL) );
  tree->n_vchild = i;

  return tree;
}

// esptree の交換，生成，削除
void update_esptree( Spvt *vt, ESPEnode *node, ESPTree *parent_tree,
		     double gamma, ESPEList *elist )
{
  if ( vt == NULL ) return;

  if ( vt->tree != NULL ) {
    delete_esptree( vt->tree );
    vt->tree = NULL;
  }
    
  // vt を root とする sub-tree
  ESPTree *tree = initialize_esptree( vt, gamma, node, parent_tree, elist );
  vt->tree = tree;

}  

void esptree_parent_route( ESPTree *tree )
{
  if ( tree == NULL ) return;
  
  display("tree %d route.\n", tree->id );

  ESPTree *tr = tree;
  while ( tr != NULL ) {
    display("%d ", tr->id );
    tr = tr->parent;
  }
  display("\n");
}

void esptree_child_route( ESPTree *tree )
{
  if ( tree == NULL ) return;

  display("tree %d parent %d begin\n", tree->id,
	  ( tree->parent != NULL ) ? tree->parent->id : -1 );
  int i;
  if ( tree->vchild != NULL ) {
    for ( i = 0; i < tree->n_vchild; ++i ) {
      display("tree %d's vchild No.%d\n", tree->id, i );
      esptree_child_vnode_route( tree->vchild[i] );
    }
  }
  if ( tree->child != NULL ) {
    for ( i = 0; i < tree->n_child; ++i ) 
      esptree_child_enode_route( tree->child[i] );
  }
  display("tree %d end\n", tree->id );
}

void esptree_child_vnode_route( ESPVnode *vnode )
{
  if ( vnode == NULL ) return;

  display("vnode %d tree %d\n", vnode->id, vnode->tree->id );

  if ( vnode->vt->tree != NULL ) {
    ESPTree *tree =vnode->vt->tree;
    if ( tree->parent == vnode->tree ) {
      esptree_child_route( tree );
    }
  }
}

void esptree_child_enode_route( ESPEnode *enode )
{
  if ( enode == NULL ) return;

  display("enode %d parent %d tree %d\n", enode->id,
	  (enode->parent != NULL) ? enode->parent->id : -1,
	  enode->tree->id );

  if ( enode->lchild != NULL ) {
    esptree_child_enode_route( enode->lchild );
  }
  if ( enode->rchild != NULL ) {
    esptree_child_enode_route( enode->rchild );
  }
  
  if ( enode->vnode != NULL ) {
    assert( enode->tree == enode->vnode->tree );
    esptree_child_vnode_route( enode->vnode );
  }
}
    
    
  
			 
// path distance for multiple trees
// tree(s) までの経路の長さ + tree からの経路の長さ
double esptree_path_distance( ESPTree *otree, Vec *bar_i, Vec *vec )
  {
  // vec から現在の tree の root (vt) までの距離
  double dis = V3DistanceBetween2Points( bar_i, vec );
//   display("first dis %g\n", dis );

  // current_tree
  ESPTree *tree = otree;
  
  while ( tree != NULL ) {
    dis += tree->gamma;
//     display("\tdis %g tree %d root_vt %d gamma %g\n",
// 	    dis,
// 	    tree->id,
// 	    tree->root_vt->no,
// 	    tree->gamma );
    tree = tree->parent;
  }
//   display("distance %g\n", dis);
  return dis;
}

// path route for multiple trees
void print_esp_path_route( Spvt *ovt )
{
  if ( ovt->vnode == NULL ) return;

  Spvt *vt = ovt;
  ESPTree *tree = ovt->vnode->tree;

  while ( tree != NULL ) {

    // begin tracing a route for subtree
    ESPVnode *vnode = vt->vnode;
    if ( vnode != NULL ) {
      display("\tvt %d vnode %d tree %d\n", vt->no, vnode->id,
	      tree->id );
      display("\t(tree distance: %g gamma %g)\n",
	      V3DistanceBetween2Points( &(vt->vnode->unfold_vec),
					&(vt->vec) ), tree->gamma );
    } else {
      display("\tvnode is not found.\n");
      break;
    }

    ESPEnode *enode = vnode->enode;
    if ( enode == NULL ) {
      display("\tvt %d is 1-ring neighborhood of vt %d\n",
	      vt->no, tree->root_vt->no );
    }
    
    
	    
    while ( enode != NULL ) {
      display("\ted %d enode %d tree %d\n", enode->he->ed->no,
	      enode->id, enode->tree->id );
      enode = enode->parent;
    }

    vt = tree->root_vt;
    display("\troot_vt %d\n", vt->no );
    if ( tree->parent != NULL ) {
      display("tree parennt %d\n", tree->parent->id );
    }
    tree = tree->parent;
      

  }

}

// path route for multiple trees
double tmp_path_distance( ESPTree *cur_tree, Vec *bar_i, Vec *vec )
{
  if ( cur_tree == NULL ) return 0;

  double dis = 0;

  dis += V3DistanceBetween2Points( bar_i, vec );
  dis += cur_tree->gamma;

  Spvt *vt = cur_tree->root_vt;
  if ( vt->vnode == NULL ) return dis;
  ESPTree *tree = vt->vnode->tree;

  while ( tree != NULL ) {

    dis += tree->gamma;
    //dis += V3DistanceBetween2Points( &(vt->vnode->unfold_vec), &(vt->vec) );
//     display("\t(tree distance: %g gamma %g)\n",
// 	    V3DistanceBetween2Points( &(vt->vnode->unfold_vec),
// 				      &(vt->vec) ), tree->gamma );
    // begin tracing a route for subtree

    // TEMP
    //    ESPVnode *vnode = vt->vnode;
    
//     if ( vnode != NULL ) {
//       display("\tvt %d vnode %d tree %d\n", vt->no, vnode->id,
//  	      tree->id );
//     } else {
//       display("\tvnode is not found.\n");
//       break;
//     }
	      
//     ESPEnode *enode = vnode->enode;
//     if ( enode == NULL ) {
//       display("\tvt %d is 1-ring neighborhood of vt %d\n",
// 	      vt->no, tree->root_vt->no );
//     }
    
//     while ( enode != NULL ) {
// //       display("\ted %d enode %d tree %d\n", enode->he->ed->no,
// // 	      enode->id, enode->tree->id );
//       enode = enode->parent;
//     }

//     vt = tree->root_vt;
    tree = tree->parent;

  }

//   display("(in tmp_path_distance) distance %g\n", dis );
  return dis;
}

// path route for multiple trees
void tmp_path_route( ESPTree *cur_tree, ESPEnode *cur_node, Vec *bar_i, Vec *vec )
{
  //if ( ovt->vnode == NULL ) return;

  if ( cur_tree == NULL ) return;
  //if ( cur_node == NULL ) return;

  display("\t(distance: %g)\n", V3DistanceBetween2Points( bar_i, vec ) );

  // cur_node から cur_tree->vt までの道のり
  ESPEnode *node = cur_node;
  while ( node != NULL ) {
    display("\ted %d enode %d tree %d\n", node->he->ed->no,
	    node->id, node->tree->id );
    node = node->parent;
  }
  
  Spvt *vt = cur_tree->root_vt;
  if ( vt->vnode == NULL ) return;
  ESPTree *tree = vt->vnode->tree;

  while ( tree != NULL ) {

    display("\t(tree distance: %g gamma %g)\n",
	    V3DistanceBetween2Points( &(vt->vnode->unfold_vec),
				      &(vt->vec) ), tree->gamma );
    // begin tracing a route for subtree
    ESPVnode *vnode = vt->vnode;
    if ( vnode != NULL ) {
      display("\tvt %d vnode %d tree %d\n", vt->no, vnode->id,
	      tree->id );
    } else {
      display("\tvnode is not found.\n");
      break;
    }
	      
    ESPEnode *enode = vnode->enode;
    if ( enode == NULL ) {
      display("\tvt %d is 1-ring neighborhood of vt %d\n",
	      vt->no, tree->root_vt->no );
    }
    
    while ( enode != NULL ) {
      display("\ted %d enode %d tree %d\n", enode->he->ed->no,
	      enode->id, enode->tree->id );
      enode = enode->parent;
    }

    vt = tree->root_vt;
    tree = tree->parent;

  }

}

void print_esptree_leafs( ESPTree *tree )
{
  void print_espenode_leafs( ESPEnode * );
  if ( tree == NULL ) return;

  for ( int i = 0; i < tree->n_child; ++i )
    print_espenode_leafs( tree->child[i] );
}

void print_espenode_leafs( ESPEnode *enode )
{
  if ( enode == NULL ) return;

  if ( enode->lchild != NULL ) print_espenode_leafs( enode->lchild );
  if ( enode->rchild != NULL ) print_espenode_leafs( enode->rchild );
  
  if ( enode->vnode != NULL ) {
    ESPVnode *vnode = enode->vnode;
    if ( vnode->tree != NULL ) {
      Spvt *vt = vnode->vt;
      if ( vt->tree != NULL ) {
	if ( vt->tree->id > enode->tree->id ) 
	  print_esptree_leafs( vt->tree );
      }
    }
  }

  if ( !(enode->n_child) ) {
    display("(leaf) enode %d tree %d parent %d elist %d\n",
	    enode->id, enode->tree->id,
	    ( enode->tree->parent != NULL ) ? enode->tree->parent->id : 0,
	    ( enode->elnode != NULL ) ? 1 : 0 );
  }

}

/////////////////////////////////////////////////////////////////////
// ESPEnode functions
/////////////////////////////////////////////////////////////////////
ESPEnode *create_espenode( void )
{
  ESPEnode* node = (ESPEnode *) malloc( sizeof(ESPEnode) );

  // corresponded halfedge
  node->he = (Sphe *) NULL;

  // unfold_vec, sproj, eproj are not initialized

  // id initialize
  node->id = SMDNULL;
  // childs
  node->n_child = 0;
//   node->child   = (ESPEnode **) NULL;
  node->lchild = (ESPEnode *) NULL;
  node->rchild = (ESPEnode *) NULL;

  // for setting current enode1 id
  // obsolete
//   node->cur_id = SMDNULL;
//   node->cur_eary = (ESPEnode **) NULL;

  // the link to a current elnode
  node->elnode = NULL;

  // parent node
  node->parent = NULL;

  // my tree
  node->tree = NULL;

  // vnode
  node->vnode = NULL;

  // vt
  node->vt = NULL;
  
  // for occupy
  //node->occupy = NULL;

  return node;
}
  
ESPEnode *insert_espenode( Sphe *he, Vec *vec, double sprm, double eprm,
			   ESPEnode *parent, ESPTree *tree, 
			   ESPEList *elist )
{
  ESPEnode* node = create_espenode();

  // corresponded halfedge
  node->he = he;
  // set 3D positions of a unfolded image source (2D, indeed)
  node->unfold_vec.x = vec->x;
  node->unfold_vec.y = vec->y;
  node->unfold_vec.z = vec->z;
  // parameters of projections
  node->sproj = sprm;
  node->eproj = eprm;

  node->parent = parent;
  node->tree = tree;
//   if ( parent != NULL ) {
//     node->tree = parent->tree;
//   }
  node->id = tree->id_enode;

  ++(tree->id_enode);
  ++(tree->n_enode);

  // elnode
  ESPELnode *elnode = create_espelnode( elist );
  // mutual links.
  elnode->enode = node;
  node->elnode = elnode;

  return node;
}

// delete one of node's children which is impossible 
// to define a shortest sequence
int cutoff_enode( ESPEnode *node )
{
  if ( node == NULL ) return 0;
  // the node doesn't have two children
  //assert( node->n_child == TWO_CHILDREN );
  if ( node->n_child < TWO_CHILDREN ) return 0;

  Sphe *lhe = node->lchild->he;
  Sphe *rhe = node->rchild->he;
  Spvt *c = lhe->vt;
  Spvt *a = rhe->vt; 
  Spvt *b = rhe->nxt->vt;
  // ac, ab, ai
  Vec ac, ab, ai;
  V3Sub( &(c->vec), &(a->vec), &ac );
  V3Sub( &(b->vec), &(a->vec), &ab );
  V3Sub( &(node->lchild->unfold_vec), &(a->vec), &ai );
  double lcos = fabs( V3Cosign( &ac, &ai ) );
  double rcos = fabs( V3Cosign( &ab, &ai ) );

  // remain a node which has the smaller angle
  // rnode: remain node
  // dnode: delete node
  int n = 0;
  if ( lcos > rcos ) { // lchild が残る
    n += delete_espenode( node->rchild );
    node->rchild = NULL;
  } else { // rchild が残る
    n += delete_espenode( node->lchild );
    node->lchild = NULL;
  }
  node->n_child = ONE_CHILD;
  
  return n;
}

// delete enode
int delete_espenode( ESPEnode *node )
{
  int n = 0;
  
  if ( node == NULL ) return 0;

  //
  // delete node's children
  //

//   display("delete node %d (tree %d) begin.\n", node->id, node->tree->id );
  // delete lchild
  ESPEnode *denode;
  if ( node->lchild != NULL ) {
//     display("node %d's lchild %d parent %d\n", node->id,
// 	    node->lchild->id, node->lchild->parent->id );
    node->lchild->parent = NULL;
    denode = node->lchild;
    node->lchild = NULL;
    n += delete_espenode( denode );
//     display("node %d's lchild deleted.\n", node->id );
  }
  
  // delete rchild
  if ( node->rchild != NULL ) {
//     display("node %d's rchild %d parent %d\n", node->id, 
// 	    node->rchild->id, node->rchild->parent->id );
    node->rchild->parent = NULL;
    denode = node->rchild;
    node->rchild = NULL;
    n += delete_espenode( denode );
//     display("node %d's rchild deleted.\n", node->id );
  }
  
  // 親からのリンクをきる
  ESPEnode *parent = node->parent;
  if ( parent != NULL ) {
    
    if ( parent->lchild == node ) parent->lchild = NULL;
    if ( parent->rchild == node ) parent->rchild = NULL;
    node->parent = NULL;
    
  } else { // node は tree の直下にある
    
    ESPTree *tree = node->tree;
    for ( int i = 0; i < tree->n_child; ++i ) {
      if ( tree->child[i] == node ) tree->child[i] = NULL;
    }
    
  }
    
  // current enode1 link cut
  if ( node->elnode != NULL ) {
    node->elnode->enode = NULL;
    node->elnode = NULL;
  }
  
  // vnode link cut and delete sub-tree
  // vnode があれば，subtree が存在する可能性がある
  ESPVnode *dvnode;
  if ( node->vnode != NULL ) {
    dvnode = node->vnode;

    if ( dvnode->vt->tree != NULL ) {
      ESPTree *tree = dvnode->vt->tree;
      // tree が node->tree の子であれば消去する
      if ( tree->parent == node->tree ) {
// 	display("bb\n");
	delete_esptree( tree );
// 	display("bbb\n");
	dvnode->vt->tree = NULL;
      }
    }
    dvnode->enode = NULL;
    node->vnode = NULL;
    delete_espvnode( dvnode );
  }

  --(node->tree->n_enode);
  
//   display("enode %d (tree %d) deleted.\n", node->id, node->tree->id );
  // delete node
  free( node );

  ++n;
  
  return n;
}

//
// update esptree child
// normal sequence tree
//
int espenode_update_child( ESPTree *tree, ESPEnode *cur_node,
			   int n_child, Vec *bar_i,
			   Sphe *he0, double sprm0, double eprm0, // lchild
			   Sphe *he1, double sprm1, double eprm1, // rchild
			   ESPEList *elist, BOOL replace )
{
  int n_deleted = 0;

  if ( cur_node == NULL ) return n_deleted;

  cur_node->n_child = n_child;
  
  if ( he0 != NULL ) {

    ESPEnode *lnode = insert_espenode( he0, bar_i, sprm0, eprm0,
				       cur_node, tree, elist );
//     display("(lchild) new_node %d parent %d ed %d s %g e %g\n",
// 	    lnode->id, cur_node->id, he0->ed->no, sprm0, eprm0 );

    if ( (params.save_path != TRUE) && (replace == TRUE) ) {
      // for modified sequence tree
//       display("cur_node %d replaced.\n", cur_node->id );
      int lval = replace_espenode( cur_node, lnode );
      // parent is not found
      if ( !lval ) {
	cur_node->lchild = lnode;
      }
      n_deleted += lval;
      
    } else {
      cur_node->lchild = lnode;
    }
  }
    
  if ( he1 != NULL ) {

    ESPEnode *rnode = insert_espenode( he1, bar_i, sprm1, eprm1,
				       cur_node, tree, elist );
//     display("(rchild) new_node %d parent %d ed %d s %g e %g\n",
// 	    rnode->id, cur_node->id, he1->ed->no, sprm1, eprm1 );

    //cur_node->rchild = rnode;

    if ( (params.save_path != TRUE) && (replace == TRUE) ) {
      
      // for modified sequence tree
//       display("cur_node %d replaced.\n", cur_node->id );
      int rval = replace_espenode( cur_node, rnode );
      // parent is not found
      if ( !rval ) {
	cur_node->rchild = rnode;
      }
      n_deleted += rval;
      
    } else {
      cur_node->rchild = rnode;
    }
  }
    
  return n_deleted;
}

//
// for modified sequence tree
// the current leaf will be discarded and replaced by its child
//
int replace_espenode( ESPEnode *cur_node, ESPEnode *new_node )
{
  int n_deleted = 0;

  // one of parent's children will be replaced by new_node
  ESPEnode *parent = cur_node->parent;

  // parent is not found
  if ( parent == NULL ) return n_deleted;

  // parent is found
  if ( parent->lchild == cur_node ) {
    parent->lchild = new_node;
    new_node->parent = parent;
  }
  
  if ( parent->rchild == cur_node ) {
    parent->rchild = new_node;
    new_node->parent = parent;
  }
  assert( cur_node->lchild == NULL );
  assert( cur_node->rchild == NULL );
  
  // delete cur_node
  n_deleted += delete_espenode( cur_node );

  return n_deleted;
}

//
// espenode to project
//
void espenode_to_proj( ESPEnode *cur_node, // input 
		       Vec *bar_i,
		       BOOL *ca_proj, double *ca_sprm, double *ca_eprm,
		       BOOL *ab_proj, double *ab_sprm, double *ab_eprm )
{
  if ( cur_node == NULL ) return;
  if ( cur_node->he->mate == NULL ) return;

  //  I を \bar{I} に広げる;
  // (\bar{I} は △ABC ( e の影の面 ) と同一平面上にある)

  unfold_imagesource( cur_node->he, &(cur_node->unfold_vec), bar_i );

  // 
  // compute projections Proj_{AB}^{\bar{I}}, Proj_{CA}^{\bar{I}}
  // (Proj_{AB}^{\bar{I}}, Proj_{CA}^{\bar{I}} を計算)
  //
  // どこかの面に投影しても結果は同じ
  // - 干渉判定だけは二次元で考える
  // - 交点の算出は三次元座標値を使う
  // 
  // project 3D points (A, B, C, bar_i) to 2D plane

  // edges of a face ofc
  Sphe *bc = cur_node->he->mate;
  Sphe *ca = bc->nxt;
  Sphe *ab = ca->nxt;
  // set mate(bc)'s parameter
  double sproj = 1.0 - cur_node->eproj;
  double eproj = 1.0 - cur_node->sproj;

  // vertices of face ofc
  Spvt *b = bc->vt;
  Spvt *c = ca->vt;
  Spvt *a = ab->vt;
      
  // face
  Spfc *ofc = bc->bpfc;

  // TEMP
//       double pa, pb, pc, pd;
//       ppdface_to_plane( ofc, &pa, &pb, &pc, &pd );
//       display("(plane) a %g b %g c %g d %g\n", pa, pb, pc, pd );
//       display("(plane) ");
//       //display("old_i %g ", a * old_i->x + b * old_i->y + c * old_i->z + d );
//       display("bar_i %g\n", pa * bar_i.x + pb * bar_i.y + pc * bar_i.z + pd );
//       display("bar_i %g %g %g\n", bar_i.x, bar_i.y, bar_i.z );
  // TEMP
      
  // compute Proj_{CA}^{\bar{I}} parameter (s, e)
  *ca_proj = ProjectHalfedgeParam3D( bar_i,
				     // BC, normal vectors and parameters
				     &(b->vec), &(c->vec), &(ofc->nrm),
				     sproj, eproj,
				     // CA and computed parameters
				     // ( if ca_proj == TRUE )
				     &(c->vec), &(a->vec),
				     ca_sprm, ca_eprm );
  // compute Proj_{AB}^{\bar{I}} parameter (s, e)
  *ab_proj = ProjectHalfedgeParam3D( bar_i,
				     // BC, normal vectors and parameters
				     &(b->vec), &(c->vec), &(ofc->nrm),
				     sproj, eproj,
				     // AB and computed parameters
				     // ( if ab_proj == TRUE )
				     &(a->vec), &(b->vec),
				     ab_sprm, ab_eprm );
	      
//       display("(proj) ca %d ab %d\n", ca_proj, ab_proj );
//       display("(ca) sprm %g eprm %g (ab) sprm %g eprm %g\n",
// 	      ca_sprm, ca_eprm, ab_sprm, ab_eprm );
}
      
/////////////////////////////////////////////////////////////////////
// ESPVnode functions
/////////////////////////////////////////////////////////////////////

// create espvnode
ESPVnode *create_espvnode( void )
{
  ESPVnode *node = (ESPVnode *) malloc( sizeof(ESPVnode) );

  // corresponding enode
  node->enode = (ESPEnode *) NULL;

  // a link to a vertex 
  node->vt = (Spvt *) NULL;

  return node;
}

// insert espvnode
ESPVnode *insert_espvnode( ESPEnode *enode, ESPTree *tree, Vec *vec, Spvt *vt )
{
  ESPVnode *node = create_espvnode();

  // corresonding enode
  node->enode = enode;
  if ( enode != NULL ) {
    enode->vnode = node;
  }

  // ID
  node->id = tree->id_vnode;
  ++( tree->id_vnode );
  ++( tree->n_vnode );
  
  // my tree
  node->tree = tree;
  
  // unfolded vector
  node->unfold_vec.x = vec->x;
  node->unfold_vec.y = vec->y;
  node->unfold_vec.z = vec->z;

  // corresponding vertex
  node->vt = vt;

//   display("vt %d vnode %d tree %d created.\n", vt->no,
// 	  node->id, tree->id );

  return node;
}

void delete_espvnode( ESPVnode *node )
{
  if ( node == NULL ) return;
  
//   display("vnode %d (vt %d)deleted.\n", node->id, node->vt->no );
  if ( node->vt != NULL ) {
    if ( node->vt->vnode == node ) {
      node->vt->vnode = NULL;
      node->vt = NULL;
    }
  }

  // node->enode のリンクをきる
  if ( node->enode != NULL ) {
    
    node->enode->vnode = NULL;
    node->enode = NULL;
    
  } else { // node は tree の直下にある

    ESPTree *tree = node->tree;
    for ( int i = 0; i < tree->n_vchild; ++i ) {
      if ( tree->vchild[i] == node ) tree->vchild[i] = NULL;
    }
    
  }
    
  --(node->tree->n_vnode);

  free( node );
}

// create, update espvnode 
ESPVnode *update_espvnode( Spvt *vt, ESPTree *tree,
			   ESPEnode *cur_node, Vec *bar_i )
  // vt:   生成する vnode の頂点
  // tree: vnode の属する tree
  // cur_node: vnode の経路の起点となる cur_node
  //           (NULL の場合は，vt は その tree の root_vt の 1-ring 近傍にある)
  // bar_i: root_vt を cur_node を回転軸として平面上に展開したもの
{
  if ( vt->vnode != NULL ) {
    delete_espvnode( vt->vnode );
  }

  ESPVnode *vnode = insert_espvnode( cur_node, tree, bar_i, vt );
  vt->vnode = vnode;

  // enode->vt: (for occupation)
  // このポインタで以前 vnode があったかどうかを判断する
  // update_espenode_leafs で用いる
  if ( cur_node != NULL ) {
    cur_node->vt = vt;
  }

//   if ( vt->tree ) {
    
//     // vt->tree->parent の更新
//     if ( vt->tree->parent != NULL ) {
//       if ( vt->tree->id > tree->id ) {
// 	vt->tree->parent = tree;
//       }
//     }
//     // vt->tree の gamma の更新
//     vt->tree->gamma = V3DistanceBetween2Points( bar_i, &(vt->vec) );

//   }

//   if ( vt->tree ) {

//     // 葉を調べて見る
//     print_esptree_leafs( vt->tree );
    
//   }

//   display("vt %d vnode updated. vnode %d tree %d\n",
// 	  vt->no, vt->vnode->id, vt->vnode->tree->id );

  return vnode;
}

/////////////////////////////////////////////////////////////////////
// ESPEList functions
/////////////////////////////////////////////////////////////////////

// create ESPEList
ESPEList *create_espelist()
{
  ESPEList *elist;

  elist = (ESPEList *) malloc( sizeof( ESPEList ) );
  elist->n_elnode = 0;
  elist->id_elnode = 0;
  elist->selnode = elist->eelnode = NULL;

  return elist;
}

// free ESPEList
void free_espelist( ESPEList *elist )
{
  if ( elist == NULL ) return;

  ESPELnode *elnode, *nelnode;
  for ( elnode = elist->selnode; elnode != NULL; elnode = nelnode ) {

    nelnode = elnode->nxt;

    // cut off mutual links
    if ( elnode->enode != NULL ) {
      elnode->enode->elnode = NULL;
      elnode->enode = NULL;
    }
    
    free_espelnode( elnode, elist );
    
  }
  free( elist );
}

/////////////////////////////////////////////////////////////////////
// ESPELnode functions
/////////////////////////////////////////////////////////////////////

// create ESPELnode
ESPELnode *create_espelnode( ESPEList *elist )
{
  ESPELnode *elnode;

  elnode = (ESPELnode *) malloc( sizeof(ESPELnode) );

  elnode->nxt = (ESPELnode *) NULL;
  if ( elist->selnode == (ESPELnode *) NULL ) {
    elnode->prv   = (ESPELnode *) NULL;
    elist->eelnode = elist->selnode = elnode;
  } else {
    elnode->prv   = elist->eelnode;
    elist->eelnode = elist->eelnode->nxt = elnode;
  }

  elnode->id = elist->id_elnode;
  ++( elist->id_elnode );
  
  elnode->enode = NULL;

  ++( elist->n_elnode );
  
  return elnode;
}

// free ESPELnode
void free_espelnode( ESPELnode *elnode, ESPEList *elist )
{
  if ( elnode == (ESPELnode *) NULL ) return;
  
  if ( elist->selnode == elnode ) {
    if ( (elist->selnode = elnode->nxt) != (ESPELnode *) NULL )
      elnode->nxt->prv = (ESPELnode *) NULL;
    else {
      elist->eelnode = (ESPELnode *) NULL;
    }
  } else if ( elist->eelnode == elnode ) {
    elnode->prv->nxt = (ESPELnode *) NULL;
    elist->eelnode = elnode->prv;
  } else {
    elnode->prv->nxt = elnode->nxt;
    elnode->nxt->prv = elnode->prv;
  }

  --( elist->n_elnode );
  free( elnode );
}

//
// OBSOLETE
//

void update_esptree_leafs( ESPTree *tree, ESPEList *elist )
{
  if ( tree == NULL ) return;

//   display("tree %d leafs update.\n", tree->id );

  for ( int i = 0; i < tree->n_child; ++i )
    update_espenode_leafs( tree->child[i], elist );
}

void update_espenode_leafs( ESPEnode *enode, ESPEList *elist )
{
  if ( enode == NULL ) return;
  if ( enode->he->mate == NULL ) return;

  // child enode の探索
  if ( enode->lchild != NULL ) update_espenode_leafs( enode->lchild, elist );
  if ( enode->rchild != NULL ) update_espenode_leafs( enode->rchild, elist );
      
  ESPVnode *vnode = enode->vnode;
  Spvt *vt;
  
  if ( vnode != NULL ) {  // enode が vt を占有している
    // vt->tree != NULL の場合 (subtree を持つ)
    // subtree のアップデート
    // 子供の木だけをアップデートする
    vt = vnode->vt;
    if ( vt->tree != NULL ) {
      if ( vt->tree->parent == enode->tree ) {
	//if ( vt->tree->id > enode->tree->id ) 
// 	display("vt %d tree %d leafs update.\n", vt->no, vt->tree->id );
// 	update_esptree_leafs( vt->tree, elist );
      }
    }

  } else { //  enode は vt を占有していない

    // vnode == NULL の場合
    vt = enode->vt;
    if ( vt != NULL ) { // かつては占有していた

      // cur_node (n)
      ESPTree *otree = enode->tree;
      Vec ovec, pvec;
      unfold_imagesource( enode->he, &(enode->unfold_vec), &(ovec) );
      double dis0 = tmp_path_distance( otree, &(ovec), &(vt->vec) );
      
      // prev_node (n')
//       display("enode %d (tree %d) は以前 vt %d を占有していた\n",
// 	      enode->id,
// 	      otree->id,
// 	      vt->no );

      if ( vt->vnode == NULL ) { // 今はもう何も占有していない

	// 復活
	BOOL ca_proj, ab_proj;
	double ca_sprm, ca_eprm, ab_sprm, ab_eprm;
	Vec bar_i;
	espenode_to_proj( enode, &(bar_i), 
			  &ca_proj, &ca_sprm, &ca_eprm,
			  &ab_proj, &ab_sprm, &ab_eprm );
	
	Sphe *bc = enode->he->mate; Sphe *ca = bc->nxt; Sphe *ab = ca->nxt;
	if ( (ca_proj == TRUE) && (enode->lchild == NULL) ) {
	  Sphe *ca = bc->nxt;
	  (void) espenode_update_child( otree, enode,
					ONE_CHILD,
					&(bar_i),
					ca, ca_sprm, ca_eprm,
					NULL, 0.0, 0.0,
					elist, FALSE );
//  	  display("enode %d fukkatu!.\n", enode->lchild->id );
	}
	if ( (ab_proj == TRUE) && (enode->rchild == NULL) ) {
	  (void) espenode_update_child( otree, enode,
					ONE_CHILD,
					&(bar_i),
					NULL, 0.0, 0.0,
					ab, ab_sprm, ab_eprm,
					elist, FALSE );
//  	  display("enode %d fukkatu!.\n", enode->rchild->id );
	}
	enode->n_child = 0;
	if ( enode->lchild != NULL ) ++(enode->n_child);
	if ( enode->rchild != NULL ) ++(enode->n_child);
	
	(void) update_espvnode( vt, otree, enode, &(bar_i) );
	
      } else { // 今も何かが占有している
	
	// すでにある vt の enode との距離の評価を行う
	// 新たな距離が短ければ，vnode をまた新たに生成する
	ESPEnode *penode = vt->vnode->enode;
	ESPTree *ptree = vt->vnode->tree;
	if ( penode != NULL ) {
	  // prev_node の bar_i = その子供の unfold_vec
	  // prev_node の子供は二つある(はず)
	  //ptree = ptree->parent;
	  unfold_imagesource( penode->he, &(penode->unfold_vec), &(pvec) );
	} else {
	  // vt->vnode->enode == NULL
	  // -> a は vt->vnode->tree->root_vt の 1-ring 近傍
	  Spvt *root_vt = ptree->root_vt;
	  pvec.x = root_vt->vec.x;
	  pvec.y = root_vt->vec.y;
	  pvec.z = root_vt->vec.z;
	}
	double dis1 = esptree_path_distance( ptree, &(pvec), &(vt->vec) );
	
	if ( dis0 < dis1 ) { // n が n' に対し角CAB を占有するようになった 
	  
// 	  display("enode %d (tree %d) は penode %d (tree %d) にかわり占有する\n",
// 		  enode->id, otree->id,
// 		  (penode != NULL) ? penode->id : -1,
// 		  ptree->id );
	  
	  (void) cutoff_enode( penode );
	  (void) update_espvnode( vt, otree, enode, &(ovec) );

	  // 復活
	  BOOL ca_proj, ab_proj;
	  double ca_sprm, ca_eprm, ab_sprm, ab_eprm;
	  Vec bar_i;
	  espenode_to_proj( enode, &(bar_i), 
			    &ca_proj, &ca_sprm, &ca_eprm,
			    &ab_proj, &ab_sprm, &ab_eprm );
	
	  Sphe *bc = enode->he->mate; Sphe *ca = bc->nxt; Sphe *ab = ca->nxt;
	  if ( (ca_proj == TRUE) && (enode->lchild == NULL) ) {
	    Sphe *ca = bc->nxt;
	    (void) espenode_update_child( otree, enode,
					  ONE_CHILD,
					  &(bar_i),
					  ca, ca_sprm, ca_eprm,
					  NULL, 0.0, 0.0,
					  elist, FALSE );
//  	    display("enode %d fukkatu!.\n", enode->lchild->id );
	  }
	  if ( (ab_proj == TRUE) && (enode->rchild == NULL) ) {
	    (void) espenode_update_child( otree, enode,
					  ONE_CHILD,
					  &(bar_i),
					  NULL, 0.0, 0.0,
					  ab, ab_sprm, ab_eprm,
					  elist, FALSE );
//  	    display("enode %d fukkatu!.\n", enode->rchild->id );
	  }
	  enode->n_child = 0;
	  if ( enode->lchild != NULL ) ++(enode->n_child);
	  if ( enode->rchild != NULL ) ++(enode->n_child);
	}
	
      }

    } // vt == NULL: 以前も vnode がない -> 何もしない

  }
  
  if ( enode->elnode == NULL ) {

    // 葉に対する elnode の再生成
    // elnode
    ESPELnode *elnode = create_espelnode( elist );
    // mutual links.
    elnode->enode = enode;
    enode->elnode = elnode;
      
//     display("(leaf) enode %d tree %d parent %d elnode %d re-created.\n",
// 	    enode->id, enode->tree->id,
// 	    ( enode->tree->parent != NULL ) ? enode->tree->parent->id : 0,
// 	    enode->elnode->id );
  }
}


