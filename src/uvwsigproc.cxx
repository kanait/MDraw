//
// uvwsigproc.cxx
//
// Copyright (c) 1998 Takashi Kanai
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

#include "smd.h"
#include "tile.h"
#include "veclib.h"
#include "ppdvertex.h"

#include "spm.h"
#include "umbrella.h"
#include "uvwsigproc.h"

void uvwsigproc( Sppd *ppd, double lambda, double mu, int nrepeat, int type )
{
  //
  // k��̃t�F�A�����O�X�e�b�v
  //
  double ans  = 0.0;
  int k = 0;
  while ( TRUE )
    {
      switch ( type )
	{
	case SP_MEMBRANE:
	  ans = param_membrane( ppd );
	  break;
	case SP_EWEIGHT:
	  ans = param_eweight( ppd );
	  break;
	case SP_FWEIGHT:
	  ans = param_fweight( ppd );
	  break;
	case SP_TPWEIGHT:
	  ans = param_tpweight( ppd );
	  break;
	case SP_HMWEIGHT:
	  ans = param_hmweight( ppd );
	  break;
	}

      if ( ans < 1.0e-04 ) return;
      
      ++k;
    }
}

void tfcparam( TFace *tfc, int type )
{
  //
  // k��̃t�F�A�����O�X�e�b�v
  //
  
  display("fairing type %d\n", type );
  int i;
  // initialize
  // tvec->uv ���� vt->uvw �փR�s�[ (���Ƃ� ppd �̐ڑ������g������)
  for ( i = 0; i < tfc->n_tvector; ++i ) {
    TVector *tvec = &(tfc->tvec[i]);
    Spvt *vt = tvec->vt;
    vt->uvw.x = tvec->uv.x;
    vt->uvw.y = tvec->uv.y;
    display("before param vt %d sp_type %d uv %g %g\n", i, vt->sp_type, 
	    tfc->tvec[i].uv.x, tfc->tvec[i].uv.y );
  }
  double ans  = 0.0;
  int k = 0;
  while ( TRUE )
    {
      switch ( type )
	{
	case SP_MEMBRANE:
	  ans = tfcparam_membrane( tfc );
	  break;
	case SP_EWEIGHT:
	  ans = tfcparam_eweight( tfc );
	  break;
	case SP_FWEIGHT:
	  ans = tfcparam_fweight( tfc );
	  break;
	case SP_TPWEIGHT:
	  ans = tfcparam_tpweight( tfc );
	  break;
	case SP_HMWEIGHT:
	  ans = tfcparam_hmweight( tfc );
	  break;
	}

      if ( ans < 1.0e-04 )
	{
	  break; // while ���𔲂���
	}
      
      ++k;
      
    }

  // vt->uvw ���� tvec->uv �փR�s�[
  for ( i = 0; i < tfc->n_tvector; ++i )
    {
      TVector *tvec = &(tfc->tvec[i]);
      Spvt *vt = tvec->vt;
      tvec->uv.x = vt->uvw.x;
      tvec->uv.y = vt->uvw.y;
      display("after param vt %d sp_type %d uv %g %g\n", i, vt->sp_type, 
	      tfc->tvec[i].uv.x, tfc->tvec[i].uv.y );
    }
}

//
// ���̒e���G�l���M�[(membrane energy) �̍ŏ����ɂ�镽����
//
// Kobbelt �_�� (SG98) �� umbrella-algorithm ���g���ĉ����Ă���
// Taubin �̏d�S�@ (�ł��ȒP�ȗ��U���v���V�A��, SG95) �ƈ�v
//
double tfcparam_membrane( TFace* tfc )
{
  Vec2d *update = (Vec2d *) malloc( tfc->n_tvector * sizeof(Vec2d) );
  
  int i;
  Spvt *vt;
  for ( i = 0; i < tfc->n_tvector; ++i ) 
    {
      TVector *tvec = &(tfc->tvec[i]);
      vt = tvec->vt;
      if ( vt->sp_type == SP_VERTEX_NORMAL ) // �������_�Ɋւ��鏈��
	{
	  //
	  // umbrella-operator
	  Vec2d d;
	  tfc_umbrella_operator( vt, &d );
	  //uvw_sd_umbrella_operator( vt, &d );
	  
	  // update vertex coordinate
	  update[i].x = vt->uvw.x + d.x;
	  update[i].y = vt->uvw.y + d.y;
	}
      else  // ���E���_�Ɋւ��鏈��
	{
	  update[i].x = vt->uvw.x;
	  update[i].y = vt->uvw.y;
	}
    }
  // �l�̍X�V
  double ans = 0.0;
  for ( i = 0; i < tfc->n_tvector; ++i )
    {
      TVector *tvec = &(tfc->tvec[i]);
      vt = tvec->vt;
      ans += V2DistanceBetween2Points( &(vt->uvw), &(update[i]) );
      vt->uvw.x = update[i].x;
      vt->uvw.y = update[i].y;
    }
  ans /= (double) tfc->n_tvector;

  free( update );

  return ans;
}

//
// umbrella-operator �̏d�݂̌v�Z�ɃG�b�W�̒������g���Ă���
// Taubin �_�� (SG95) �ɓo��
//
// Kobbelt �_�� (SG98) �� umbrella-algorithm ���g���ĉ����Ă���
//
double tfcparam_eweight( TFace* tfc )
{
  Vec2d *update = (Vec2d *) malloc( tfc->n_tvector * sizeof(Vec2d) );
  
  int i;
  Spvt *vt;
  for ( i = 0; i < tfc->n_tvector; ++i )
    {
      TVector *tvec = &(tfc->tvec[i]);
      vt = tvec->vt;
      if ( vt->sp_type == SP_VERTEX_NORMAL ) // �������_�Ɋւ��鏈��
	{
	  // vt �̗אڒ��_�ɑ΂��鏈��
	  double sum = 0.0;
	  Vted *vted = NULL;
	  for ( vted = vt->svted; vted != NULL; vted = vted->nxt )
	    {
	      sum += ( vted->ed->length );
	    }
	  
	  Vec2d d; d.x = 0.0; d.y = 0.0;
	  for ( vted = vt->svted; vted != NULL; vted = vted->nxt )
	    {
	      double frac = ( vted->ed->length ) / sum;
	      Spvt *avt = another_vt( vted->ed, vt );
	      d.x += (frac * avt->uvw.x);
	      d.y += (frac * avt->uvw.y);
	    }
	  d.x -= vt->uvw.x;
	  d.y -= vt->uvw.y;
	  update[i].x = vt->uvw.x + d.x;
	  update[i].y = vt->uvw.y + d.y;
	}
      else  // ���E���_�Ɋւ��鏈��
	{
	  update[i].x = vt->uvw.x;
	  update[i].y = vt->uvw.y;
	}
    }
  // �l�̍X�V
  double ans = 0.0;
  for ( i = 0; i < tfc->n_tvector; ++i ) 
    {
      TVector *tvec = &(tfc->tvec[i]);
      vt = tvec->vt;
      ans += V2DistanceBetween2Points( &(vt->uvw), &(update[i]) );
      vt->uvw.x = update[i].x;
      vt->uvw.y = update[i].y;
    }
  ans /= (double) tfc->n_tvector;
  
  free( update );
  
  return ans;
}

//
// umbrella-operator �̏d�݂̌v�Z�ɗאږʂ̖ʐς��g���Ă���
// Taubin �_�� (SG95) �ɓo��
//
// Kobbelt �_�� (SG98) �� umbrella-algorithm ���g���ĉ����Ă���
//
double tfcparam_fweight( TFace* tfc )
{
  Vec2d *update = (Vec2d *) malloc( tfc->n_tvector * sizeof(Vec2d) );
  
  int i;
  Spvt *vt;
  for ( i = 0; i < tfc->n_tvector; ++i )
    {
      TVector *tvec = &(tfc->tvec[i]);
      vt = tvec->vt;
      if ( vt->sp_type == SP_VERTEX_NORMAL ) // �������_�Ɋւ��鏈��
	{
	  // vt �̗אڒ��_�ɑ΂��鏈��
	  double sum = 0.0;
	  Vted *vted = NULL;
	  for ( vted = vt->svted; vted != NULL; vted = vted->nxt )
	    {
	      double val = 0.0;
	      val += ( vted->ed->lhe->bpfc->area / 2.0 );
	      val += ( vted->ed->rhe->bpfc->area / 2.0 );
	      sum += val;
	    }
	  
	  Vec2d d; d.x = 0.0; d.y = 0.0;
	  for ( vted = vt->svted; vted != NULL; vted = vted->nxt )
	    {
	      double val = 0.0;
	      val += ( vted->ed->lhe->bpfc->area / 2.0 );
	      val += ( vted->ed->rhe->bpfc->area / 2.0 );
	      double frac = val / sum;
	      Spvt *avt = another_vt( vted->ed, vt );
	      d.x += (frac * avt->uvw.x);
	      d.y += (frac * avt->uvw.y);
	    }
	  d.x -= vt->uvw.x;
	  d.y -= vt->uvw.y;
	  update[i].x = vt->uvw.x + d.x;
	  update[i].y = vt->uvw.y + d.y;
	}
      else  // ���E���_�Ɋւ��鏈��
	{
	  update[i].x = vt->uvw.x;
	  update[i].y = vt->uvw.y;
	}
    }
  // �l�̍X�V
  double ans = 0.0;
  for ( i = 0; i < tfc->n_tvector; ++i )
    {
      TVector *tvec = &(tfc->tvec[i]);
      vt = tvec->vt;
      ans += V2DistanceBetween2Points( &(vt->uvw), &(update[i]) );
      vt->uvw.x = update[i].x;
      vt->uvw.y = update[i].y;
    }
  ans /= (double) tfc->n_tvector;

  free( update );

  return ans;
}

//
// thin plate energy �ɂ�闣�U���v���V�A���t�F�A�����O
// ( in Kobbelt's SG98 paper )
//
// Kobbelt �_�� (SG98) �� umbrella-algorithm ���g���ĉ����Ă���
//
double tfcparam_tpweight( TFace* tfc )
{
  Vec2d *update = (Vec2d *) malloc( tfc->n_tvector * sizeof(Vec2d) );
  
  int i;
  Spvt *vt;
  for ( i = 0; i < tfc->n_tvector; ++i )
    {
      TVector *tvec = &(tfc->tvec[i]);
      vt = tvec->vt;
      if ( vt->sp_type == SP_VERTEX_NORMAL ) // �������_�Ɋւ��鏈��
	{
	  //
	  // (umbrella-operator)^2
	  //
	  double frac = 1.0 / (double) tfc_num_neighbor( vt );
//  	  display("ven %d neighbor %d\n", vt->ven, tfc_num_neighbor( vt ) );
	  // for calculating diagonal element
	  double fracj = 0.0; 
	  Vec2d dd; dd.x = dd.y = 0.0;
	  // vt �͓����̓_�̂�
	  for ( Vted *vted = vt->svted; vted != NULL; vted = vted->nxt )
	    {
	      // avt �͋��E�̓_��������Ȃ�
	      Spvt *avt = another_vt( vted->ed, vt );
	      Vec2d da;
	      tfc_umbrella_operator_bfree( avt, &da );
	      if ( avt->tile_id == vt->tile_id )
		{
		  dd.x += (frac * da.x);
		  dd.y += (frac * da.y);
		}
		  
	      // for calculating diagonal element
	      fracj += (1.0 / (double) tfc_num_neighbor( avt ));
//  	      display("\taven %d aneighbor %d\n", avt->ven, tfc_num_neighbor( avt ) );
	    }
	  Vec2d dp;
	  tfc_umbrella_operator_bfree( vt, &dp );
	  dd.x -= dp.x;
	  dd.y -= dp.y;

	  // diagonal element
	  double mu = 1.0 + frac * fracj;
	  // update vertex coordinate
	  update[i].x = vt->uvw.x - dd.x / mu;
	  update[i].y = vt->uvw.y - dd.y / mu;
	}
      else  // ���E���_�Ɋւ��鏈��
	{
	  update[i].x = vt->uvw.x;
	  update[i].y = vt->uvw.y;
	}
    }
  // �l�̍X�V
  double ans = 0.0;
  for ( i = 0; i < tfc->n_tvector; ++i )
    {
      TVector *tvec = &(tfc->tvec[i]);
      vt = tvec->vt;
      ans += V2DistanceBetween2Points( &(vt->uvw), &(update[i]) );
      vt->uvw.x = update[i].x;
      vt->uvw.y = update[i].y;
    }
  ans /= (double) tfc->n_tvector;

  free( update );

  return ans;
}

//
// harmonic mapping �̏d�݂��g�������U���v���V�A���t�F�A�����O
// Eck95 �̌W��
//
// Kobbelt �_�� (SG98) �� umbrella-algorithm ���g���ĉ����Ă���
//
double tfcparam_hmweight( TFace* tfc )
{
  Vec2d *update = (Vec2d *) malloc( tfc->n_tvector * sizeof(Vec2d) );
  
  int i;
  Spvt *vt;
  for ( i = 0; i < tfc->n_tvector; ++i ) 
    {
      TVector *tvec = &(tfc->tvec[i]);
      vt = tvec->vt;
      if ( vt->sp_type == SP_VERTEX_NORMAL ) // �������_�Ɋւ��鏈��
	{
	  // �אڂ̃G�b�W�ɑ΂���d��
	  double *w = (double *) malloc( vt->ven * sizeof(double) );
	  
	  // vt �̗אڒ��_�ɑ΂��鏈��
	  double sum = 0.0;
	  int j = 0;
	  Vted *vted = NULL;
	  for ( vted = vt->svted; vted != NULL; vted = vted->nxt )
	    {
	      w[j] = param_hmw( vted->ed );
	      sum += w[j];
	      ++j;
	    }
	  
	  Vec2d d; d.x = 0.0; d.y = 0.0;
	  j = 0;
	  for ( vted = vt->svted; vted != NULL; vted = vted->nxt )
	    {
	      double frac = w[j] / sum;
	      Spvt *avt = another_vt( vted->ed, vt );
	      d.x += (frac * avt->uvw.x);
	      d.y += (frac * avt->uvw.y);
	      ++j;
	    }
	  d.x -= vt->uvw.x;
	  d.y -= vt->uvw.y;
	  update[i].x = vt->uvw.x + d.x;
	  update[i].y = vt->uvw.y + d.y;

	  free( w );
	}
      else  // ���E���_�Ɋւ��鏈��
	{
	  update[i].x = vt->uvw.x;
	  update[i].y = vt->uvw.y;
	}
    }
  // �l�̍X�V
  double ans = 0.0;
  for ( i = 0; i < tfc->n_tvector; ++i ) 
    {
      TVector *tvec = &(tfc->tvec[i]);
      vt = tvec->vt;
      ans += V2DistanceBetween2Points( &(vt->uvw), &(update[i]) );
      vt->uvw.x = update[i].x;
      vt->uvw.y = update[i].y;
    }
  ans /= (double) tfc->n_tvector;

  free( update );

  return ans;
}


