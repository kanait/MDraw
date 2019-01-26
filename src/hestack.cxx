//
// hestack.cxx
//
// Hestack functions for subdividing faces according to the path edge
//
// Copyright (c) 1999 Takashi Kanai
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
#include "ppdvertex.h"
#include "ppdface.h"
#include "ppdedge.h"
#include "veclib.h"

#include "hestack.h"

//
// face subdivision algorithm (new)
//
// (��̖ʂɏ���Ă��镪���G�b�W�� 4 �{�܂łƂ�������������)
// �^�C��������쐬���C���̓s�x�ʂ��A�b�v�f�[�g����
// �A�b�v�f�[�g:
// - ed->sp_fc (only if ed->type = EDGE_ONFACE )
//
void subdivide_ppdface( SubdivFace *sfc, Sppd *ppd,
			int* n_newfc, SubdivFace* newfcall,
			int* n_newed, SubdivEdge* newedall )
{
  Spfc *fc = sfc->fc;

  if ( fc == NULL ) return;
  //if ( fc->hn != TRIANGLE ) return;

  //
  // Hestack �̏�����
  // Hestack, Hevt �֊i�[
  // - he->vt �� Hevt (�̐擪) �Ɋi�[
  // - val �ɂ���ă\�[�g
  // �� Hevt ����G�b�W�ւ̑o�������񃊃X�g�����
  //    (���̉ߒ��ŕK�v�ȃG�b�W����������)
  // newedall �ւ̊i�[���s��
  //
  Hestack *hestack = (Hestack *) malloc( fc->hen * sizeof( Hestack ) );
  initialize_hestack( sfc, hestack, ppd, n_newed, newedall );
  
  //
  // Hestack ���g���Ėʂ𐶐�
  // ����������� (���Ȃ��Ă͂Ȃ�Ȃ�����)
  // - face
  // - halfedge
  // - fc->nrm
  // newfcall �ւ̊i�[���s��
  subdivide_ppdface_by_hestack( fc, hestack, ppd, n_newfc, newfcall );

  // 
  // �������߂�
  //
  for ( int i = 0; i < fc->hen; ++i ) {
    for ( int j = 0; j < hestack[i].hevtn; ++j ) {
      free_halfedgevertexedge( &(hestack[i].hevt[j]) );
    }
    free( hestack[i].hevt );
  }
  free( hestack );
}

/////////////////////////////////////////////////////////////////////
//
// Hestack Functions
//

//
// Hestack �̏�����
// Hestack, Hevt �֊i�[
// - he->vt �� Hevt (�̐擪) �Ɋi�[
// - val �ɂ���ă\�[�g
// �� Hevt ����G�b�W�ւ̑o�������񃊃X�g�����
//
void initialize_hestack( SubdivFace *sfc, Hestack *hestack, Sppd *ppd,
			 int* n_newed, SubdivEdge* newedall )
{ 
  Spfc *fc = sfc->fc;
  Sphe *he = fc->sphe;
  // temp
//    display("fc %d\n", fc->no );
//    do {
//      display("he vt %d ed %d\n", he->vt->no, he->ed->no );
//    } while ( (he = he->nxt) != fc->sphe );
  //
  // Hestack �̏�����
  //
  int i, j, n;
  //Sphe *he = fc->sphe;
  for ( i = 0; i < fc->hen; ++i ) {

    hestack[i].he = he;
    hestack[i].hevtn = 0;

    // 
    // hevt �̏�����
    // �ő�� (sfc->en + 1) �̐��� hevt �����̂ł��ꂾ���m��
    // sfc->en: ���̖ʂ�ʂ�G�b�W�̐� + 1 (���̃G�b�W (�n�[�t�G�b�W) �̎n�_)
    //
    hestack[i].hevt = (Hevt *) malloc( (sfc->en +1) * sizeof(Hevt) );
    for ( j = 0; j < (sfc->en + 1); ++j ) initialize_hevt( &(hestack[i].hevt[j]) );
    
    he = he->nxt;
    
  }

  //
  // Hestack, Hevt �֊i�[
  // - he->vt �� Hevt (�̐擪) �Ɋi�[
  // - val �ɂ���ă\�[�g
  //

  // he->vt �� Hevt (�̐擪) �Ɋi�[
  he = fc->sphe;
  i = 0;
  do {
    hestack[i].hevt[0].vt = he->vt;
    hestack[i].hevt[0].val = 0.0;
    //hestack[i].hevt[0].isUsed = FALSE;
    ++(hestack[i].hevtn);
    ++i;
  } while ( (he = he->nxt) != fc->sphe );

  
  // �����G�b�W�̒��_�̑}��
  Sped *ed;
  for ( i = 0; i < sfc->en; ++i ) {
    ed = sfc->ed[i];
    if ( ed->sv->type != VERTEX_ORIGINAL ) {
      n = find_henum_hestack( ed->sv->sp_ed, fc->hen, hestack );
//        display("n %d\n", n );
      assert( n != SMDNULL );
      insert_hevt_in_hestack( ed->sv, ed, &(hestack[n]) );
    }
    if ( ed->ev->type != VERTEX_ORIGINAL ) {
      n = find_henum_hestack( ed->ev->sp_ed, fc->hen, hestack );
//        display("n %d\n", n );
      assert( n != SMDNULL );
      insert_hevt_in_hestack( ed->ev, ed, &(hestack[n]) );
    }
  }

  // i �� j �� hevt �̒��ɓ���� (��Ŏg���̂�)
  for ( i = 0; i < fc->hen; ++i ) {
    for ( j = 0; j < hestack[i].hevtn; ++j ) {
      hestack[i].hevt[j].hsid = i;
      hestack[i].hevt[j].hvid = j;
    }
  }

  // hevted (�o�������񃊃X�g) ���쐬����
  //
  // 1. �O�p�`���[�v��̃G�b�W���� hevted ���쐬���鎞
  //   ���̒��_�ɓ���G�b�W�� val=1, �o��G�b�W�� val =0 �Ƃ���
  //
  // 2. �����G�b�W���� hevted ���쐬���鎞
  //   2.1 �������_�ɓ��鎞 ( vt->type != ORIGINAL )
  //     val=0.5 �Ƃ��đ}��
  //     ( ����: �����������_�� 2 �{�ȏ�̃G�b�W������
  //     ���Ƃ͂Ȃ� -> ���E�G�b�W�̓��͎��Ƀ`�F�b�N���� )
  //   2.2 �O�p�`�̒��_�̎�
  //     ���Α��� hestack �ɂ���Ή����� hevt->val �̒l��p����

  // �O�p�`���[�v��̃G�b�W���� hevted ���쐬����
  Hevt *hevt = &(hestack[0].hevt[0]);
  Hevt *nhevt = NULL;
  Spvt *vt, *nvt;
  do {
    nhevt = next_hevt_triloop( hevt, fc->hen, hestack );

    vt  = hevt->vt;

//      if ( vt->no == 10323 ) display("vt %d fc %d \n", vt->no, fc->no );
    
    nvt = nhevt->vt;
    if ( ( ed = find_ppdedge( vt, nvt ) ) == NULL ) {
      ed = create_ppdedge( ppd );
      ed->sv = vt;
      ed->ev = nvt;
      ed->length = V3DistanceBetween2Points( &(ed->sv->vec), &(ed->ev->vec) );
      (void) create_vtxed( vt, ed );
      (void) create_vtxed( nvt, ed );
      if ( ed->no == 37391 ) {
	display("@@@@ ed %d sv %d %g %g %g ev %d %g %g %g\n",
		ed->no,
		ed->sv->no, ed->sv->vec.x, ed->sv->vec.y, ed->sv->vec.z, 
		ed->ev->no, ed->ev->vec.x, ed->ev->vec.y, ed->ev->vec.z );
      }

      // newedall �ւ̊i�[
      if ( n_newed != NULL )
	{
	  // �V�����������ꂽ�G�b�W�̊i�[
	  newedall[*n_newed].ed = ed; ++(*n_newed);
	}
    }
    (void) insert_hevted( 0.0, ed, hevt );
    (void) insert_hevted( 1.0, ed, nhevt );
    
    hevt = nhevt;
  } while ( hevt != &(hestack[0].hevt[0]) );
  
  // �����G�b�W���� hevted ���쐬����
  for ( i = 0; i < sfc->en; ++i ) {
    ed = sfc->ed[i];
    for ( j = 0; j < 2; ++j ) {
      vt = (!j) ? ed->sv : ed->ev;
      hevt = find_hevt_from_hestack( vt, fc->hen, hestack );
      if ( vt->type == VERTEX_ADDED ) {
	// val=0.5 �Ƃ��đ}��
	(void) insert_hevted( 0.5, ed, hevt );
      } else {
	Spvt *pvt = another_vt( ed, vt );
	Hevt *phevt = find_hevt_from_hestack( pvt, fc->hen, hestack );
	(void) insert_hevted( phevt->val, ed, hevt );
      }
    }
  }

  // hevted �� mate ���쐬����
  hevted_makemate( fc->hen, hestack );

  // temp
  for ( i = 0; i < fc->hen; ++i ) {

    display("hestack no.%d ed %d hevtn %d\n",
	    i,
	    hestack[i].he->ed->no,
	    hestack[i].hevtn );
    for ( j = 0; j < hestack[i].hevtn; ++j ) {
      display("\thevt hsid %d hvid %d vt %d val %g\n",
	      hestack[i].hevt[j].hsid,
	      hestack[i].hevt[j].hvid,
	      hestack[i].hevt[j].vt->no,
	      hestack[i].hevt[j].val );
      for ( Hevted *hve = hestack[i].hevt[j].svted; hve != NULL; hve = hve->nxt ) {
	display("\t hevted ed %d val %g mate ed %d vt %d\n",
		hve->ed->no,
		hve->val,
		( hve->mate != NULL ) ? hve->mate->ed->no : -1,
		( hve->mate != NULL ) ? hve->mate->hevt->vt->no : -1
		);
      }
    }
  }

}

//
// he->ed ==ed �ƂȂ� hestack �̔ԍ����Ƃ��Ă���
//
int find_henum_hestack( Sped *ed, int hen, Hestack *hs )
{
  for ( int j = 0; j < hen; ++j ) {
    if ( hs[j].he->ed == ed ) {
      return j;
    }
  }
  return SMDNULL;
}

//
// �O�p�`�̕Ӄ��[�v��̎��� hevt ������Ă���
//
Hevt *next_hevt_triloop( Hevt *hevt, int hn, Hestack *hs )
{
  // hestack �̎��� hevt ��Ԃ�
  int i = hevt->hsid;
  int j = hevt->hvid;
  Hestack *chs = &(hs[i]);
  if ( (i == hn - 1) && (j == chs->hevtn - 1) ) {
    return &(hs[0].hevt[0]);
  } else if ( j == chs->hevtn - 1 ) {
    return &(hs[i+1].hevt[0]);
  } else {
    return &(hs[i].hevt[j+1]);
  }
}

//
// hestack ���g���Ėʂ𕪊�����
//
void subdivide_ppdface_by_hestack( Spfc *fc, Hestack *hestack, Sppd *ppd,
				   int* n_newfc, SubdivFace* newfcall )
{
  for (int i = 0; i < fc->hen; ++i ) {
    for ( int j = 0; j < hestack[i].hevtn; ++j ) {
      Hevt *hevt = &(hestack[i].hevt[j]);
      for ( Hevted *hve = hevt->svted; hve != NULL; hve = hve->nxt ) {
	
	if ( isvalid_create_ppdface( hve, hevt ) == TRUE ) {
	  // create face
	  Spfc *newfc = create_ppdface( ppd );
	  newfc->bpso = ppd->spso;
	  newfc->bppt = ppd->sppt;
	  display("create fc %d\n", newfc->no );
	  
	  // newfcall �ւ̊i�[
	  if ( n_newfc != NULL )
	    {
	      newfcall[*n_newfc].fc = fc; ++(*n_newfc);
	    }

	  Hevted *chve = hve;
	  Hevt *chevt = hevt;
	  do {
	    chve->isVisited = TRUE;
	    
	    { // create halfedge begin

	      Sphe *newhe = create_ppdhalfedge( newfc );
	      newhe->vt = chevt->vt;
	      newhe->ed = chve->ed;

	      // mate �̕t������
	      Sped *ed = newhe->ed;
	      if ( ed->sv == newhe->vt ) {
		ed->lhe = newhe;
		if ( ed->rhe != NULL ) {
		  newhe->mate = ed->rhe;
		  ed->rhe->mate = newhe;
		}
	      } else {
		ed->rhe = newhe;
		if ( ed->lhe != NULL ) {
		  newhe->mate = ed->lhe;
		  ed->lhe->mate = newhe;
		}
	      }

	      // vertex -> halfedge link
	      newhe->vt->sphe = newhe;

	      // edge �̑����̏C�� (EDGE_ONFACE -> EDGE_ORIGINAL)
	      if ( newhe->ed->type == EDGE_ONFACE )
		{
		  newhe->ed->type = EDGE_ORIGINAL;
		  newhe->ed->sp_fc = NULL;
		}
	    
	      display("\tcreate halfedge vt %d ed %d\n", 
		      newhe->vt->no, newhe->ed->no );
	      
	    } // create halfedge end
	    
	    Hevted *mate = chve->mate;
	    chevt = mate->hevt;
	    chve = mate->prv;
	  } while ( chevt != hevt );

	  // normal
	  calc_fnorm( newfc );
	}
      }
    }
  }
}

BOOL isvalid_create_ppdface( Hevted *hve, Hevt *hevt )
{
  Hevted *chve = hve;
  Hevt *chevt = hevt;
  do {
    if ( chve->isVisited == TRUE ) return FALSE;
    Hevted *mate = chve->mate;
    chevt = mate->hevt;
    if ( mate->prv == NULL ) return FALSE;
    chve = mate->prv;
  } while ( chevt != hevt );

  return TRUE;
}
  
/////////////////////////////////////////////////////////////////////
//
// Hevt Functions
//

void initialize_hevt( Hevt *hevt )
{
  hevt->vt = NULL;
  hevt->hsid = SMDNULL;
  hevt->hvid = SMDNULL;
//    hevt->isUsed = FALSE;
  hevt->nhve = 0;
  hevt->svted = NULL;
  hevt->evted = NULL;
  
}
  
//
// �\�[�g���� Hestack �ɓ���� ( vt->type == VERTEX_ADDED �̂�)
//
void insert_hevt_in_hestack( Spvt *vt, // �}�����钸�_
			     Sped *ed, // �؂����G�b�W (�؂�ꂽ�G�b�W�ł͂Ȃ�)
			     Hestack *hs )
{
  double val = ( vt->sp_ed->sv == hs->he->vt ) ? vt->sp_val : 1.0 - vt->sp_val;
  int n = SMDNULL;
  for ( int i = 1; i < hs->hevtn; ++i ) {
    if ( hs->hevt[i].vt == NULL ) {
      // �ǉ�
      n = i;
      break;
    } else {
      if ( hs->hevt[i].val > val ) {
	// �}��
	n = i;
	// ������炷
	for ( int j = hs->hevtn; j > i; --j ) {
	  //if ( j - 1 == 0 ) continue; // he->vt �͓������Ȃ�
	  hs->hevt[j].vt = hs->hevt[j-1].vt;
	  hs->hevt[j].val = hs->hevt[j-1].val;
//	  hs->hevt[j].isUsed = hs->hevt[j-1].isUsed;
	}
	break;
      }
    }
  }
  if ( n == SMDNULL ) n = hs->hevtn;
//    display("----- n %d\n", n );
  assert( n != SMDNULL );
  hs->hevt[n].vt = vt;
  hs->hevt[n].val = val;
//  hs->hevt[n].isUsed = FALSE;

  ++(hs->hevtn);

//    display("(tmp)hevtn %d\n", hs->hevtn );
//    for ( i = 0; i < hs->hevtn; ++i ) {
//      display("\thevt hsid %d hvid %d vt %d val %g\n", 
//  	    hs->hevt[i].hsid,
//  	    hs->hevt[i].hvid,
//  	    hs->hevt[i].vt->no,
//  	    hs->hevt[i].val );
//    }
    
}

//
// ���� hevt ������Ă���
//
//  Hevt *next_hevt( Hevt *hevt, int hn, Hestack *hs )
//  {
//    // hevt->ed �����ꍇ�́C�G�b�W�̂�������̒��_���C�܂�
//    // �Ȃ����Ă��Ȃ� (isUsed = FALSE) �ꍇ�ɂ̂݁C���̒��_�ƂȂ�
//    if ( hevt->ed != NULL ) {
//      Spvt *vt = (hevt->ed->sv != hevt->vt) ? hevt->ed->sv : hevt->ed->ev;
//      // vt ������ hevt ��������
//      Hevt *nhv = find_hevt_from_hestack( vt, hn, hs );
//      assert( nhv != NULL );
//      if ( nhv->isUsed == FALSE ) return nhv;
//    }

//    // hestack �̎��� hevt ��Ԃ�
//    int i = hevt->hsid;
//    int j = hevt->hvid;
//    Hestack *chs = &(hs[i]);
//    if ((i == hn - 1) && (j == chs->hevtn - 1)) {
//      return &(hs[0].hevt[0]);
//    } else if (j == chs->hevtn - 1) {
//      return &(hs[i+1].hevt[0]);
//    } else {
//      return &(hs[i+1].hevt[j+1]);
//    }
//  }

//
// vt ������ hevt ��Ԃ�
//
Hevt *find_hevt_from_hestack( Spvt *vt, int hn, Hestack *hs )
{
  for ( int i = 0; i < hn; ++i ) {
    for ( int j = 0; j < hs[i].hevtn; ++j ) {
      if ( hs[i].hevt[j].vt == vt ) return &(hs[i].hevt[j]);
    }
  }
  return NULL;
}

//
// hestack �̒��ł܂� isUsed = TRUE �ɂȂ��Ă��Ȃ����_������Ă���
//
//  Hevt *hestack_unused_hevt( int hn, Hestack *hs )
//  {
//    for ( int i = 0; i < hn; ++i ) {
//      for ( int j = 0; j < hs[i].hevtn; ++j ) {
//        if ( hs[i].hevt[j].isUsed == FALSE ) return &(hs[i].hevt[j]);
//      }
//    }
//    return NULL;
//  }

/////////////////////////////////////////////////////////////////////
//
// Hevted Functions (�o�������񃊃X�g)
//

//
// Create Hevted
Hevted *create_hevted( Sped *ed, Hevt *hevt )
{
  Hevted *hve;

  if (ed == NULL) return NULL;
  
  hve = (Hevted *) malloc( sizeof(Hevted) );

  hve->nxt = NULL;
  if ( hevt->svted == NULL) { // first
    hve->prv    = NULL;
    hevt->evted = hevt->svted = hve;
  } else {
    hve->prv    = hevt->evted;
    hevt->evted = hevt->evted->nxt = hve;
  }
  
  hve->ed = ed;
  hve->hevt = hevt;
  hve->mate = NULL;
  hve->isVisited = FALSE;
  hve->val = 0.0;

  ++( hevt->nhve );
  
  return hve;
}

Hevted *find_hevted( Sped *ed, Hevt *hevt )
{
  for ( Hevted *hve = hevt->svted; hve != NULL; hve = hve->nxt ) {
    if ( hve->ed == ed ) return hve;
  }
  return NULL;
}

//
// �l val ��]���ɂ��� nhve ��}��
//
Hevted *insert_hevted( double val, Sped *ed, Hevt *hevt )
{
//    display("insert.\n");
  Hevted *nhve;
  if ( hevt->svted == NULL ) { // first
    nhve = create_hevted( ed, hevt );
    nhve->val = val;
    return nhve;
  }

  //
  // hve->val �̒l�� val ������������ hve ��T��
  //
  Hevted *hve = hevt->svted;
  while ( (hve != NULL) && (val > hve->val) ) {
//      display("--- hve ed %d val %g\n", hve->ed->no, hve->val );
    hve = hve->nxt;
  }

  if ( hve == NULL ) { // last
    nhve = create_hevted( ed, hevt );
    nhve->val = val;
    return nhve;
  } 

  //
  // hve �̒��O�� nhve ��}������
  //
//    display("hve ed %d val %g\n", hve->ed->no, hve->val );
  nhve = (Hevted *) malloc( sizeof(Hevted) );
  nhve->prv = hve->prv;
  nhve->nxt = hve;
  if ( hve->prv != NULL ) hve->prv->nxt = nhve;
  hve->prv = nhve;
  if ( hevt->svted == hve ) hevt->svted = nhve;

  nhve->ed = ed;
  nhve->hevt = hevt;
  nhve->mate = NULL;
  nhve->val = val;
  nhve->isVisited = FALSE;
  
  ++( hevt->nhve );

  return nhve;
}

void free_halfedgevertexedge( Hevt *hevt )
{
  Hevted *hve, *nhve;
  for ( hve = hevt->svted; hve != NULL; hve = nhve ) {
    nhve = hve->nxt;
    free_hevted( hve, hevt );
  }
  hevt->nhve = 0;
  hevt->svted = hevt->evted = NULL;
}

void free_hevted( Hevted *hve, Hevt *hevt )
{
  if ( hve == NULL ) return;

  hve->ed = NULL;
  hve->hevt = NULL;

  // mate �̃����N���͂���
  if ( hve->mate != NULL ) {
    hve->mate->mate = NULL;
    hve->mate = NULL;
  }
  
  if ( hevt->svted == hve ) {
    if ( (hevt->svted = hve->nxt) != NULL )
      hve->nxt->prv = NULL;
    else {
      hevt->evted = NULL;
    }
  } else if ( hevt->evted == hve ) {
    hve->prv->nxt = NULL;
    hevt->evted = hve->prv;
  } else {
    hve->prv->nxt = hve->nxt;
    hve->nxt->prv = hve->prv;
  }
  free( hve );
  --( hevt->nhve );
}

void hevted_makemate( int hen, Hestack *hs )
{
  for ( int i = 0; i < hen; ++i ) {
    for ( int j = 0; j < hs[i].hevtn; ++j ) {

      Hevt *hevt = &(hs[i].hevt[j]);
      for ( Hevted *hve = hevt->svted; hve != NULL; hve = hve->nxt ) {
	// �������ł� mate ������
	Sped *ed = hve->ed;
	if ( hve->mate == NULL ) {
	  // mate ��T��
	  Spvt *pvt = another_vt( ed, hevt->vt );
	  Hevt *phevt = find_hevt_from_hestack( pvt, hen, hs );
	  Hevted *phve = find_hevted( ed, phevt );
	  assert( phve != NULL );
	  hve->mate = phve;
	  phve->mate = hve;
	}
      }
    }
  }
}
	
// SubdivFace �֐�

void init_subdivface( SubdivFace* sfc )
{
  // �I���W�i����
  sfc->fc = NULL;

  // �������邽�߂̃G�b�W
  sfc->en = 0;

  // �������ꂽ��
//    sfc->fn = 0;
}
  
SubdivFace* find_subdivface( int fn, SubdivFace* sfcall, Spfc* fc )
{
  for ( int i = 0; i < fn; ++i )
    {
      if ( sfcall[i].fc == fc ) return &(sfcall[i]);
    }
  return NULL;
}

// SubdivEdge �֐�

void init_subdivedge( SubdivEdge* sed )
{
  // �I���W�i���G�b�W
  sed->ed = NULL;

  // �������邽�߂̒��_
  sed->vn = 0;

  // �������ꂽ�G�b�W
//    sed->en = 0;
}
  
SubdivEdge *find_subdivedge( int en, SubdivEdge* sedall, Sped* ed )
{
  for ( int i = 0; i < en; ++i )
    {
      if ( sedall[i].ed == ed ) return &(sedall[i]);
    }
  return NULL;
}
