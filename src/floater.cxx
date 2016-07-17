//
// floater.cxx
//
// Copyright (c) 1997-2000 Takashi Kanai; All rights reserved. 
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
#include "tile.h"
#include "veclib.h"
#include "linbcg.h"
#include "intsec.h"

#include "floater.h"

///////////////////////////////////////////////////////////////////////////
//
// shape-preserving parameterization (in [Floater97(CAGD)])
// for ppd with a boundary
//
///////////////////////////////////////////////////////////////////////////

void ppdspm( Sppd *ppd )
{

  // ������
  ppdspm_initialize( ppd );
  
  // matrix creation process
  Semat *emat = create_spm_emat( ppd );
  create_spm_vector( ppd, emat );

  // iteration process
  int    iter;
  double rsq;
  (void) linbcg( emat, emat->bx, emat->xx, 1, SMDZEROEPS, 1000, &iter, &rsq );
  (void) linbcg( emat, emat->by, emat->yy, 1, SMDZEROEPS, 1000, &iter, &rsq );

  // vectors -> ppdvertex
  solvec_ppdvertex( ppd, emat );

  //free emat, vtxvt
  free_emat( emat );

  ppdspm_finish( ppd );
}  

// 
// ������
// 
void ppdspm_initialize( Sppd *ppd )
{
  // �G�b�W�̎����Ȃ����_���폜
  free_ppdvertex_noedge( ppd );
  
  // �����̌���
  // lambda, vec2d �̗̈�m��
  Spvt *vt;
  int i;
  for ( i = 0, vt = ppd->spvt; vt != NULL; vt = vt->nxt, ++i )
    {
    
      // ���_�ԍ��̏�����
      vt->sid = i;

      // �����̌���
      vt->n_degree = ppdvertex_num_edge( vt );
    
      // lambda, vec2d initialize
      vt->vec2d  = (Vec2d *) malloc( (vt->n_degree + 1) * sizeof(Vec2d) );
      vt->lambda = (double *) malloc( (vt->n_degree + 1) * sizeof(double) );
    
    }

  // �G�b�W�̒���
  Sped *ed;
  for ( i = 0, ed = ppd->sped; ed != NULL; ed = ed->nxt, ++i )
    {
      ed->sid = i;
      ed->length = V3DistanceBetween2Points( &(ed->sv->vec),&(ed->ev->vec) );
    }

  // �ʂ̔ԍ��t���C�ʐς̌v�Z
  Spfc *fc;
  for ( i = 0, fc = ppd->spfc; fc != NULL; fc = fc->nxt, ++i )
    {
      fc->sid = i;
      fc->area = calc_farea( fc );
    }
}

// 
// ������
// 
void ppdspm_finish( Sppd *ppd )
{
  // lambda, vec2d �̗̈�폜
  for ( Spvt *vt = ppd->spvt; vt != NULL; vt = vt->nxt )
    {
      free( vt->lambda );  vt->lambda = NULL;
      free( vt->vec2d );  vt->vec2d = NULL;
    }
}

Semat *create_spm_emat( Sppd *ppd )
{
  // �ɂ̌���
  spm_lambda( ppd );

  // �}�g���b�N�X�̍s���𐔂���
  int vn = 0;
  Spvt *vt;
  for ( vt = ppd->spvt; vt != NULL; vt = vt->nxt ) {
    if ( vt->isBoundary == FALSE ) {
      vn += vt->n_degree;
    }
  }

  //
  // create indexed sparse matrix
  //
 Semat *emat = init_emat( ppd->vn );
  emat->num = vn + ppd->vn + 1;
  emat->sa  = (double *) malloc(emat->num * sizeof(double));
  int i;
  for (i = 0; i < emat->num; ++i) emat->sa[i] = 0.0;
  emat->ija = (int *) malloc(emat->num * sizeof(int));
  
  //
  // indexed sparse matrix �e�s�̗v�f�ɒl���i�[����
  //
  emat->ija[0] = emat->cnum + 1;
  int j = emat->cnum;
  Sped *ed, *fed;
  for ( vt = ppd->spvt; vt != NULL; vt = vt->nxt ) {
    
    emat->sa[vt->sid] = 1.0;

    if ( vt->isBoundary == FALSE ) {
      ed = ppdvertex_first_edge( vt );
      fed = ed;
      i = 1;
      do {
	++j;
	emat->sa[j]  = -1.0 * vt->lambda[i];
	emat->ija[j] = another_vt( ed, vt )->sid;
	++i;
	ed = ppdvertex_next_edge( ed, vt );
      } while ( (ed != NULL) && (ed != fed) );
    }
    
    emat->ija[vt->sid + 1] = j+1;
  }
  
  return emat;
}

#define RADIUS 1.0

static double cornerx[4] = {0.0, 1.0, 1.0, 0.0};
static double cornery[4] = {0.0, 0.0, 1.0, 1.0};

void create_spm_vector( Sppd *ppd, Semat *emat )
{
  // initialize vector
  int i;
  for ( i = 0; i < ppd->vn; ++i ) {
    emat->bx[i] = emat->by[i] = 0.0;
    emat->xx[i] = emat->yy[i] = 0.0;
  }

  // outer loop �̃G�b�W�̒����̌v�Z
  Splp *lp = ppd->splp;

  // ���[�v�̃G�b�W�����̑��a
  Splv *lv;
  double sum = 0.0;
  for ( lv = lp->splv; lv != NULL; lv = lv->nxt ) {
    Splv *nlv = (lv->nxt != NULL ) ? lv->nxt : lp->splv;
    Sped *ed = find_ppdedge( lv->vt, nlv->vt );
    assert( ed != NULL );
    sum += ed->length;
  }

  // lvn:  ���[�v�̒��_�̐�
  // lcvn: ���[�v�̊p���_�̐�
  // ���[�v�̈�ԍŏ��̓_ (lp->splv) �́C�p�̓_
  Vec2d* vtb = (Vec2d*) malloc( lp->lcvn * sizeof(Vec2d) );
  double *midsum = (double *) malloc( lp->lcvn * sizeof(double) );
  for ( i = 0; i < lp->lcvn; ++i ) midsum[i] = 0.0;
  int cnt = -1;
  double angle = 0;
  for ( lv = lp->splv; lv != NULL; lv = lv->nxt ) {

    if ( lv->type == LOOPVT_CORNER ) {
      ++cnt;
      int id = lv->vt->sid;
      // for uv mapping
      vtb[cnt].x = emat->bx[id] = cornerx[cnt];
      vtb[cnt].y = emat->by[id] = cornery[cnt];
//        vtb[cnt].x = emat->bx[id] = RADIUS * cos(angle);
//        vtb[cnt].y = emat->by[id] = RADIUS * sin(angle);
    }

    Splv *nlv = (lv->nxt != NULL ) ? lv->nxt : lp->splv;
    Sped *ed = find_ppdedge( lv->vt, nlv->vt );
    assert( ed != NULL );
    midsum[cnt] += ed->length;
    angle += (2 * SMDPI * ed->length / sum);
    
  }

  cnt = -1;
  double a = 0.0;
  for ( lv = lp->splv; lv != NULL; lv = lv->nxt ) {

    if ( lv->type == LOOPVT_CORNER )
      {
	++cnt;
	a = 0.0;
      }
    else
      {
	int p = cnt;
	int q = (cnt != lp->lcvn - 1) ? cnt+1 : 0;
	double b = a / midsum[p];
	int id = lv->vt->sid;
	emat->bx[id] = vtb[p].x + b * (vtb[q].x - vtb[p].x);
	emat->by[id] = vtb[p].y + b * (vtb[q].y - vtb[p].y);
//  	emat->bx[id] = ( b * vtb[p].x + a * vtb[q].x ) / midsum[p];
//  	emat->by[id] = ( b * vtb[p].y + a * vtb[q].y ) / midsum[p];
      }

    Splv *nlv = (lv->nxt != NULL ) ? lv->nxt : lp->splv;
    Sped *ed = find_ppdedge( lv->vt, nlv->vt );
    assert( ed != NULL );
    a += ed->length;
    
  }
  
  free( vtb );
  free( midsum );
  
}

// �ɂ����߂邽�߂̊֐�
void spm_lambda( Sppd *ppd )
{
  for (Spvt *vt = ppd->spvt; vt != NULL; vt = vt->nxt ) {

    // geodesic polar map
    ppdvertex_gpm( vt );

    // make lambda
    ppdvertex_spm_lambda( vt );
    
  }
}

//
// geodesic polar map functions
// in Welch & Witkin's SIGGRAPH 94 paper
//
// lambda: �}�g���b�N�X�Ɋi�[����p�����[�^
// vec2d:  ���n���Ɏʑ��̃p�����[�^�l (x, y)
// ����(n_degree)+1(���̒��_) �̐������m��
// ���_�܂��̔����v����̏���
//
void ppdvertex_gpm( Spvt *vt )
{
  // �ߖT�̒��_�܂��̊p�x���i�[
  double *angle;
  angle = (double *) malloc( vt->n_degree * sizeof(double) );

  // vt �����_�ɂ���
  vt->vec2d[0].x = 0.0;
  vt->vec2d[0].y = 0.0;

  // �p�x�̌v�Z
  Spvt *nvt, *pvt;
  Vec vec0, vec1;
  Sphe *he = vt->sphe;
  double nangle = 0.0;
  int n = 0;
  do {
    nvt = he->nxt->vt;
    pvt = he->prv->vt;
    V3Sub( &(nvt->vec), &(vt->vec), &vec0 );
    V3Sub( &(pvt->vec), &(vt->vec), &vec1 );
    angle[n] = acos( V3Cosign( &vec0, &vec1 ) );
    nangle += angle[n];
    ++n;
    he = ppdvertex_ccwnext_halfedge( he );
  } while ( (he != vt->sphe) && (he != NULL) );

  double mul = 2.0 * SMDPI / nangle;
  int i;
  for ( i = 0; i < n; ++i ) angle[i] *= mul;

  // �p�����[�^�̌v�Z
  Sped *ed = ppdvertex_first_edge( vt );
  Sped *fed = ed;
  double tangle = 0.0;
  i = 0;
  do {
    vt->vec2d[i+1].x = ed->length * cos( tangle );
    vt->vec2d[i+1].y = ed->length * sin( tangle );
    tangle += angle[i];
    ++i;
    ed = ppdvertex_next_edge( ed, vt );
  } while ( (ed != NULL) && (ed != fed) );

  free( angle );
}

//
// ���_�ߖT�̃ɂ�����
//
void ppdvertex_spm_lambda( Spvt *vt )
{
  double area;

  //
  // ������ 3 ���傫���Ƃ�
  //
  if ( vt->n_degree > 3 ) {

    int i;
    for ( i = 1; i <= vt->n_degree; ++i ) {
      vt->lambda[i] = 0.0;
    }

    for ( i = 1; i <= vt->n_degree; ++i ) {

      int j = 1;
      do {
	int v1 = j;
	int v2 = (j != vt->n_degree) ? j+1 : 1;
	if ( (v1 != i) && (v2 != i) ) {
	  // vt->vec2d[i], vt->vec2d[0] ��ʂ钼���ƁC
	  // ���� vt->vec2d[v1], vt->vec2d[v2] �̌�������
	  if ( isLeftSide(&(vt->vec2d[i]), &(vt->vec2d[0]), &(vt->vec2d[v1]))
	       != isLeftSide(&(vt->vec2d[i]), &(vt->vec2d[0]), &(vt->vec2d[v2])) ) {

	    // i, v1, v2 �̎O�_���Ώ�
	    area = V2TriArea( &(vt->vec2d[i]), &(vt->vec2d[v1]), &(vt->vec2d[v2]) );
	    vt->lambda[i]  += (V2TriArea( &(vt->vec2d[0]), &(vt->vec2d[v1]), &(vt->vec2d[v2]) )
			       / area);
	    vt->lambda[v1] += (V2TriArea( &(vt->vec2d[i]), &(vt->vec2d[0]), &(vt->vec2d[v2]) )
			       / area);
	    vt->lambda[v2] += (V2TriArea( &(vt->vec2d[i]), &(vt->vec2d[v1]), &(vt->vec2d[0]) )
			       / area);
	    break;
	  }
	}
	++j;
      } while ( j <= vt->n_degree );
      
    }

    for ( i = 1; i <= vt->n_degree; ++i )
      vt->lambda[i] /= (double) vt->n_degree;

    
  } else if ( vt->n_degree == 3 ) {
    // ������ 3 �̂Ƃ�
    area = V2TriArea( &(vt->vec2d[1]), &(vt->vec2d[2]), &(vt->vec2d[3]) );
    vt->lambda[1] = V2TriArea( &(vt->vec2d[0]), &(vt->vec2d[2]), &(vt->vec2d[3]) ) / area;
    vt->lambda[2] = V2TriArea( &(vt->vec2d[1]), &(vt->vec2d[0]), &(vt->vec2d[3]) ) / area;
    vt->lambda[3] = V2TriArea( &(vt->vec2d[1]), &(vt->vec2d[2]), &(vt->vec2d[0]) ) / area;
    
  }
  
  double sum=0.0;
  for ( int j = 1; j <= vt->n_degree; ++j ) {
    sum += vt->lambda[j];
  }
}

void solvec_ppdvertex( Sppd *ppd, Semat *emat )
{
  for ( Spvt *vt = ppd->spvt; vt != NULL; vt = vt->nxt ) {
    vt->uvw.x = emat->xx[ vt->sid ];
    vt->uvw.y = emat->yy[ vt->sid ];
  }
}

///////////////////////////////////////////////////////////////////////////
//
// shape-preserving parameterization (in [Floater97(CAGD)])
// for tile faces
//
///////////////////////////////////////////////////////////////////////////

void tfcspm( TFace *tfc )
{

  // ������
  tfcspm_initialize( tfc );
  
  // matrix creation process
  Semat *emat = create_tfcspm_emat( tfc );
  create_tfcspm_vector( tfc, emat );

  //printemat( emat );
  
  // iteration process
  int    iter;
  double rsq;
  (void) linbcg( emat, emat->bx, emat->xx, 1, SMDZEROEPS, 1000, &iter, &rsq );
  (void) linbcg( emat, emat->by, emat->yy, 1, SMDZEROEPS, 1000, &iter, &rsq );

  // vectors -> ppdvertex
  solvec_tvector( tfc, emat );

  //free emat, vtxvt
  free_emat( emat );

  tfcspm_finish( tfc );

  // parameterization flag
  tfc->isParameterized = TRUE;
}

// 
// ������
// 
void tfcspm_initialize( TFace *tfc )
{
  // ���_�ւ� tvector �̔ԍ��̖��ߍ���
  // �����̌���
  // lambda, vec2d �̗̈�m��
  TVector *tvec;
  for ( int i = 0; i < tfc->n_tvector; ++i ) {

    tvec = &(tfc->tvec[i]);
    Spvt *vt = tvec->vt;

    // ���_�ւ� tvector �̔ԍ��̖��ߍ���
    vt->tvec_id = i;

    // �����̌���
    tvec->n_degree = ppdvertex_num_edge( vt );
    
    // lambda, vec2d initialize
    tvec->vec2d  = (Vec2d *) malloc( (tvec->n_degree + 1) * sizeof(Vec2d) );
    tvec->lambda = (double *) malloc( (tvec->n_degree + 1) * sizeof(double) );
  }

}

Semat *create_tfcspm_emat( TFace *tfc )
{
  // �ɂ̌���
  tfcspm_lambda( tfc );

  //
  // �}�g���b�N�X�̍s���𐔂���
  //
  int i;
  int vn = 0;
  TVector *tvec;
  for ( i = 0; i < tfc->n_tvector; ++i ) {
    tvec = &(tfc->tvec[i]);
    if ( tvec->vt->sp_type != SP_VERTEX_NORMAL ) continue;
    vn += tvec->n_degree;
  }
  
  //
  // create indexed sparse matrix
  //
  Semat *emat = init_emat( tfc->n_tvector );
  emat->num = vn + tfc->n_tvector + 1;
  emat->sa  = (double *) malloc( emat->num * sizeof(double) );
  for ( i = 0; i < emat->num; ++i ) emat->sa[i] = 0.0;
  emat->ija = (int *) malloc( emat->num * sizeof(int) );
  
  //
  // indexed sparse matrix �e�s�̗v�f�ɒl���i�[����
  //
  emat->ija[0] = emat->cnum + 1;
  int j = emat->cnum;
  for ( i = 0; i < tfc->n_tvector; ++i ) {
    tvec = &(tfc->tvec[i]);
    Spvt *vt = tvec->vt;
    
    emat->sa[vt->tvec_id] = 1.0;

    // �������_�̂�
    if ( vt->sp_type == SP_VERTEX_NORMAL ) {

      Sped *ed = ppdvertex_first_edge( vt );
      Sped *fed = ed;
      int k = 1;
      do {
	++j;
	emat->sa[j]  = -1.0 * tvec->lambda[k];
	emat->ija[j] = another_vt( ed, vt )->tvec_id;
	++k;
	ed = ppdvertex_next_edge( ed, vt );
      } while ( (ed != NULL) && (ed != fed) );
    }
      
    emat->ija[vt->tvec_id + 1] = j+1;
  }
  
  return emat;
}

//
// �ɂ����߂邽�߂̊֐�
//
void tfcspm_lambda( TFace *tfc )
{
  for ( int i = 0; i < tfc->n_tvector; ++i ) {

    //Spvt *vt = tfc->tvec[i].vt;
    
    // geodesic polar map
    tvector_gpm( &(tfc->tvec[i]) );

    // make lambda
    tvector_spm_lambda( &(tfc->tvec[i]) );
    
  }
}

//
// geodesic polar map functions
// in Welch & Witkin's SIGGRAPH 94 paper
//
// lambda: �}�g���b�N�X�Ɋi�[����p�����[�^
// vec2d:  ���n���Ɏʑ��̃p�����[�^�l (x, y)
// ����(n_degree)+1(���̒��_) �̐������m��
// ���_�܂��̔����v����̏���
//
void tvector_gpm( TVector *tvec )
{
  // �ߖT�̒��_�܂��̊p�x���i�[
  double *angle;
  angle = (double *) malloc( tvec->n_degree * sizeof(double) );

  // vt �����_�ɂ���
  tvec->vec2d[0].x = 0.0;
  tvec->vec2d[0].y = 0.0;

  // �p�x�̌v�Z
  Spvt *vt = tvec->vt;
  Spvt *nvt, *pvt;
  Vec vec0, vec1;
  Sphe *he = vt->sphe;
  double nangle = 0.0;
  int n = 0;
  do {
    nvt = he->nxt->vt;
    pvt = he->prv->vt;
    V3Sub( &(nvt->vec), &(vt->vec), &vec0 );
    V3Sub( &(pvt->vec), &(vt->vec), &vec1 );
    angle[n] = acos( V3Cosign( &vec0, &vec1 ) );
    nangle += angle[n];
    ++n;
    he = ppdvertex_ccwnext_halfedge( he );
  } while ( (he != vt->sphe) && (he != NULL) );

  double mul = 2.0 * SMDPI / nangle;
  int i;
  for ( i = 0; i < n; ++i ) angle[i] *= mul;

  // �p�����[�^�̌v�Z
  Sped *ed = ppdvertex_first_edge( vt );
  Sped *fed = ed;
  double tangle = 0.0;
  i = 0;
  do {
    tvec->vec2d[i+1].x = ed->length * cos( tangle );
    tvec->vec2d[i+1].y = ed->length * sin( tangle );
    tangle += angle[i];
    ++i;
    ed = ppdvertex_next_edge( ed, vt );
  } while ( (ed != NULL) && (ed != fed) );

  free( angle );
}

//
// ���_�ߖT�̃ɂ�����
//
void tvector_spm_lambda( TVector *tvec )
{
  double area;
  
  // ������ 3 ���傫���Ƃ�
  if ( TRUE ) {

    int i;
    for ( i = 1; i <= tvec->n_degree; ++i ) {
      tvec->lambda[i] = 0.0;
    }

    for ( i = 1; i <= tvec->n_degree; ++i ) {

      int j = 1;
      do {
	int v1 = j;
	int v2 = (j != tvec->n_degree) ? j+1 : 1;
	if ( (v1 != i) && (v2 != i) ) {
	  // tvec->vec2d[i], tvec->vec2d[0] ��ʂ钼���ƁC
	  // ���� tvec->vec2d[v1], tvec->vec2d[v2] �̌�������
	 if ( isLeftSide(&(tvec->vec2d[i]), &(tvec->vec2d[0]), &(tvec->vec2d[v1]))
	       != isLeftSide(&(tvec->vec2d[i]), &(tvec->vec2d[0]), &(tvec->vec2d[v2])) ) {

	    // i, v1, v2 �̎O�_���Ώ�
	   area = V2TriArea( &(tvec->vec2d[i]), &(tvec->vec2d[v1]), &(tvec->vec2d[v2]) );
	   tvec->lambda[i] += (V2TriArea( &(tvec->vec2d[0]), &(tvec->vec2d[v1]), &(tvec->vec2d[v2]) ) / area);
	   tvec->lambda[v1] += (V2TriArea( &(tvec->vec2d[i]), &(tvec->vec2d[0]), &(tvec->vec2d[v2]) ) / area);
	   tvec->lambda[v2] += (V2TriArea( &(tvec->vec2d[i]), &(tvec->vec2d[v1]), &(tvec->vec2d[0]) ) / area);
	    break;
	  }
	}
	++j;
      } while ( j <= tvec->n_degree );
      
    }

    for ( i = 1; i <= tvec->n_degree; ++i )
      tvec->lambda[i] /= (double) tvec->n_degree;

    
  } else if ( tvec->n_degree == 3 ) {
    // ������ 3 �̂Ƃ�
     area = V2TriArea( &(tvec->vec2d[1]), &(tvec->vec2d[2]), &(tvec->vec2d[3]) );
     tvec->lambda[1] = V2TriArea( &(tvec->vec2d[0]), &(tvec->vec2d[2]), &(tvec->vec2d[3]) ) / area;
     tvec->lambda[2] = V2TriArea( &(tvec->vec2d[1]), &(tvec->vec2d[0]), &(tvec->vec2d[3]) ) / area;
     tvec->lambda[3] = V2TriArea( &(tvec->vec2d[1]), &(tvec->vec2d[2]), &(tvec->vec2d[0]) ) / area;
     
  }
}

//
// �������̃x�N�g������ (Ax = b �� b �����߂�֐�)
//
void create_tfcspm_vector( TFace *tfc, Semat *emat )
{
  // initialize vector
  int i;
  for ( i = 0; i < tfc->n_tvector; ++i ) {
    emat->bx[i] = emat->by[i] = 0.0;
    emat->xx[i] = emat->yy[i] = 0.0;
  }

  // outer loop �̃G�b�W�̒����̌v�Z
  Splp *lp = tfc->clp;
  Splv *lv;

  // ���[�v�̃G�b�W�����̑��a
  double sum = 0.0;
  for ( lv = lp->splv; lv != NULL; lv = lv->nxt )
    {
      Splv *nlv = (lv->nxt != NULL ) ? lv->nxt : lp->splv;
      Sped *ed = find_ppdedge( lv->vt, nlv->vt );
      assert( ed != NULL );
      sum += ed->length;
    }

  //
  // �p�̓_�� uv ���W�l�̌���
  //
  // lvn:  ���[�v�̒��_�̐�
  // tfc->n_tedge: ���[�v�̊p���_�̐�
  // ���[�v�̈�ԍŏ��̓_ (lp->splv) �́C�p�̓_
  //
  Vec2d* vtb = (Vec2d*) malloc( tfc->n_tedge * sizeof(Vec2d) );
  double *midsum = (double *) malloc( tfc->n_tedge * sizeof(double) );
  for ( i = 0; i < tfc->n_tedge; ++i ) midsum[i] = 0.0;
  int cnt = -1;
  double angle = 0;
  for ( lv = lp->splv; lv != NULL; lv = lv->nxt ) {

    if ( lv->vt->sp_type == SP_VERTEX_TVERTEX ) {
      ++cnt;
      int id = lv->vt->tvec_id;
      vtb[cnt].x = emat->bx[id] = RADIUS * cos(angle);
      vtb[cnt].y = emat->by[id] = RADIUS * sin(angle);
    }

    Splv *nlv = (lv->nxt != NULL ) ? lv->nxt : lp->splv;
    Sped *ed = find_ppdedge( lv->vt, nlv->vt );
    assert( ed != NULL );
    midsum[cnt] += ed->length;
    angle += (2 * SMDPI * ed->length / sum);
    
  }

  //
  // �p�ȊO�̋��E�� uv ���W�l�̌���
  //
  cnt = -1;
  double a = 0.0;
  for ( lv = lp->splv; lv != NULL; lv = lv->nxt ) {

    if ( lv->vt->sp_type == SP_VERTEX_TVERTEX )
      {
	++cnt;
	a = 0.0;
      }
    else
      {
	int p = cnt;
	int q = (cnt != tfc->n_tedge - 1) ? cnt+1 : 0;
	int id = lv->vt->tvec_id;
	double b = a / midsum[p];
	emat->bx[id] = vtb[p].x + b * (vtb[q].x - vtb[p].x);
	emat->by[id] = vtb[p].y + b * (vtb[q].y - vtb[p].y);
      }

    Splv *nlv = (lv->nxt != NULL ) ? lv->nxt : lp->splv;
    Sped *ed = find_ppdedge( lv->vt, nlv->vt );
    assert( ed != NULL );
    a += ed->length;
    
  }
  
  free( vtb );
  free( midsum );
}

void solvec_tvector( TFace *tfc, Semat *emat )
{
  for ( int i = 0; i < tfc->n_tvector; ++i ) {
    TVector *tvec = &(tfc->tvec[i]);
    //Spvt *vt = tvec->vt;
    tvec->uv.x = emat->xx[i];
    tvec->uv.y = emat->yy[i];
  }
}

//
// �I���葱
// 
void tfcspm_finish( TFace *tfc )
{
  // lambda, vec2d �̗̈�폜
  for ( int i = 0; i < tfc->n_tvector; ++i ) {
    TVector *tvec = &(tfc->tvec[i]);
    free( tvec->lambda );  tvec->lambda = NULL;
    free( tvec->vec2d );  tvec->vec2d = NULL;
  }
}

