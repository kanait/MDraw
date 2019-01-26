//
// spedprique.cxx
// Priority queue functions for ppd edge
//
// Copyright (c) 2000 Takashi Kanai
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

#include "spedprique.h"

SpedPQHeap *create_spedpqheap( int size )
{
  SpedPQHeap *pqh = (SpedPQHeap *) malloc( sizeof(SpedPQHeap) );
  pqh->pqcont = (SpedPQCont *) malloc( size * sizeof(SpedPQCont) );
  
  for ( int i = 0; i < size; ++i ) pqh->pqcont[i].id = i;
  pqh->size = size;
  pqh->last = 0;

  return pqh;
}

void swap_spedpqcont( SpedPQCont *a, SpedPQCont *b )
{
  // 評価値
  double temp_length = a->length;
  a->length   = b->length;
  b->length   = temp_length;

  // 移動する頂点
  Spvt *temp_vt = a->vt;
  a->vt = b->vt;
  b->vt = temp_vt;

  // 交差するエッジ
  Sped *temp_ed = a->ed;
  a->ed = b->ed;
  b->ed = temp_ed;

  // ed と交差する頂点
  Spvt *tmp_ivt = a->ivt;
  a->ivt = b->ivt;
  b->ivt = tmp_ivt;
  
  // 移動する終点のベクトル
  double evecx = a->evec.x;
  a->evec.x = b->evec.x;
  b->evec.x = evecx;
  double evecy = a->evec.y;
  a->evec.y = b->evec.y;
  b->evec.y = evecy;
  
  a->vt->pqc = a;
  b->vt->pqc = b;
}  

SpedPQCont *insert_spedpqcont( SpedPQHeap *pqh, double length, Spvt *vt )
{
  if ( pqh->last >= pqh->size ) return NULL;
  
  pqh->pqcont[pqh->last].length = length;
  pqh->pqcont[pqh->last].vt     = vt;

  vt->pqc = &(pqh->pqcont[pqh->last]);
  vt->pq_type = EXIST;
  
  (void) spedpqcont_adjust_to_parent( pqh->last, pqh );

  ++(pqh->last);
  
  return vt->pqc;
}

// parent direction process
void spedpqcont_to_root( int id, SpedPQHeap *pqh )
{
  int c = id;
  while ( TRUE ) {
    if ( !c ) break;
    int p = parent_num( c );
    swap_spedpqcont( &(pqh->pqcont[c]), &(pqh->pqcont[p]) );
    c = p;
  }
}

int spedpqcont_adjust_to_parent( int id, SpedPQHeap *pqh )
{
  int c = id;
  while ( c > 0 ) {
    int p = parent_num( c );
    if ( pqh->pqcont[p].length > pqh->pqcont[c].length ) {
      swap_spedpqcont( &(pqh->pqcont[c]), &(pqh->pqcont[p]) );
      c = p;
    } else {
      return c;
    }
  }
  return c;
}

// child direction process
int spedpqcont_adjust_to_child( int id, SpedPQHeap *pqh )
{
  int p = id;
  int c, i2l, i2r;
  while ( p <= parent_num(pqh->last-1) ) {

    // child: i2l, i2r
    i2l = 2*p+1; i2r = i2l+1;
    if ( i2r == pqh->last ) { // pqh->last does not exist.
      c = i2l;
    } else if ( pqh->pqcont[i2l].length < pqh->pqcont[i2r].length ) {
      c = i2l;
    } else {
      c = i2r;
    }
    
    if ( pqh->pqcont[p].length > pqh->pqcont[c].length ) {
      swap_spedpqcont( &(pqh->pqcont[p]), &(pqh->pqcont[c]) );
    } else {
      return p;
    }
    p = c;
  }
  return p;
}

//
// 現在 id に入っている pqc を調節する
//
int adjust_spedpqcont( Id id, SpedPQHeap *pqh )
{
  // update values
  //Spvt *vt = pqh->pqcont[id].vt;
  //pqh->pqcont[id].length = vt->length;
  
  // update heap trees
//    display("id %d\n", id );
  int pid = spedpqcont_adjust_to_parent( id, pqh );
//    display("pid %d\n", pid );
  int cid = spedpqcont_adjust_to_child( pid, pqh );
//    display("cid %d\n", cid );

  return cid;
}

void deletemin_spedpqcont( SpedPQHeap *pqh )
{
  // empty
  if ( pqh->last == 0 ) return;
  
  // delete mininum (root) 
  pqh->pqcont[0].vt->pq_type = DELETED;
  pqh->pqcont[0].vt->pqc     = NULL;

  pqh->pqcont[0].length = pqh->pqcont[pqh->last-1].length;
  pqh->pqcont[0].vt     = pqh->pqcont[pqh->last-1].vt;
  pqh->pqcont[0].ed     = pqh->pqcont[pqh->last-1].ed;
  pqh->pqcont[0].evec.x = pqh->pqcont[pqh->last-1].evec.x;
  pqh->pqcont[0].evec.y = pqh->pqcont[pqh->last-1].evec.y;

  pqh->pqcont[0].vt->pqc = &(pqh->pqcont[0]);

  --(pqh->last);

  int i = 0;
  (void) spedpqcont_adjust_to_child( i, pqh );
}

void delete_spedpqcont( Id id, SpedPQHeap *pqh )
{
  spedpqcont_to_root( id, pqh );
  deletemin_spedpqcont( pqh );
}

void free_spedpqheap( SpedPQHeap *pqh )
{
  free( pqh->pqcont );
  free( pqh );
}

int parent_num( int child )
{
  return (int) (child - 1) / 2;
}


