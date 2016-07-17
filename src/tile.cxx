//
// tile.cxx
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
#include "ppd.h"
#include "ppdvertex.h"
#include "ppdedge.h"
#include "ppdface.h"
#include "ppdloop.h"

#include "hestack.h"
#include "tile.h"

Tile *create_tile()
{
  Tile *tile;
  
  if ((tile = (Tile *) malloc(sizeof(Tile)) ) == (Tile *) NULL) {
    return (Tile *) NULL;
  }

  // tvertex
  tile->n_tvertex = 0;
  tile->n_tvtid = 0;
  tile->stvt = NULL;
  tile->etvt = NULL;

  // tedge
  tile->n_tedge = 0;
  tile->n_tedid = 0;
  tile->sted = NULL;
  tile->eted = NULL;

  // tface
  tile->n_tface = 0;
  tile->n_tfcid = 0;
  tile->stfc = NULL;
  tile->etfc = NULL;

  // ppd
  tile->ppd = NULL;

  // filename
  strcpy( tile->filename, "" );
  
  return tile;
}

void free_tile( Tile *tile )
{
  TVertex *tvt, *ntvt;
  TEdge *ted, *nted;
  TFace *tfc, *ntfc;

  if (  tile == NULL ) return;

  // edge
  for ( ted = tile->sted; ted != NULL; ted = nted ) {
    nted = ted->nxt;
    free_tedge( ted, tile );
  }
  tile->sted = tile->eted = NULL;

  // face
  for ( tfc = tile->stfc; tfc != NULL; tfc = ntfc ) {
    ntfc = tfc->nxt;
    free_tface( tfc, tile );
  }
  tile->stfc = tile->etfc = NULL;

  // vertex
  for ( tvt = tile->stvt; tvt != NULL; tvt = ntvt ) {
    ntvt = tvt->nxt;
    free_tvertex( tvt, tile );
  }
  tile->stvt = tile->etvt = NULL;

  // tile
  free( tile );
}

//
// Tile vertex functions
//
TVertex *create_tvertex( Tile *tile )
{
  TVertex *tvt;

  tvt = (TVertex *) malloc(sizeof(TVertex));

  tvt->nxt = (TVertex *) NULL;
  if (tile->stvt == (TVertex *) NULL) {
    tvt->prv   = (TVertex *) NULL;
    tile->etvt = tile->stvt = tvt;
  } else {
    tvt->prv   = tile->etvt;
    tile->etvt = tile->etvt->nxt = tvt;
  }

  tvt->no = tile->n_tvtid;
  tvt->sid = SMDNULL;
  
  // ppdvertex
  tvt->vt = NULL;
  
  ++( tile->n_tvertex );
  ++( tile->n_tvtid );

  return tvt;
}

TVertex *list_tvertex( Tile *tile, Id id )
{
  TVertex *tvt;
  
  for ( tvt = tile->stvt; tvt != (TVertex *) NULL; tvt = tvt->nxt ) {
    if ( tvt->no == id ) return tvt;
  }
  return (TVertex *) NULL;
}

TVertex *find_tvertex( Tile *tile, Spvt *vt )
{
  TVertex *tvt;
  
  for ( tvt = tile->stvt; tvt != (TVertex *) NULL; tvt = tvt->nxt ) {
    if ( tvt->vt == vt ) return tvt;
  }
  return (TVertex *) NULL;
}

void free_tvertex( TVertex *tvt, Tile *tile )
{
  if (tvt == (TVertex *) NULL) return;
  
  if ( tile->stvt == tvt ) {
    if ( (tile->stvt = tvt->nxt) != (TVertex *) NULL )
      tvt->nxt->prv = (TVertex *) NULL;
    else {
      tile->etvt = (TVertex *) NULL;
    }
  } else if ( tile->etvt == tvt ) {
    tvt->prv->nxt = (TVertex *) NULL;
    tile->etvt = tvt->prv;
  } else {
    tvt->prv->nxt = tvt->nxt;
    tvt->nxt->prv = tvt->prv;
  }

  --( tile->n_tvertex );
  
  free( tvt );
}

//
// Tile edge functions
//
TEdge *create_tedge( Tile *tile )
{
  TEdge *ted;

  ted = (TEdge *) malloc( sizeof(TEdge) );

  ted->nxt = (TEdge *) NULL;
  if (tile->sted == (TEdge *) NULL) {
    ted->prv   = (TEdge *) NULL;
    tile->eted = tile->sted = ted;
  } else {
    ted->prv   = tile->eted;
    tile->eted = tile->eted->nxt = ted;
  }

  ted->no = tile->n_tedid;
  ted->sid = SMDNULL;
  ted->sv = NULL;
  ted->ev = NULL;
  
  // ppd open loop 
  ted->lp = NULL;
  
  ++( tile->n_tedge );
  ++( tile->n_tedid );

  return ted;
}

void free_tedge( TEdge *ted, Tile *tile )
{
  if (ted == (TEdge *) NULL) return;
  
  if ( tile->sted == ted ) {
    if ( (tile->sted = ted->nxt) != (TEdge *) NULL )
      ted->nxt->prv = (TEdge *) NULL;
    else {
      tile->eted = (TEdge *) NULL;
    }
  } else if ( tile->eted == ted ) {
    ted->prv->nxt = (TEdge *) NULL;
    tile->eted = ted->prv;
  } else {
    ted->prv->nxt = ted->nxt;
    ted->nxt->prv = ted->prv;
  }

  --( tile->n_tedge );
  
  free( ted );
}

TEdge *list_tedge( Tile *tile, Id id )
{
  TEdge *ted;
  
  for ( ted = tile->sted; ted != NULL; ted = ted->nxt ) {
    if ( ted->no == id ) return ted;
  }
  return NULL;
}

BOOL isusedtedge( TEdge *ted, Tile *tile )
{
  for ( TFace *tfc = tile->stfc; tfc != NULL; tfc = tfc->nxt )
    {
      for ( int i = 0; i < tfc->n_tedge; ++i )
	{
	  if ( tfc->ed[i] == ted ) return TRUE;
	}
    }
  return FALSE;
}

BOOL istedge_in_tface( TEdge* ted, TFace* tfc )
{
  for ( int i = 0; i < tfc->n_tedge; ++i )
    {
      if ( tfc->ed[i] == ted ) return TRUE;
    }
  return FALSE;
}
  
void delete_tedge( TEdge *ted, Tile *tile )
{
  Splp* lp = ted->lp;
  Sppd* ppd = tile->ppd;

  // tedge �� lp ������
  delete_tedge_lp( lp, ppd );
  
  free_tedge( ted, tile );
  if ( !tile->n_tedge ) {
    tile->sted = tile->eted = NULL;
  }
}

// tedge �� lp ������
void delete_tedge_lp( Splp* lp, Sppd* ppd )
{
  // free loop vertex and loop
  Splv *nlv = NULL;
  for ( Splv *lv = lp->splv; lv != NULL; lv = nlv ) {

    nlv = lv->nxt;

    Spvt *vt = lv->vt;
    assert( vt != NULL );

    // lv �� nlv �̊Ԃ̃G�b�W�̏���
    if ( nlv != NULL ) {
      Spvt *nvt = nlv->vt;
      assert( nvt != NULL );

      Sped *ed = find_ppdedge( vt, nvt );
      assert( ed != NULL );
      if ( ed->type == EDGE_ONFACE ) {
	ed->sp_fc = NULL;
	// �o�H�̂��߂ɒǉ����ꂽ�G�b�W�Ȃ̂ŏ���
	free_isolated_ppdedge( ed, ppd );
      }
    }
      
    --( vt->spn );
    if ( !(vt->spn) ) lv->vt->sp_type = SP_VERTEX_NORMAL;
    if ( vt->type == VERTEX_ADDED ) {
      vt->sp_ed = NULL;
      // �o�H�̂��߂ɒǉ����ꂽ���_�Ȃ̂ŏ���
      free_ppdvertex( vt, ppd );
    }
    lv->vt = NULL;

    free_ppdloopvertex( lv, lp );
  }
  free( lp );
}
  
//
// Tile face functions
//
TFace *create_tface( Tile *tile )
{
  TFace *tfc;

  tfc = (TFace *) malloc( sizeof(TFace) );

  tfc->nxt = (TFace *) NULL;
  if ( tile->stfc == (TFace *) NULL ) {
    tfc->prv   = (TFace *) NULL;
    tile->etfc = tile->stfc = tfc;
  } else {
    tfc->prv   = tile->etfc;
    tile->etfc = tile->etfc->nxt = tfc;
  }

  tfc->no = tile->n_tfcid;
  tfc->sid = SMDNULL;
  tfc->col = FACEBLUE;

  tfc->n_tedge = 0;
  tfc->ed = NULL;

  // parameterization flag
  tfc->isParameterized = FALSE;
  
  // ppd part
  tfc->pt = NULL;
  
  // ppd closed loop
  tfc->clp = NULL;

  // tvector
  tfc->n_tvector = 0;
  tfc->tvec = NULL;

  // pattern type
  tfc->tptype = TILE_PATTERN_NONE;

  // number of subdivision
  tfc->ndiv = 10;

  // center vertex
  tfc->cv = NULL;
  
  // remesh ppd
  tfc->rppd = NULL;

  // cylindrical face
  tfc->num_edges_per_loop = 0;
  tfc->cyl_lp[0] = NULL;
  tfc->cyl_lp[1] = NULL;
  
  ++( tile->n_tface );
  ++( tile->n_tfcid );

  return tfc;
}

void free_tface( TFace *tfc, Tile *tile )
{
  if (tfc == (TFace *) NULL) return;

  // loop
  if ( tfc->clp != NULL ) free_splp( tfc->clp );
  if ( tfc->cyl_lp[0] != NULL )
    {
      free_splp( tfc->cyl_lp[0] );
      free_splp( tfc->cyl_lp[1] );
    }
  
  // remesh ppd
  if ( tfc->rppd != NULL ) {
    free_ppd( tfc->rppd );
    tfc->rppd = NULL;
  }

  // free tvector
  if ( tfc->tvec != NULL ) {
    for ( int i = 0; i < tfc->n_tvector; ++i ) {
      if ( tfc->tvec[i].lambda != NULL ) free(tfc->tvec[i].lambda);
      if ( tfc->tvec[i].vec2d != NULL ) free(tfc->tvec[i].vec2d);
    }
    free( tfc->tvec );
    tfc->tvec = NULL;
  }
  
  if ( tile->stfc == tfc ) {
    if ( (tile->stfc = tfc->nxt) != (TFace *) NULL )
      tfc->nxt->prv = (TFace *) NULL;
    else {
      tile->etfc = (TFace *) NULL;
    }
  } else if ( tile->etfc == tfc ) {
    tfc->prv->nxt = (TFace *) NULL;
    tile->etfc = tfc->prv;
  } else {
    tfc->prv->nxt = tfc->nxt;
    tfc->nxt->prv = tfc->prv;
  }

  --( tile->n_tface );
  
  free( tfc );
}

TFace *find_tface( Tile *tile, int id )
{
  for ( TFace *tf = tile->stfc; tf != NULL; tf = tf->nxt ) {
    if ( tf->no == id ) return tf;
  }
  return NULL;
}

void initialize_tvector( TVector *tvec )
{
  if ( tvec == NULL ) return;
  
  tvec->vt = NULL;
  tvec->n_degree = 0;
  tvec->vec2d = NULL;
  tvec->lambda = NULL;
  //tvec->isBoundary = FALSE;
}

//
// clp �̒ʂ��Ă��� ppd face �̕��� (for create_tile_face)
// 
void tface_subdivide_ppdface( TFace* tfc, Tile* tile, Sppd *ppd )
{
//  #if defined(_DEBUG)
//    Spvt* tmp_vt = NULL;
//  #endif

//    display("check 1 ppd->en %d\n", ppd->en );
  // ONFACE �� loop edge ���� subdiv face �𒊏o
  SubdivFace *sfcall = (SubdivFace *) malloc( tfc->clp->lvn * sizeof(SubdivFace) );
  SubdivFace *sfc;
  int n_sfc = 0;
  int n_sed = 0;
  
  // VERTEX_ADDED �� loop vertex ���� subdiv edge �𒊏o
  SubdivEdge *sedall = (SubdivEdge *) malloc( tfc->clp->lvn * sizeof(SubdivEdge) );
  SubdivEdge *sed;
  
  Spvt *vt = NULL;
  
//  #if defined(_DEBUG)
//  tmp_vt = list_ppdvertex( ppd, 10323 );
//  display("---->1 vt %d neighbor edges %d\n", tmp_vt->no, ppdvertex_num_edge( tmp_vt ) );
//  #endif
    
  Splv *lv = NULL;
  Sped *ed = NULL;
  for ( lv = tfc->clp->splv; lv != NULL; lv = lv->nxt ) {

    Splv *slv = lv;
    Splv *elv = ( lv != tfc->clp->eplv ) ? lv->nxt : tfc->clp->splv;
    ed = find_ppdedge( slv->vt, elv->vt );
    assert( ed != NULL );

    // ���[�v�G�b�W�̕����ɃG�b�W�𐮗�
    order_ppdedge( ed, slv->vt, elv->vt );
    
    //
    // SubdivFace �ւ̒l�̊i�[
    // sfc �� �������ׂ� loop edge ���i�[����
    //
    if ( ed->type == EDGE_ONFACE )
      {
	if ( (sfc = find_subdivface( n_sfc, sfcall, ed->sp_fc )) == NULL )
	  {
	    // �V���� subdivface �̏�����
	    sfc = &(sfcall[n_sfc]);
	    init_subdivface( sfc );
	    ++n_sfc;
	  }
	// �i�[
	sfc->fc = ed->sp_fc;
	sfc->ed[sfc->en] = ed;
	++(sfc->en);
      }
    //
    // SubdivEdge �ւ̒l�̊i�[
    // sed �� �������ׂ� loop vertex ���i�[����
    //
    vt = lv->vt;
    if ( vt->type == VERTEX_ADDED )
      {
	if ( (sed = find_subdivedge( n_sed, sedall, vt->sp_ed )) == NULL )
	  {
	    // �V���� subdivedge �̏�����
	    sed = &(sedall[n_sed]);
	    init_subdivedge( sed );
	    ++n_sed;
	  }
	// �i�[
	sed->ed = vt->sp_ed;
	sed->vt[sed->vn] = vt;
	++(sed->vn);
      }
    //
    //
  }

//  #if defined(_DEBUG)
//    display("---->2 vt %d neighbor edges %d\n", tmp_vt->no, ppdvertex_num_edge( tmp_vt ) );
//  #endif
  
  int i;
//    display("check 2\n");
//    display("sfc %d\n", n_sfc );
//    for ( i = 0; i < n_sfc; ++i ) {
//      display("fc %d en %d\n", sfcall[i].fc->no, sfcall[i].en );
//    }
//    display("sed %d\n", n_sed );
//    for ( i = 0; i < n_sed; ++i ) {
//      display("ed %d vt %d\n", sedall[i].ed->no, sedall[i].vn );
//    }
  
  // �V�������������ʁC�G�b�W���i�[���邽�߂̕ϐ�
  // - ��̖ʂɊ܂܂�镪���G�b�W�̐��͂�������2��
  // - ��̃G�b�W�Ɋ܂܂�镪�����_�̐��͂�������2��
  // �ł��邩��C��̖ʁC�G�b�W�ɂ����ꂼ��V�������������ʁC�G�b�W��
  // ���ꂼ�ꂹ������ 3 ��
  int n_newed = 0;
  int n_newfc = 0;
  SubdivFace *newfcall = (SubdivFace *) malloc( 3 * tfc->clp->lvn * sizeof(SubdivFace) );
  SubdivEdge *newedall = (SubdivEdge *) malloc( 3 * tfc->clp->lvn * sizeof(SubdivEdge) );
  
  //
  // �o�H���ʂ�ʁC�G�b�W���ɕ����ʁC�����G�b�W�̐����Ɗi�[
  // (�����G�b�W�������ō쐬�C�i�[)
  //
  for ( i = 0; i < n_sfc; ++i )
    {
      subdivide_ppdface( &(sfcall[i]), ppd, &(n_newfc), newfcall, &(n_newed), newedall );
    }

//    display("n_newfc %d n_newed %d\n", n_newfc, n_newed );
  
//  #if defined(_DEBUG)
//    display("---->3 vt %d neighbor edges %d\n", tmp_vt->no, ppdvertex_num_edge( tmp_vt ) );
//  #endif

  //
  // ���̋��E�� (tfc->clp �łȂ�����) �ɂ��Ă���
  // - vt->sp_ed (if vt->type = VERTEX_ADDED)
  // - ed->sp_fc (if ed->type = EDGE_ONFACE)
  // ���C�V���������������̂ɒu��������͖̂ʓ|�Ȃ̂ŁC
  // �ŒZ�o�H���Čv�Z����
  for ( TEdge* ted = tile->sted; ted != NULL; ted = ted->nxt )
    {
      // �Čv�Z�����邩�ǂ����̏�����
      ted->isRecalc = FALSE;
      
      // ted �� tfc �Ɋ܂܂��ꍇ�͏������Ȃ�
      if ( istedge_in_tface( ted, tfc ) == TRUE ) continue; 
      
      for ( Splv* alv = ted->lp->splv; alv != NULL; alv = alv->nxt )
	{
	  //
	  // sp_ed �� sedall �Ɋ܂܂�Ă��Ȃ������`�F�b�N
	  //
	  Spvt* avt = alv->vt;
	  if ( avt->type == VERTEX_ADDED )
	    {
	      if ( (sed = find_subdivedge( n_sed, sedall, avt->sp_ed )) != NULL )
		{
//  		  display("sp_ed found. vt %d sp_ed %d\n", avt->no, avt->sp_ed->no);
		  ted->isRecalc = TRUE;
		  // �ꉞ�����N�������Ă���
		  avt->sp_ed = NULL;
		}
	    }
	  //
	  //
	  //
	  if ( alv->nxt == NULL ) continue;
	  //
	  // sp_fc �� sfcall �Ɋ܂܂�Ă��Ȃ������`�F�b�N
	  //
	  Sped* aed = find_ppdedge( alv->vt, alv->nxt->vt );
	  if ( aed->type == EDGE_ONFACE )
	    {
	      if ( (sfc = find_subdivface( n_sfc, sfcall, aed->sp_fc )) != NULL )
		{
//  		  display("sp_fc found. ed %d sp_fc %d\n", aed->no, aed->sp_fc->no );
		  ted->isRecalc = TRUE;
		  // �ꉞ�����N�������Ă���
		  aed->sp_fc = NULL;
		}
	    }
	  //
	  //
	  //
	}
//        display("tedge no.%d recalc %d\n", ted->no, ted->isRecalc );
    }
	  
  
  //
  // ���E�̐ؒf���ꂽ�Ǘ��G�b�W�̏����� vt->type, ed->type �̍X�V
  //
  ed = NULL;
  for ( lv = tfc->clp->splv; lv != NULL; lv = lv->nxt ) {

    //display("lv %d vt %d\n", lv->no, lv->vt->no );

    if ( lv->vt->type == VERTEX_ADDED ) {

      // �Ǘ��G�b�W�̏���
      ed = lv->vt->sp_ed;
      // ���� ed �������_�̃����N��؂��Ă���
      for ( Splv *clv = tfc->clp->splv; clv != NULL; clv = clv->nxt ) {
	if ( ed == clv->vt->sp_ed ) {
	  clv->vt->sp_ed = NULL;
	}
      }
      free_isolated_ppdedge( ed, ppd );

      // vt->type �̍X�V
      lv->vt->type = VERTEX_ORIGINAL;
    }
    
    // ed->type �̍X�V
    Splv *nlv = ( lv != tfc->clp->eplv ) ? lv->nxt : tfc->clp->splv;
    ed = find_ppdedge( lv->vt, nlv->vt );
    assert( ed != NULL );
    if ( ed == NULL ) continue;
    if ( ed->type == EDGE_ONFACE ) {
      ed->sp_fc = NULL;
      ed->type = EDGE_ORIGINAL;
    }

//  #if defined(_DEBUG)
//      display("---->4 vt %d neighbor edges %d\n", tmp_vt->no, ppdvertex_num_edge( tmp_vt ) );
//  #endif
    
  }

//    display("check 4\n");
  
  // �Ǘ��ʂ̏���
  Spfc *fc = NULL;
  //Sphe *he = NULL;
  for ( i = 0; i < n_sfc; ++i ) {
    fc = sfcall[i].fc;
    sfcall[i].fc = NULL;
    free_isolated_ppdface( fc, ppd );
  }

//    display("check 5\n");

  // ��O�p�`�ʂ̎O�p�`��
  for ( lv = tfc->clp->splv; lv != NULL; lv = lv->nxt ) {
    Splv *nlv = ( lv != tfc->clp->eplv ) ? lv->nxt : tfc->clp->splv;
    ed = find_ppdedge( lv->vt, nlv->vt );
    assert( ed != NULL );
    if ( ed->lhe != NULL ) {
      fc = ed->lhe->bpfc;
      if ( fc->hen > TRIANGLE ) {
	ppdface_triangulation( fc, ppd );
        free_isolated_ppdface( fc, ppd );
      }
    }
    if ( ed->rhe != NULL ) {
      fc = ed->rhe->bpfc;
      if ( fc->hen > TRIANGLE ) {
	ppdface_triangulation( fc, ppd );
        free_isolated_ppdface( fc, ppd );
      }
    }
  }

//  #if defined(_DEBUG)
//    display("---->5 vt %d neighbor edges %d\n", tmp_vt->no, ppdvertex_num_edge( tmp_vt ) );
//  #endif

  ////////////////////////////////////////////////////////////////////
  // TEST
//    for ( Splv* plv = tfc->clp->splv; plv != NULL; plv = plv->nxt ) {

//      Spvt* vt1 = plv->vt;
//      Spvt* vt2 = ( plv->nxt != NULL ) ? plv->nxt->vt : tfc->clp->splv->vt;
//      Sped* ped = find_ppdedge( vt1, vt2 );
//      display("ed %d lf %d rf %d\n",
//  	    ped->no,
//  	    (ped->lhe != NULL) ? ped->lhe->bpfc->no : SMDNULL,
//  	    (ped->rhe != NULL) ? ped->rhe->bpfc->no : SMDNULL );
//    }
  ////////////////////////////////////////////////////////////////////
  
//    display("check 6\n");
  
  // �������߂�
  free( sfcall );
  free( sedall );
  free( newfcall );
  free( newedall );
		   
}

//
// cyl_lp �̒ʂ��Ă��� ppd face �̕��� (for create_tile_cylindrical_face)
// 
void cyl_loop_subdivide_ppdface( TFace *tfc, Sppd *ppd )
{
//    display("check 1\n");
  // ONFACE �� loop edge ���� face �𒊏o
  int n_sfcall = tfc->cyl_lp[0]->lvn + tfc->cyl_lp[1]->lvn; 
  SubdivFace *sfcall = (SubdivFace *) malloc( n_sfcall * sizeof(SubdivFace) );
  int cnt = 0;
  int k;
  for ( k = 0; k < 2; ++k )
    {
      Splp *lp = tfc->cyl_lp[k];
      for ( Splv* lv = lp->splv; lv != NULL; lv = lv->nxt )
	{
	  Splv* slv = lv;
	  Splv* elv = ( lv != lp->eplv ) ? lv->nxt : lp->splv;
	  Sped* ed = find_ppdedge( slv->vt, elv->vt );
	  assert( ed != NULL );

	  // ���[�v�G�b�W�̕����ɃG�b�W�𐮗�
	  order_ppdedge( ed, slv->vt, elv->vt );
	  
	  // ����
	  if ( ed->type == EDGE_ONFACE )
	    {
	      SubdivFace *sfc;
	      if ( (sfc = find_subdivface( cnt, sfcall, ed->sp_fc )) == NULL )
		{
		  sfc = &(sfcall[cnt]);
		  sfc->en = 0;
		  ++cnt;
		}
	      sfc->fc = ed->sp_fc;
	      sfc->ed[sfc->en] = ed;
	      ++(sfc->en);
	    }
	}
    }
  
//    display("check 2\n");

  //
  // �o�H���ʂ�ʂ��Ƃɕ����ʂ̐���
  // (�����G�b�W�������ō쐬)
  int i;
  for ( i = 0; i < cnt; ++i )
    {
      subdivide_ppdface( &(sfcall[i]), ppd, NULL, NULL, NULL, NULL );
    }

//    display("check 3\n");

  //
  // ���E�̐ؒf���ꂽ�Ǘ��G�b�W�̏����� vt->type, ed->type �̍X�V
  //
  for ( k = 0; k < 2; ++k )
    {
      Splp *lp = tfc->cyl_lp[k];
      for ( Splv *lv = lp->splv; lv != NULL; lv = lv->nxt )
	{
	  
	  Sped *ed = NULL;
	  if ( lv->vt->type == VERTEX_ADDED )
	    {

	      // �Ǘ��G�b�W�̏���
	      ed = lv->vt->sp_ed;
	      // ���� ed �������_�̃����N��؂��Ă���
	      for ( Splv *clv = lp->splv; clv != NULL; clv = clv->nxt )
		{
		  if ( ed == clv->vt->sp_ed )
		    {
		      clv->vt->sp_ed = NULL;
		    }
		}
	      free_isolated_ppdedge( ed, ppd );

	      // vt->type �̍X�V
	      lv->vt->type = VERTEX_ORIGINAL;
	    }
    
	  // ed->type �̍X�V
	  Splv *nlv = ( lv != lp->eplv ) ? lv->nxt : lp->splv;
	  ed = find_ppdedge( lv->vt, nlv->vt );
	  assert( ed != NULL );
	  if ( ed == NULL ) continue;
	  if ( ed->type == EDGE_ONFACE )
	    {
	      ed->sp_fc = NULL;
	      ed->type = EDGE_ORIGINAL;
	    }
    
	}
    }

//    display("check 4\n");
  
  // �Ǘ��ʂ̏���
  Spfc *fc = NULL;
  //Sphe *he = NULL;
  for ( i = 0; i < cnt; ++i )
    {
      fc = sfcall[i].fc;
      sfcall[i].fc = NULL;
      free_isolated_ppdface( fc, ppd );
    }

//    display("check 5\n");

  // ��O�p�`�ʂ̎O�p�`��
  for ( k = 0; k < 2; ++k )
    {
      Splp *lp = tfc->cyl_lp[k];
      for ( Splv *lv = lp->splv; lv != NULL; lv = lv->nxt )
	{
	  Splv *nlv = ( lv != lp->eplv ) ? lv->nxt : lp->splv;
	  Sped *ed = find_ppdedge( lv->vt, nlv->vt );
	  assert( ed != NULL );
	  if ( ed->lhe != NULL )
	    {
	      fc = ed->lhe->bpfc;
	      if ( fc->hen > TRIANGLE )
		{
		  ppdface_triangulation( fc, ppd );
		  free_isolated_ppdface( fc, ppd );
		}
	    }
	  if ( ed->rhe != NULL )
	    {
	      fc = ed->rhe->bpfc;
	      if ( fc->hen > TRIANGLE )
		{
		  ppdface_triangulation( fc, ppd );
		  free_isolated_ppdface( fc, ppd );
		}
	    }
	}
    }

//    display("check 6\n");
  
  // �������߂�
  free( sfcall );
		   
}

void order_ppdedge( Sped *ed, Spvt *sv, Spvt *ev )
{
  if ( (ed->sv == sv) && (ed->ev == ev) ) return;
  
  //   display("ordered.\n");
  // reverse halfedge
  Sphe *he = ed->lhe;
  ed->lhe = ed->rhe;
  ed->rhe = he;

  Spvt *vt = ed->sv;
  ed->sv = ed->ev;
  ed->ev = vt;
}

//
// face �̃O���[�v��
// vertex �� face �� id �����C���o
//
void tface_grouping( TFace *tfc, Sppd *ppd )
{
  int id = tfc->no;
  Sped *fed = find_ppdedge( tfc->clp->splv->vt, tfc->clp->splv->nxt->vt );

  // �O���[�v��
  Spfc *fc = fed->lhe->bpfc;
  ppdface_grouping( tfc, fc, id );
  
  // tvector �̍쐬
  tfc->tvec = (TVector *) malloc( tfc->n_tvector * sizeof(TVector) );
  int i;
  for ( i = 0; i < tfc->n_tvector; ++i ) {
    initialize_tvector( &(tfc->tvec[i]) );
  }
//    display("tvector no %d\n", tfc->n_tvector );
  i = 0;
  for ( Spvt *vt = ppd->spvt; vt != NULL; vt = vt->nxt ) {
    if ( vt->tile_id != id ) continue;
    tfc->tvec[i].vt = vt;
//      display("tvector vt %d\n", vt->no );
    ++i;
    //vt->tile_id = SMDNULL;
  }
  
}

void ppdface_grouping( TFace *tfc, Spfc *fc, int id )
{
  if ( fc->tile_id == id ) return;
  
  fc->tile_id = id;

  // �p�[�g����
  fc->bppt = tfc->pt;
    
  Sphe *he = fc->sphe;
  do {

    // vertex grouping
    if ( he->vt->tile_id != id ) {
      he->vt->tile_id = id;
      ++( tfc->n_tvector );
    }

    // other face grouping
    if ( (he->mate != NULL) && (he->ed->sp_type != SP_EDGE_BOUNDARY) ) {
      //display("fc %d\n", he->mate->bpfc->no );
      ppdface_grouping( tfc, he->mate->bpfc, id );
    }
    
  } while ( (he = he->nxt) != fc->sphe );
}

#define TEMPNUM -999

//
// cylindrical face �̃O���[�v��
// vertex �� face �� id �����C���o
//
void cyl_tface_grouping( TFace *tfc, Sppd *ppd )
{
  // �ʂɂ���ԍ�
  int id = tfc->no;

  // cyl_lp[0] �̕����猟��
  Splp *lp = tfc->cyl_lp[0];
  Sped *fed = find_ppdedge( lp->splv->vt, lp->splv->nxt->vt );

  Sphe* he = fed->lhe;
  //Sphe *he = (k == 0) ? fed->lhe : fed->rhe;
  if ( he == NULL ) return;
  Spfc *fc = he->bpfc;

  ppdface_grouping( tfc, fc, id );
  
//    for ( int k = 0; k < 2; ++k )
//      {
//        // �܂��͍����C���ɉE���̖ʂɑ΂��ăO���[�v���̃e�X�g
//        Sphe *he = (k == 0) ? fed->lhe : fed->rhe;
//        if ( he == NULL ) continue;
//        Spfc *fc = he->bpfc;
      
//        if ( check_ppdface_grouping( tfc, fc, TEMPNUM ) == TRUE )
//  	{
//  	  renumber_ppdface( TEMPNUM, id ); // �������ԍ��ɕt������
//  	  break;
//  	}
//        else
//  	{
//  	  renumber_ppdface( TEMPNUM, SMDNULL ); // �ԍ������Z�b�g
//  	}
//      }
  
  // tvector �̍쐬
  tfc->tvec = (TVector *) malloc( tfc->n_tvector * sizeof(TVector) );
  int i;
  for ( i = 0; i < tfc->n_tvector; ++i )
    {
      initialize_tvector( &(tfc->tvec[i]) );
    }
//    display("tvector no %d\n", tfc->n_tvector );

  i = 0;
  for ( Spvt *vt = ppd->spvt; vt != NULL; vt = vt->nxt )
    {
      if ( vt->tile_id != id ) continue;
      tfc->tvec[i].vt = vt;
//        display("tvector vt %d\n", vt->no );
      ++i;
      //vt->tile_id = SMDNULL;
    }
  
}

//
// ppd �̒��_�ɑ΂��� tvector �̔ԍ��t��
//
void rename_tvector( TFace *tfc )
{
  for ( int i = 0; i < tfc->n_tvector; ++i ) {
    tfc->tvec[i].vt->tile_id = tfc->no;
    tfc->tvec[i].vt->tvec_id = i;
  }
}
      
//
// vt (tile_id == no) �̗אڂ̒��_�̂����Ctile_id == no �̂��̂̐�
// (used in tfc_umbrella_operator*)
//
int tfc_num_neighbor( Spvt *vt )
{
  int num = 0;
  for (Vted *vted = vt->svted; vted != NULL; vted = vted->nxt ) {
    Spvt *avt = another_vt( vted->ed, vt );
    assert( avt );
    if ( avt->tile_id == vt->tile_id )
      {
	++num;
      }
  }
  return num;
}

//
void tfc_calc_boundary( TFace* tfc )
{
  Splp* lp = tfc->clp;
  for ( Splv *lv = lp->splv; lv != NULL; lv = lv->nxt )
    {

      Spvt *vt0 = lv->vt;
      Spvt *vt1 = ( lv == lp->eplv ) ? lp->splv->vt : lv->nxt->vt;
      Sped *ed = find_ppdedge_new( vt0, vt1 );
      ed->isBoundary = TRUE;
    }
}
  
