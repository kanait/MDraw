//
// esp.cxx
//
// exact shortest paths functions based on:
// "Shortest Paths on a Polyhedron"
// Jindong Chen and Yijie Han
// in 6th Annual Symposium on Computational Geometry (SCG'90)
//
// Copyright (c) 1997-1999 Takashi Kanai
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
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
#include "screen.h"

#include "ppdvertex.h"
#include "ppdloop.h"
#include "veclib.h"
#include "matlib.h"
#include "intsec.h"
//#include "file.h"
#include "mytime.h"

#include "esptree.h"

#include "esp.h"

#include "params.h"

// TEMP
#include "ppdface.h"
Sppd *tmp_ppd;
// TEMP

int n_esptree;
//
// Exact Shortest Path Routine
//
Splp *esp( Sppd *ppd, Spvt *src, Spvt *dist )
{
  if ( (src == NULL) || (dist == NULL) ) return NULL;

  // time start
#ifdef _WINDOWS
  DWORD start = GetTickCount();
#else  
  time_start();
#endif

  // TEMP
  tmp_ppd = ppd;
  
  // conversion from ppd to a sequence tree (we call it "esptree")
  n_esptree = 0;
  ESPTree *esptree = ppd_to_esptree( ppd, src );
  ppd->esptree = esptree;

  Splp *lp;
  double length;
  if ( dist->vnode != NULL ) {
    
    if ( params.save_path == TRUE ) {
      lp = esptree_to_esp( ppd, dist, src );
      //write_sp_file(lp, "tmp.sp", "bunnyf1000.ppd" );
    } else {
      lp = create_splp();
    }
    //Vec *vec = &(dist->vnode->unfold_vec);
    //length = V3DistanceBetween2Points( vec, &(dist->vec) );
    length = esptree_path_distance( dist->vnode->tree,
				    &(dist->vnode->unfold_vec),
				    &(dist->vec) );
    lp->length = length;
    
  } else {
    lp = NULL;
    return NULL;
  }
  
#ifdef _WINDOWS
  DWORD end = GetTickCount();
  lp->usertime = ( (double) ( end - start ) ) / 1000.0;
#else
  double realtime, usertime, systime;
  time_stop_value( &realtime, &usertime, &systime );
  lp->realtime = realtime;
  lp->usertime = usertime;
  lp->systime  = systime;
#endif  

  for ( Spvt *vt = ppd->spvt; vt != NULL; vt = vt->nxt ) {
    if ( vt->vnode != NULL ) {
      double dis = esptree_path_distance( vt->vnode->tree,
					  &(vt->vnode->unfold_vec),
					  &(vt->vec) );
//       ESPTree *ptree;
//       ESPEnode *prev_node = vt->vnode->enode;
//       Vec vec;
//       if ( prev_node != NULL ) {
// 	ptree = vt->vnode->tree;
// 	ptree = ptree->parent;
// 	unfold_imagesource( prev_node->he, &(prev_node->unfold_vec), &(vec) );
//       } else {
// 	ptree = vt->vnode->tree;
// 	Spvt *root_vt = ptree->root_vt;
// 	vec.x = root_vt->vec.x;
// 	vec.y = root_vt->vec.y;
// 	vec.z = root_vt->vec.z;
// 	ptree = ptree->parent;
//       }
      // prev_node (n')
//       double dis = esptree_path_distance( ptree, &(vec), &(vt->vec) );
      fprintf(stdout, "src: %d dist: %d distance: %g time: %g\n",
	      src->no, vt->no, dis, lp->usertime );
    } else {
      fprintf(stdout, "src: %d dist: %d distance: error. time: error.\n",
	      src->no, vt->no );
    }

  }
      
  //
  // This operation takes much time!
  //
  delete_espall( ppd );
  
  return lp;
}

// �������������X�s�[�h�d��
ESPTree *ppd_to_esptree( Sppd *ppd, Spvt *src )
{
  // number of current enodes 
  
  // storing current leaves (initialize)
  ESPEList *elist = create_espelist();
  ESPEList *cur_elist = elist;
  // create and initialize tree
  // root = S;
  // for ( i = 0; i < S �ɗאڂ���ʂ̒��́CS �̔��Α��ɂ��� e �̐�; ++i ) {
  //   root->child[i] = triple( e, S, e );
  // }

  // src �ɍŏ��� vnode �����Ă���
  ESPTree *org_tree = NULL;
  if ( (org_tree = initialize_esptree( src, 0.0, NULL, NULL, elist )) == NULL ) {
    return NULL;
  }
  (void) update_espvnode( src, org_tree, NULL, &(src->vec) );

  
  int i = 0;
  int n_cover = 0;
  int n_all = 0;
  int n_deleted_all = 0;
  ESPTree *cur_tree = org_tree;
  do {
    // current elist in next level initialize
    int n_deleted = 0;
    ESPEList *new_elist = create_espelist();

    for ( ESPELnode *elnode = cur_elist->selnode;
	  elnode != NULL; elnode = elnode->nxt ) {

      ++(n_all);

      ESPEnode *cur_node;
      if ( (cur_node = elnode->enode) == NULL ) continue;
      if ( cur_node->he->mate == (Sphe *) NULL ) continue;
      // current tree
      cur_tree = cur_node->tree;

      BOOL ca_proj, ab_proj;
      double ca_sprm, ca_eprm, ab_sprm, ab_eprm;
      Vec bar_i;
      espenode_to_proj( cur_node, &(bar_i), 
			&ca_proj, &ca_sprm, &ca_eprm,
			&ab_proj, &ab_sprm, &ab_eprm );

      // edges of a face ofc
      Sphe *bc = cur_node->he->mate; Sphe *ca = bc->nxt; Sphe *ab = ca->nxt;
      // vertices of face ofc
      Spvt *b = bc->vt; Spvt *c = ca->vt; Spvt *a = ab->vt;

//       display("cur_node %d tree %d fc %d (ed) bc %d ca %d ab %d\n (vt) a %d b %d c %d\n",
// 	      cur_node->id, cur_tree->id, cur_node->he->bpfc->no,
// 	      bc->ed->no, ca->ed->no, ab->ed->no,
// 	      a->no, b->no, c->no );
      
      //  if ( n �̉e�� A ���J�o�[���� ) {
      if ( (ca_proj == TRUE) && (ab_proj == TRUE) ) {

	++(n_cover);
	// �ȑO �pCAB ���L���Ă����m�[�h
	// previous a's node (which a halfedge ab has)
	n_deleted += esp_occupation( a, cur_tree,
				     cur_node, &(bar_i),
				     b, c, 
				     TWO_CHILDREN,
				     ca, ca_sprm, ca_eprm,
				     ab, ab_sprm, ab_eprm,
				     new_elist );
	
      } else { //  } else { // n �̉e�� A ���J�o�[���Ȃ�
	
	// e' = CA �� AB �̂�����łȂ���;
	// n->child[0] = ( e', \bar{I}, Proj_{e'}^{\bar{I}} );
	if ( ca_proj == TRUE ) {
	  
	  // a->vnode ������ (1)
	  if ( fabs( ca_eprm - 1.0 ) < SMDZEROEPS ) {
	    n_deleted += esp_occupation( a, cur_tree, cur_node,
					 &(bar_i),
					 b, c, 
					 ONE_CHILD,
					 ca, ca_sprm, ca_eprm,
					 NULL, 0.0, 0.0,
					 new_elist );
	  } else {
	    n_deleted += espenode_update_child( cur_tree, cur_node,
						ONE_CHILD,
						&(bar_i),
						ca, ca_sprm, ca_eprm,
						NULL, 0.0, 0.0,
						new_elist, TRUE );
	  }
	} else if ( ab_proj == TRUE ) {

	  // a->vnode ������ (1)
 	  if ( fabs( ab_sprm - 0.0 ) < SMDZEROEPS ) {
	    
	    n_deleted += esp_occupation( a, cur_tree, cur_node,
					 &(bar_i),
					 b, c, 
					 ONE_CHILD,
					 NULL, 0.0, 0.0,
					 ab, ab_sprm, ab_eprm,
					 new_elist );
 	  } else {
	    n_deleted += espenode_update_child( cur_tree, cur_node, ONE_CHILD,
						&(bar_i),
						NULL, 0.0, 0.0,
						ab, ab_sprm, ab_eprm,
						new_elist, TRUE );
	  }
	  
	} else { // ca_proj == ab_proj == FALSE
	}

      } // } else { // n �̉e�� A ���J�o�[���Ȃ�
      
    } // for j

    n_deleted_all += n_deleted;

    display("i %05d old %d new: %d\n", i, cur_elist->id_elnode, new_elist->id_elnode );
	    
    // delete enode0
    free_espelist( cur_elist );

    if ( !(new_elist->n_elnode) ) break;

    cur_elist = new_elist;
    
    ++i;

  } while ( i < ppd->fn );

  display("all %d deleted %d cover %d\n", n_all, n_deleted_all, n_cover ); 
 
  // delete all enodeary
  free_espelist( cur_elist );
  
  return org_tree;
}

//  I �� \bar{I} �ɍL����;
// unfold the previous image source
BOOL unfold_imagesource( Sphe *he, Vec *old_i, Vec *bar_i )
{
  if ( he->mate == NULL ) return FALSE;

  // face
  Spfc *fc = he->bpfc;
  // opposite face
  Spfc *ofc = he->mate->bpfc;

  //
  // rotate old_i according to an axis
  //

  // set axis
  // set a vector direction from he->vt to he->nxt->vt
  Vec axis;
  axis.x = he->nxt->vt->vec.x - he->vt->vec.x;
  axis.y = he->nxt->vt->vec.y - he->vt->vec.y;
  axis.z = he->nxt->vt->vec.z - he->vt->vec.z;

  // set angles between normals of two faces
  double co = V3Cosign( &(fc->nrm), &(ofc->nrm) );
  double si;
  if ( (fabs(co - 1.0) > SMDZEROEPS) && (fabs(co + 1.0) > SMDZEROEPS) ) {
    si = sqrt( 1 - co * co );
  } else {
    si = 0.0;
  }

  // decide plus or minus of si
  Vec cross;
  V3Cross( &(fc->nrm), &(ofc->nrm), &cross );
  if ( V3Cosign( &axis, &cross ) < 0.0 ) {
    si *= -1.0;
  }
  
  // make a rotation matrix 
  double m[16];
  MakeRotVec( co, si, &axis, (double *)m );
  
  // transformation
  Vec org_i;
  org_i.x = old_i->x - he->vt->vec.x;
  org_i.y = old_i->y - he->vt->vec.y;
  org_i.z = old_i->z - he->vt->vec.z;
  MultMatrixVec( (double *)m, &org_i, bar_i );
  bar_i->x += he->vt->vec.x;
  bar_i->y += he->vt->vec.y;
  bar_i->z += he->vt->vec.z;

  return TRUE;
}
  
//
// compute project parameter in 3D space
//
BOOL ProjectHalfedgeParam3D( Vec *bar_i, Vec *vb, Vec *vc, Vec *nrm,
			     double s, double e,
			     Vec *sv, Vec *ev, double *sprm, double *eprm )
{
  Vec start, end, sub;
  V3Sub( vc, vb, &sub );
  start.x = vb->x + s * sub.x;
  start.y = vb->y + s * sub.y;
  start.z = vb->z + s * sub.z;
  end.x   = vb->x + e * sub.x;
  end.y   = vb->y + e * sub.y;
  end.z   = vb->z + e * sub.z;

//   display("(bar_i) %g %g %g\n", bar_i->x, bar_i->y, bar_i->z );
//   display("(start) %g %g %g\n", start.x, start.y, start.z );
//   display("(end) %g %g %g\n", end.x, end.y, end.z );

  // �����ɂ���
  if ( isLeftSide3D( vb, vc, nrm, bar_i ) == TRUE ) return FALSE;

  // bc �� start���Cend�������S�ɏd�Ȃ����ꍇ�@-- FALSE
  if ( (isOnLine3D( bar_i, &(start), nrm, sv ) == TRUE) && 
       (isOnLine3D( bar_i, &(start), nrm, ev ) == TRUE) ) {
    return FALSE;
  }
  if ( (isOnLine3D( bar_i, &(end), nrm, sv ) == TRUE) && 
       (isOnLine3D( bar_i, &(end), nrm, ev ) == TRUE) ) {
    return FALSE;
  }

  // vec(bc) �� vec(is), vec(ie) �̂ǂ��炩�����s�Ȃ�� FALSE
//   if( isParallelLine( vb, vc, bar_i, &(start) ) == TRUE ) return FALSE;
//   if( isParallelLine( vb, vc, bar_i, &(end) ) == TRUE ) return FALSE;
  
  // i �����Ƃ��鐍�̒��� �G�b�W�������Ă��邩�ǂ����̃`�F�b�N
  BOOL ss, se, es, ee;
  ss = isLeftSide3D( bar_i, &(start), nrm, sv );
  se = isLeftSide3D( bar_i, &(start), nrm, ev );
  es = isLeftSide3D( bar_i, &(end), nrm, sv );
  ee = isLeftSide3D( bar_i, &(end), nrm, ev );

  // 1. ���[�Ƃ� start ���̍���
  if ( (ss == TRUE) && (se == TRUE) ) {
//     display("--> 1\n");
    return FALSE;
  }

  // 2. ���[�Ƃ� end ���̉E��
  if ( (es == FALSE) && (ee == FALSE) ) {
//     display("--> 2\n");
    return FALSE;
  }
  
  // 3. ���ɂ����ۂ���܂��Ă���
  if ( (ss == FALSE) && (se == FALSE) && (es == TRUE) && (ee == TRUE) ) {
    *sprm = 0;
    *eprm = 1;
//     display("--> 3\n");
    return TRUE;
  }

  //
  // �ǂ��炩�̐��������C�������͗��������� 6 �ʂ�̏ꍇ����
  //
  
  double dum, p1, p2;
  // start ���݂̂�����
  // 4. sv �� ���Cev ���O
  if ( (ss == FALSE) && (se == TRUE) && (es == TRUE) && (ee == TRUE) ) {
//     display("--> 4\n");
    (void) IntsecLinesParam3D( bar_i, &(start), sv, ev, &dum, &p1 );
    *sprm = 0.0;
    *eprm = p1;
    
    // ������Ă��Ȃ�
    if ( fabs( *eprm - 0.0 ) < SMDZEROEPS ) {
      return FALSE;
    }
    
    return TRUE;
  }

  // 5. ev �� ���Csv ���O
  if ( (ss == TRUE) && (se == FALSE) && (es == TRUE) && (ee == TRUE) ) {
//     display("--> 5\n");
    (void) IntsecLinesParam3D( bar_i, &(start), sv, ev, &dum, &p1 );
    *sprm = p1;
    *eprm = 1.0;
    // ������Ă��Ȃ�
    if ( fabs( *sprm - 1.0 ) < SMDZEROEPS ) {
      return FALSE;
    }
    return TRUE;
  }
  
  // end ���݂̂�����
  // 6. ev �� ���Csv ���O
  if ( (ee == TRUE) && (es == FALSE) && (ss == FALSE) && (se == FALSE) ) {
//     display("--> 6\n");
    (void) IntsecLinesParam3D( bar_i, &(end), sv, ev, &dum, &p1 );
    *sprm = p1;
    *eprm = 1.0;
    if ( fabs( *sprm - 1.0 ) < SMDZEROEPS ) {
      return FALSE;
    }
    return TRUE;
  }
  
  // 7. sv �� ���Cev ���O
  if ( (es == TRUE) && (ee == FALSE) && (ss == FALSE) && (se == FALSE) ) {
//     display("--> 7\n");
    (void) IntsecLinesParam3D( bar_i, &(end), sv, ev, &dum, &p1 );
    *sprm = 0.0;
    *eprm = p1;
    if ( fabs( *eprm - 0.0 ) < SMDZEROEPS ) {
      return FALSE;
    }
    return TRUE;
  }

  // ��������
  // 8. ev �� start ���̊O�Csv �� end ���̊O
  if ( (se == TRUE) && (es == FALSE) ) {
//     display("--> 8\n");
    (void) IntsecLinesParam3D( bar_i, &(start), sv, ev, &dum, &p1 );
    (void) IntsecLinesParam3D( bar_i, &(end), sv, ev, &dum, &p2 );
    *sprm = p2;
    *eprm = p1;
    if ( fabs( *eprm - *sprm ) < SMDZEROEPS ) {
      return FALSE;
    }
    return TRUE;
  }

  // 9. sv �� start ���̊O�Cev �� end ���̊O
  if ( (ss == TRUE) && (ee == FALSE) ) {
//     display("--> 9\n");
    (void) IntsecLinesParam3D( bar_i, &(start), sv, ev, &dum, &p1 );
    (void) IntsecLinesParam3D( bar_i, &(end), sv, ev, &dum, &p2 );
    *sprm = p1;
    *eprm = p2;
    if ( fabs( *eprm - *sprm ) < SMDZEROEPS ) {
      return FALSE;
    }
    return TRUE;
  }
  
  return FALSE;
}

int esp_occupation( Spvt *vt, ESPTree *tree, ESPEnode *cur_node, Vec *bar_i,
		    Spvt *b, Spvt *c, 
		    int n_child,
		    Sphe *he0, double sprm0, double eprm0,
		    Sphe *he1, double sprm1, double eprm1,
		    ESPEList *elist )
{
  if ( vt == NULL ) return 0;

  int n_deleted = 0;
  
  if ( vt->vnode == NULL ) {
//     display("(esp_update_occupation) 1111\n");
    n_deleted += esp_update_occupation( vt, vt->vnode, tree, cur_node,
					bar_i, n_child,
					he0, sprm0, eprm0, he1, sprm1, eprm1,
					elist );
  } else {

    ESPEnode *prev_node = vt->vnode->enode;
    Vec prev_bar_i;
    ESPTree *ptree = NULL;
    if ( prev_node != NULL ) {
      // prev_node �� bar_i = ���̎q���� unfold_vec
      // prev_node �̎q���͓����(�͂�)
      ptree = prev_node->tree;
      unfold_imagesource( prev_node->he, &(prev_node->unfold_vec),
			  &(prev_bar_i) );
    } else {
      // vt->vnode->enode == NULL
      // -> a �� vt->vnode->tree->root_vt �� 1-ring �ߖT
      ptree = vt->vnode->tree;
      Spvt *root_vt = ptree->root_vt;
      prev_bar_i.x = root_vt->vec.x;
      prev_bar_i.y = root_vt->vec.y;
      prev_bar_i.z = root_vt->vec.z;
      //display("tree %d\n", ptree->id );
      //ptree = ptree->parent;
//       if ( ptree != NULL )
// 	display("tree %d\n", ptree->id );
      
    }
  
    //
    // prev_node �� cur_node �Ƃ̔�r
    //
    
    // cur_node (n)
    double dis0 = tmp_path_distance( tree, bar_i, &(vt->vec) );
//     display("new path:\n");
//     tmp_path_route( tree, cur_node, bar_i, &(vt->vec) );
    
    // prev_node (n')
    double dis1 = esptree_path_distance( ptree, &(prev_bar_i), &(vt->vec) );
//     display("old path:\n");
//     print_esp_path_route( vt );
    
//     display("(new) dis0 %g (old) dis1 %g\n", dis0, dis1 );
    
    if ( dis0 < dis1 ) { // if ( n �� n' �ɑ΂��pCAB ���L���� ) 

      // n�f�̎q���̂����C�ŒZ�o�H���`���邱�Ƃ̕s�\��
      // ���؂���C���̎q�������Ƃ��镔���؂��폜����
      // delete one of prev_node's children

//       display("cutoff begin.\n");
      n_deleted += cutoff_enode( prev_node );
//       display("cutoff end.\n");
	    
//       display("(esp_update_occupation) 2222\n");
      n_deleted += esp_update_occupation( vt, vt->vnode, tree, cur_node,
					  bar_i, n_child,
					  he0, sprm0, eprm0,
					  he1, sprm1, eprm1,
					  elist );

    } else {  // n �� n' �ɑ΂��pCAB ���L���Ȃ� 

      if ( n_child == TWO_CHILDREN ) {
	// e' = CA �� AB �̂����C�ŒZ�o�H���`���邱�Ƃ̉\�Ȉ��;
	Vec vac, vab, vai;
	V3Sub( &(c->vec), &(vt->vec), &vac );
	V3Sub( &(b->vec), &(vt->vec), &vab );
	V3Sub( bar_i, &(vt->vec), &vai );
	double lcos = fabs( V3Cosign( &vac, &vai ) );
	double rcos = fabs( V3Cosign( &vab, &vai ) );
	// Proj_{e\'}^{\bar{I}} ���v�Z
        // n->child[0] = { e\', \bar{I}, Proj_{e\'}^{\bar{I}} }
        if ( lcos > rcos ) {
	  n_deleted += espenode_update_child( tree, cur_node, ONE_CHILD,
					      bar_i,
					      he0, sprm0, eprm0,
					      NULL, 0.0, 0.0,
					      elist, FALSE );
	} else {
	  n_deleted += espenode_update_child( tree, cur_node, ONE_CHILD,
					      bar_i,
					      NULL, 0.0, 0.0,
					      he1, sprm1, eprm1,
					      elist, FALSE );
	}
	
      } else {

	if ( he0 != NULL ) {
	  n_deleted += espenode_update_child( tree, cur_node, ONE_CHILD,
					      bar_i,
					      he0, sprm0, eprm0,
					      NULL, 0.0, 0.0,
					      elist, TRUE );
	}
	if ( he1 != NULL ) {
	  n_deleted += espenode_update_child( tree, cur_node, ONE_CHILD,
					      bar_i,
					      NULL, 0.0, 0.0,
					      he1, sprm1, eprm1,
					      elist, TRUE );
	}
	  
      }
    
    }
  }

  return n_deleted;
}

int esp_update_occupation( Spvt *vt,
			   ESPVnode *vnode, ESPTree *tree, ESPEnode *cur_node,
			   Vec *bar_i,
			   int n_child,
			   Sphe *he0, double sprm0, double eprm0,
			   Sphe *he1, double sprm1, double eprm1,
			   ESPEList *elist )
{
  //if ( vnode == NULL ) return 0;
  int n_deleted = 0;
			   
  // enode->child ��ǉ�
  // n->child[0] = (CA, \bar{I}, Proj_{CA}^{\bar{I}});
  // (n->child[1] = (AB, \bar{I}, Proj_{AB}^{\bar{I}});)
  //display("ccc\n");
  n_deleted += espenode_update_child( tree, cur_node, n_child,
				      bar_i,
				      he0, sprm0, eprm0, he1, sprm1, eprm1,
				      elist, FALSE );

//   if ( (n_child == ONE_CHILD) && (params.save_path != TRUE) ) {
//     return n_deleted;
//   }

  // a->vnode ������ (1)
  // display("(update_espvnode) cccc\n");
  (void) update_espvnode( vt, tree, cur_node, bar_i );
  
  //
  // a �� root �Ƃ��镛�؂𐶐� (��ʌ`��p)
  update_esptree( vt, cur_node, tree, V3DistanceBetween2Points(bar_i, &(vt->vec)), elist );
  
  return n_deleted;
}

Splp *esptree_to_esp( Sppd *ppd, Spvt *dist, Spvt *src )
{
  // can't find a shortest path
  // if the vnode of the dist vertex is not found
  if (dist->vnode == NULL ) return NULL;
  
  // create a open loop
  Splp* lp = create_splp();

  // start vertex: dist
  Splv *lv = create_ppdloopvertex( lp );
  lv->vt = dist;

  Spvt *vt = dist;
  ESPTree *tree = dist->vnode->tree;

  while ( tree != NULL ) {

//     display("tree %d\n", tree->id );
    esptree_path( tree, vt, lp, ppd );
    
    vt = tree->root_vt;
    tree = tree->parent;

  }

  return lp;
}

void esptree_path( ESPTree *tree, Spvt *vt, Splp *lp, Sppd *ppd )
  // tree: ���߂�o�H
  // vt: �o�H�̎n�_
{
  ESPVnode *vnode = vt->vnode;
  if ( vnode == NULL ) return;
  
  // begin enode
  ESPEnode *enode = vnode->enode;
  // for calculating intersection point
  Vec *vec = &(vnode->unfold_vec);

  // newvt �����_�̏ꍇ�͕ʏ���
  Splv *lv;
  while ( enode != NULL ) {

    //
    // intersection point
    //
    Sphe *he = enode->he;
    Spvt *sv = he->vt;
    Spvt *ev = he->nxt->vt;

    // s: �o�H�x�N�g���̃p�����[�^ t: �n�[�t�G�b�W�̃p�����[�^
    double s, t;
    (void) IntsecLinesParam3D( &(vt->vec), vec, &(sv->vec), &(ev->vec),
			       &s, &t );

    Vec newvec;
    newvec.x = sv->vec.x + t * ( ev->vec.x - sv->vec.x );
    newvec.y = sv->vec.y + t * ( ev->vec.y - sv->vec.y );
    newvec.z = sv->vec.z + t * ( ev->vec.z - sv->vec.z );
    // create new vertex, loopvertex
    Spvt *newvt = create_ppdvertex( ppd );
    lv = create_ppdloopvertex( lp );
    lv->vt = newvt;
    newvt->type = VERTEX_ADDED;
    newvt->vec.x = newvec.x;
    newvt->vec.y = newvec.y;
    newvt->vec.z = newvec.z;

//     display("(esptree_to_esp) ed %d enode %d tree %d n_child %d sprm %g eprm %g\n",
// 	    he->ed->no, enode->id, tree->id,
// 	    enode->n_child, enode->sproj, enode->eproj  );
//     display("\tnewvt %d x %g y %g z %g param %g\n",
// 	    newvt->no, newvt->vec.x, newvt->vec.y, newvt->vec.z, t );
    
    // next step
    vec = &(enode->unfold_vec);
    enode = enode->parent;
    vt = newvt;
  }

  // end vertex: src
  lv = create_ppdloopvertex( lp );
  lv->vt = tree->root_vt;
}  



