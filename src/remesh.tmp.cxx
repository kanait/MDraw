//
// remesh.cxx
// 
// Copyright (c) 1999 Takashi Kanai; All rights reserved. 
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

#include "ppd.h"
#include "ppdsolid.h"
#include "ppdpart.h"
#include "ppdlist.h"
#include "ppdvertex.h"
#include "ppdface.h"
#include "ppdedge.h"
#include "ppdloop.h"
#include "tile.h"
#include "veclib.h"
#include "matlib.h"
#include "linbcg.h"
#include "intsec.h"
#include "file.h"
#include "mytime.h"

#include "esp.h"

#include "remesh.h"

#if defined(_DEBUG)
// ���n����`�p
Sppd* tmpppd = NULL;
#endif

//
// grid mesh for ppd
//
Sppd *tolatticeppd( Sppd *ppd, int div )
{
  if ( ppd == NULL ) return NULL;
  if ( ppd->splp == NULL ) return NULL;
  if ( ppd->splp->lcvn != RECTANGLE ) return NULL;

#if defined(_DEBUG)
  tmpppd = ppd;
#endif
  
  // ppdvertex array
  int vn = (div + 1) * (div + 1);
  Spvt **pvt  = (Spvt **) malloc( vn * sizeof(Spvt *) );
  
  //
  // Lattice �^ remesh �̐����C���_�z��C�ʔz��ւ̊i�[
  //
  Sppd *newppd = remL_init( div, pvt );
  
  //
  // �p�̓_�̍��W�l�̌���
  //
  Vec2d uvcorner[RECTANGLE];
  remL_corner( ppd->splp, uvcorner, div, pvt );
  
  //
  // ���E��̓_�i�p���̂����j�̍��W�l�̌���
  //
  remL_boundary( ppd->splp, uvcorner, div, pvt );
  
  //
  // �����_�i���E���̂����j�̍��W�l�̌��� (�␳�Ȃ�)
  //
  remL_internal( ppd->splp, uvcorner, div, pvt );

  //
  // �����_�i���E���̂����j�̍��W�l�̃p�����[�^�␳
  //
  // �I���W�i���̃p�����[�^
  Vec2d* ouv  = (Vec2d*) malloc( vn * sizeof(Vec2d) );
  // �␳���̃p�����[�^�C�x�N�g��
  Vec2d* duv  = (Vec2d*) malloc( vn * sizeof(Vec2d) );
  Vec*   dvec = (Vec*) malloc( vn * sizeof(Vec) );
  for ( int k = 0; k < vn; ++k )
    {
      ouv[k].x = duv[k].x = pvt[k]->uvw.x;
      ouv[k].y = duv[k].y = pvt[k]->uvw.y;
      dvec[k].x = pvt[k]->vec.x;
      dvec[k].y = pvt[k]->vec.y;
      dvec[k].z = pvt[k]->vec.z;

    }

  // �␳
  remL_internal_update( ppd->splp, uvcorner, div, duv, dvec, DIFF_GEODIS );

  // ��U�A�b�v�f�[�g
  for ( int i = 0; i < vn; ++i )
    {
      pvt[i]->uvw.x = duv[i].x;
      pvt[i]->uvw.y = duv[i].y;
      pvt[i]->vec.x = dvec[i].x;
      pvt[i]->vec.y = dvec[i].y;
      pvt[i]->vec.z = dvec[i].z;

      // ��̍ăp�����[�^���Ŏg��
      pvt[i]->tvec_id = i;
    }

  //
  // ppd �̃p�����[�^�̍ăp�����[�^��
  // newppd �̃p�����[�^���Cvt->uvw ���� ouv �ɕό`�����Ƃ��� ppd �̃p�����[�^
  // �̕ϕ������߂�
  //
  remL_reparam( ppd, newppd, ouv );
  
  free( ouv );
  free( duv );
  free( dvec );
  
  free( pvt );
  
  return newppd;
}

//
// Lattice �^ remesh �̐����C���_�z��ւ̊i�[
//
Sppd* remL_init( int div, Spvt** pvt )
{
  // ppd, ppdsolid, ppdpart
  Sppd *ppd = create_ppd();
  Spso *newso = create_ppdsolid( ppd );
  Sppt *newpt = create_ppdpart( ppd );

  int i;
  // ppdvertex
  int vn = (div + 1) * (div + 1);
  for ( i = 0; i < vn; ++i )
    {
      pvt[i] = create_ppdvertex( ppd );
      pvt[i]->vec.x = 0.0;
      pvt[i]->vec.y = 0.0;
      pvt[i]->vec.z = 0.0;
      pvt[i]->bpso = newso;
    }

  // ppdface array
  int fn = div * div * 2;
  for ( i = 0; i < fn; ++i )
    {
      Spfc *fc = create_ppdface( ppd );
      fc->bpso = newso;
      fc->bppt = newpt;
//        pfc[i] = create_ppdface( ppd );
//        pfc[i]->bpso = newso;
//        pfc[i]->bppt = newpt;
    }

  // vertex-face connectivity
  Spfc *fc = ppd->spfc;
  for ( i = 0; i < div; ++i )
    {
      for ( int j = 0; j < div; ++j )
	{
	  Spvt* v1 = pvt[i * (div + 1) + j];
	  Spvt* v2 = pvt[i * (div + 1) + j + 1];
	  Spvt* v3 = pvt[(i + 1) * (div + 1) + j];
	  Spvt* v4 = pvt[(i + 1) * (div + 1) + j + 1];
//  	  f1 = pfc[fid]; ++fid;
//  	  f2 = pfc[fid]; ++fid;
	  Spfc* f1 = fc; fc = fc->nxt;
	  Spfc* f2 = fc; fc = fc->nxt;
      
	  // create halfedge
	  Sphe *he;
	  he = create_ppdhalfedge(f1); he->vt = v1;
	  he = create_ppdhalfedge(f1); he->vt = v3;
	  he = create_ppdhalfedge(f1); he->vt = v2;
	  he = create_ppdhalfedge(f2); he->vt = v2;
	  he = create_ppdhalfedge(f2); he->vt = v3;
	  he = create_ppdhalfedge(f2); he->vt = v4;
	  
	}
    }

  // make edges
  ppd_make_edges( ppd );
  
  //
  // set boundary attributes
  calc_ppd_boundary( ppd );
	
  //
  // re-attaching ppdvertex to halfedge pointer.
  // This function must be executed with ``calc_ppd_boundary''.
  reattach_ppdvertex_halfedge( ppd );

  return ppd;
}
  

//
// Lattice �^ remesh �ɂ���������_�̂R�������W�l�̌���
//
void remL_internal( Splp* lp, Vec2d* uvcorner, int div, Spvt **pvt )
{
  // �p�����[�^��ԏ�ł̋��E��̎n�_�ƏI�_�̊i�[
  Vec2d *uvstart = (Vec2d *) malloc( (div+1) * sizeof(Vec2d) );
  Vec2d *uvend   = (Vec2d *) malloc( (div+1) * sizeof(Vec2d) );
  Vec2d subs, sube;
  subs.x = uvcorner[1].x - uvcorner[0].x;
  subs.y = uvcorner[1].y - uvcorner[0].y;
  sube.x = uvcorner[2].x - uvcorner[3].x;
  sube.y = uvcorner[2].y - uvcorner[3].y;
  double ddiv = (double) div;
  int i;
  for ( i = 0; i <= div; ++i )
    {
      uvstart[i].x = uvcorner[0].x + (double) i * subs.x / ddiv;
      uvstart[i].y = uvcorner[0].y + (double) i * subs.y / ddiv;
      uvend[i].x   = uvcorner[3].x + (double) i * sube.x / ddiv;
      uvend[i].y   = uvcorner[3].y + (double) i * sube.y / ddiv;
    }

  // �n�_�ƏI�_�����Ԓ������T��
  for ( i = 1; i < div; ++i )
    {

      // uvstart[i].x, uvstart[i].y �̓����Ă��鋫�E�G�b�W��������
      Sped *ed = ppd_find_boundaryedge( lp, uvstart[i].x, uvstart[i].y );
      assert( ed != NULL );

      // ���E�G�b�W�ɗאڂ���ʂ�T��
      Spfc *fc = NULL;
      if ( ed->lhe != NULL ) fc = ed->lhe->bpfc;
      else if ( ed->rhe != NULL ) fc = ed->rhe->bpfc;
      assert( fc != NULL );

      // xparam, yparam �̓����Ă���i�q�_�̎O�������W�l�����߂�
      Vec sub;
      sub.x = uvend[i].x - uvstart[i].x;
      sub.y = uvend[i].y - uvstart[i].y;
      for ( int j = 1; j < div; ++j )
	{

	  int n = i * (div + 1) + j;
	  double xparam = uvstart[i].x + (double) j * sub.x / ddiv;
	  double yparam = uvstart[i].y + (double) j * sub.y / ddiv;

	  // point location algorithm ?
	  // xparam, yparam �̓����Ă���ʂ�������
	  while ( TRUE )
	    {
	      // xparam, yparam ���� fc �̒��ɓ����Ă���
	      if ( isParamsInFace( fc, xparam, yparam ) == TRUE )
		{
	  
		  // ���_�̍��W�l���v�Z
		  double a1, a2, a3;
		  ppdface_find_barycentric_coordinate( fc, xparam, yparam,
						       &a1, &a2, &a3 );
		  ppdface_barycentric_coordinate( fc, a1, a2, a3, &(pvt[n]->vec) );

		  // �p�����[�^���W�̐ݒ�
		  pvt[n]->uvw.x = xparam;
		  pvt[n]->uvw.y = yparam;

		  break;
	  
		}
	      else // ���̖ʂɂ͂Ȃ��̂Ŏ��̖ʂ�
		{ 
	  
		  // �ʂ̃G�b�W�̂����Ced �ł͂Ȃ� uvstart, uvend �ɂ���Ē�`�����
		  // �����ƌ����G�b�W��T��
		  ed = ppd_find_nextedge( fc, ed, &(uvstart[i]), &(uvend[i]) );
		  assert( ed != NULL );
		
		  // fc �̔��Α��̖�
		  fc = ppdedge_another_fc( ed, fc );
		  assert( fc != NULL );
		}
	    }
	}
    }

  free( uvstart );
  free( uvend );
}

#define XPARAM 0
#define YPARAM 1

//
// Lattice �^ remesh �ɂ���������_�̂R�������W�l�̑��n�������ɂ��A�b�v�f�[�g
//
void remL_internal_update( Splp* lp, Vec2d* uvcorner, int div,
  			   Vec2d* ouv, Vec* ovec, // �␳���̃p�����[�^�C�x�N�g��
			   int type )
{
  // �p�����[�^���␳
  int cnt = 0;
//    for ( int l = 0; l < 2; ++l )
  while ( TRUE )
    {
      ++cnt;
      double valx, valy;
      valx = uv_isoparam_diff( lp, div, uvcorner, ouv, ovec, type, XPARAM );
      valy = uv_isoparam_diff( lp, div, uvcorner, ouv, ovec, type, YPARAM );
      
      if ( (valx < 1.0e-05) && (valy < 1.0e-05) )
	{
	  fprintf(stderr, "iteration %02d\n", cnt );
	  break;
	}
    }

}

//
// Lattice �^ remesh �ɂ����鋫�E�_�̂R�������W�l�̌���
//
void remL_boundary( Splp* lp, Vec2d* uvcorner, int div, Spvt** pvt )
{
  int u = 0;
  int i;
  for ( i = 0; i < RECTANGLE; ++i )
    {
      // �P�ӂ̎n�_�C�I�_���߂�
      Vec2d *s = NULL, *e = NULL;
      if ( i == 0 )
	{
	  u = 0;
	  s = &(uvcorner[0]);
	  e = &(uvcorner[1]);
	}
      else if ( i == 1 )
	{
	  u = div;
	  s = &(uvcorner[3]);
	  e = &(uvcorner[2]);
	}
      else if ( i == 2 )
	{
	  u = 0;
	  s = &(uvcorner[0]);
	  e = &(uvcorner[3]);
	}
      else if ( i == 3 )
	{
	  u = div;
	  s = &(uvcorner[1]);
	  e = &(uvcorner[2]);
	}

      Vec2d sub;
      sub.x = e->x - s->x;
      sub.y = e->y - s->y;

      //
      // ���E�̓����_�ɂ�������W�l�̌v�Z
      //
      double ddiv = (double) div;
      for ( int j = 1; j < div; ++j )
	{

	  int n;
	  if ( i < 2 )
	    {
	      n = j * (div + 1) + u;
	    } else {
	      n = u * (div + 1) + j;
	    }
      
	  double xparam = s->x + (double) j * sub.x / ddiv;
	  double yparam = s->y + (double) j * sub.y / ddiv;

//  	  printf("param %g %g\n", xparam, yparam );
	  Sped *ed = ppd_find_boundaryedge( lp, xparam, yparam );
	  assert( ed != NULL );
	  Spfc *fc = NULL;
	  if ( ed->lhe != NULL ) fc = ed->lhe->bpfc;
	  else if ( ed->rhe != NULL ) fc = ed->rhe->bpfc;
	  assert( fc != NULL );
	  double a1, a2, a3;
	  ppdface_find_barycentric_coordinate( fc, xparam, yparam,
					       &a1, &a2, &a3 );
	  ppdface_barycentric_coordinate( fc, a1, a2, a3, &(pvt[n]->vec) );

	  // �p�����[�^���W�̐ݒ�
	  pvt[n]->uvw.x = xparam;
	  pvt[n]->uvw.y = yparam;
	}
    }
}
  
//
// Lattice �^ remesh �ɂ�����p�̓_�̂R�������W�l�̌���
//
void remL_corner( Splp* lp, Vec2d* uvcorner, int div, Spvt** pvt )
{
  //
  // �p�̓_�̍��W�l�̌���
  //
  int nn[RECTANGLE];
  nn[0] = 0;
  nn[1] = div * (div + 1);
  nn[2] = (div + 1) * (div + 1) - 1;
  nn[3] = div;
  
  // �p�̃p�����[�^�l���i�[
  int cnt = -1;
  for ( Splv *lv = lp->splv; lv != NULL; lv = lv->nxt )
    {
      if ( lv->type == LOOPVT_CORNER )
	{
	  ++cnt;
	  Spvt *vt = lv->vt;

	  uvcorner[cnt].x = vt->uvw.x;
	  uvcorner[cnt].y = vt->uvw.y;
	  //printf("corner[%d] %g %g\n", cnt, uvcorner[cnt].x, uvcorner[cnt].y );

	  // ���W�l�̌���
	  pvt[nn[cnt]]->vec.x = vt->vec.x;
	  pvt[nn[cnt]]->vec.y = vt->vec.y;
	  pvt[nn[cnt]]->vec.z = vt->vec.z;

	  // �p�����[�^���W�̐ݒ�
	  pvt[nn[cnt]]->uvw.x = uvcorner[cnt].x;
	  pvt[nn[cnt]]->uvw.y = uvcorner[cnt].y;
	}
    }
}

//
// �p�����[�^���� start, end �Ԃ̑��n�������̌v�Z
//
double uv_geodesic_distance_old( Vec2d* start, Vec2d* end, Sped* edS, Spfc* fcS )
{
//    display("geodesic\n");
  // �ŏ��̃G�b�W�ƒT����
  Sped* ed = edS; if ( ed == NULL ) return 0;
  Spfc* fc = fcS; if ( fc == NULL ) return 0;

  // ��_�̃p�����[�^���W
  Vec2d uv; V2Equal( &uv, start );
  // ��_�̎O�������W
  Vec pos; ppdface_uv_to_coord( fc, &uv, &pos );

  // �m���Ɍ�_���������߂�end �����ւ��������΂�
  Vec2d endL;
  double val = 1.01;
  endL.x = start->x + val * (end->x - start->x );
  endL.y = start->y + val * (end->y - start->y );

  // ���n������
  double distance = 0.0;

  // �T���ʂ��Ȃ��Ȃ�܂ő�����
  while ( TRUE )
    {
//        display("\ted %d %d fc %d\n", ed->no, ed->isBoundary, fc->no );
      // �ʂ̃G�b�W�̂����Ced �ł͂Ȃ� uvstart, uvend �ɂ���Ē�`�����
      // �����ƌ����G�b�W��T�� (�p�����[�^��_��)
      ed = ppd_find_nextedge_intsec( fc, ed, start, &endL, &uv );
      assert( ed != NULL );

      // �p�����[�^��_�̍��W�l�ւ̕ϊ�
      Vec intsec;
      ppdface_uv_to_coord( fc, &uv, &intsec );

//        display("\t+%g\n", V3DistanceBetween2Points( &pos, &intsec ) );
      distance += V3DistanceBetween2Points( &pos, &intsec );

      // �G�b�W�����E�G�b�W�̂Ƃ��ɂ̓��[�v�𔲂���
      if ( ed->isBoundary == TRUE ) break;

      // ���W�̃A�b�v�f�[�g
      V3Equal( &pos, &intsec );
      
      // fc �̔��Α��̖�
      fc = ppdedge_another_fc( ed, fc );
    }

  return distance;
}

#define MUL 1000

//
// �p�����[�^���� start, end �Ԃ̑��n�������̌v�Z
//
double uv_geodesic_distance( Sped* edS, Spfc* fcS, int div, int q, Vec2d* ouv,
			     int type )
{
//    display("geodesic\n");
  // �ŏ��̃G�b�W�ƒT����
  
  Sped* ed = edS; if ( ed == NULL ) return 0;
  Spfc* fc = fcS; if ( fc == NULL ) return 0;

#if defined(_DEBUG)
  // ���n����`�p���[�v
  Splp *lp = create_splp();
#endif
  
  // ��_�̃p�����[�^���W
  // ���E�� (j = 0) 
  int n = ( type == XPARAM ) ? q * (div + 1) : q;
  Vec2d uv; V2Equal( &uv, &(ouv[n]) );
  // ��_�̎O�������W
  Vec pos; ppdface_uv_to_coord( fc, &uv, &pos );

#if defined(_DEBUG)
  Splv *lplv; Spvt *lpvt;
  lplv = create_ppdloopvertex( lp );
  lpvt = create_ppdvertex( tmpppd );
  lpvt->vec.x = pos.x;
  lpvt->vec.y = pos.y;
  lpvt->vec.z = pos.z;
  lplv->vt = lpvt;
#endif

  // ���n������
  double distance = 0.0;

#if defined(_DEBUG)
  double tmpdis = 0.0;
#endif

  // TEMP
//    for ( int j = 0; j <= div; ++j ) {
//      Vec2d nuv;
//      int nt = ( type == XPARAM ) ? (q * (div + 1) + j) : (j * (div + 1) + q);
//      V2Equal( &(nuv), &(ouv[nt]) );
//      display("--------------- nuv %g %g ---------------\n", nuv.x, nuv.y );
//    }
  // TEMP
  
  // �T���ʂ��Ȃ��Ȃ�܂ő�����
  for ( int j = 1; j <= div; ++j )
    {
      Vec2d nuv;
      int n1 = ( type == XPARAM ) ? (q * (div + 1) + j) : (j * (div + 1) + q);
      V2Equal( &(nuv), &(ouv[n1]) );
      
      // ����������m���ɍs�����߂ɂ��������΂�
      Vec2d start, end;
//        start.x = uv.x - MUL * ( nuv.x - uv.x );
//        start.y = uv.y - MUL * ( nuv.y - uv.y );
      start.x = uv.x;
      start.y = uv.y;
      end.x   = uv.x + MUL * ( nuv.x - uv.x );
      end.y   = uv.y + MUL * ( nuv.y - uv.y );

//        display("*************** nuv %g %g ***************\n", nuv.x, nuv.y );
      // �p�����[�^ uv ���� nuv �܂ł̑��n�����������߂�
      double disL = V2DistanceBetween2Points( &uv, &nuv );
//        display("disL(created) %g\n", disL );
      while ( TRUE )
	{
  	  //display("\tj %d ed %d %d fc %d\n", j, ed->no, ed->isBoundary, fc->no );
	  
	  // �ʂ̃G�b�W�̂����Ced �ł͂Ȃ� uv, nuv �ɂ���Ē�`�����
	  // �����ƌ����G�b�W��T�� (�p�����[�^��_��)
	  Vec2d intsec2;
	  Sped* ned = ppd_find_nextedge_intsec( fc, ed, &start, &end, &intsec2 );
	  assert( ned != NULL );

	  // �p�����[�^��_�̍��W�l�ւ̕ϊ�
	  Vec intsec;
	  ppdface_uv_to_coord( fc, &intsec2, &intsec );

	  double pdis = V2DistanceBetween2Points( &uv, &intsec2 );

//  	  display("disL %g pdis %g\n", disL, pdis );
	  
	  if ( disL > pdis ) // �܂�����
	    {
//    	      display("\t\tcontinue\n");
	      double dis0 = V3DistanceBetween2Points( &pos, &intsec );
	      //display("\t+%g\n", dis0);
	      distance += dis0;
#if defined(_DEBUG)
	      tmpdis += dis0;
	      tmpdis = 0.0;
#endif
//  	      display("\t+%g\n", tmpdis);
	      
	      // ���W�C�p�����[�^�����̃A�b�v�f�[�g
	      disL -= pdis;
	      V2Equal( &uv, &intsec2 );
	      V3Equal( &pos, &intsec );

	      
#if defined(_DEBUG)
	      Splv *lplv; Spvt *lpvt;
	      lplv = create_ppdloopvertex( lp );
	      lpvt = create_ppdvertex( tmpppd );
	      lpvt->vec.x = pos.x;
	      lpvt->vec.y = pos.y;
	      lpvt->vec.z = pos.z;
	      lplv->vt = lpvt;
#endif

	      // �G�b�W�����E�G�b�W�̂Ƃ��ɂ̓��[�v�𔲂���
	      if ( ned->isBoundary == TRUE ) break;
	  
	      // fc �̔��Α��̖�
	      fc = ppdedge_another_fc( ned, fc );
	      ed = ned;
	  
	    }
	  else // �I���
	    {
//    	      display("\t\tend\n");
#if defined(_DEBUG)	      
	      Vec2d tmpuv;
	      double den = disL / pdis;
	      tmpuv.x = uv.x + den * ( intsec2.x - uv.x );
	      tmpuv.y = uv.y + den * ( intsec2.y - uv.y );
//    	      display("tmpuv %g %g nuv %g %g\n",
//    		      tmpuv.x, tmpuv.y, nuv.x, nuv.y );
#endif
	      // �I���_�͂��Ȃ炸 nuv �ɂȂ�
	      Vec fin;
	      ppdface_uv_to_coord( fc, &nuv, &fin );
	      double dis0 = V3DistanceBetween2Points( &pos, &fin );
	      //display("\t+%g\n", dis0 );
	      distance += dis0;
	      
#if defined(_DEBUG)
	      tmpdis += dis0;
#endif
	      
	      // ���̂��߂̍��W�C�p�����[�^�����̃A�b�v�f�[�g
	      V2Equal( &uv, &nuv );
	      V3Equal( &pos, &fin );

#if defined(_DEBUG)
	      if ( j == div )
		{
		  Splv *lplv; Spvt *lpvt;
		  lplv = create_ppdloopvertex( lp );
		  lpvt = create_ppdvertex( tmpppd );
		  lpvt->vec.x = pos.x;
		  lpvt->vec.y = pos.y;
		  lpvt->vec.z = pos.z;
		  lplv->vt = lpvt;
		}
#endif
	      
	      ed = NULL;
	      break;
	    }

	}
//        display("\t+%g\n", tmpdis );
      
    }

#if defined(_DEBUG)
  // ���n���t�@�C���o��
  char filename[BUFSIZ];
  sprintf( filename, "geodesic%d_%02d.sp", type, q );
  write_sp_file( lp, filename, "");

  // �J��
  for ( lplv = lp->splv; lplv != NULL; lplv = lplv->nxt )
    {
      free_ppdvertex( lplv->vt, tmpppd );
    }
  free_splp( lp );
#endif
  

  return distance;
}

//
// �p�����[�^���� start, end �Ԃ̑��n�������̌v�Z
//
double uv_chord_length( int div, int q, Vec* ovec, int dir )
{
  // ���n������
  double distance = 0.0;

  // ���E�� (j = 0) 
  int n = ( dir == XPARAM ) ? q * (div + 1) : q;
  // �T���ʂ��Ȃ��Ȃ�܂ő�����
  for ( int j = 1; j <= div; ++j )
    {
      int n1 = ( dir == XPARAM ) ? (q * (div + 1) + j) : (j * (div + 1) + q);
      distance += V3DistanceBetween2Points( &(ovec[n]), &(ovec[n1]) );
//        display("\tlength %g\n", V3DistanceBetween2Points( &(ovec[n]), &(ovec[n1]) ) );
      n = n1;
    }

  return distance;
}

// �����p�����[�^���␳�֐�
void uv_isoparam_diff_old( Splp* lp, int div, Vec2d* uvcorner,
			   Vec2d* duv, Vec* dvec, int type )
{
  // �p�����[�^��ԏ�ł̋��E��̎n�_�ƏI�_�̊i�[
  Vec2d subs, sube;
  if ( type == XPARAM )
    {
      subs.x = uvcorner[1].x - uvcorner[0].x;
      subs.y = uvcorner[1].y - uvcorner[0].y;
      sube.x = uvcorner[2].x - uvcorner[3].x;
      sube.y = uvcorner[2].y - uvcorner[3].y;
    }
  else
    {
      subs.x = uvcorner[3].x - uvcorner[0].x;
      subs.y = uvcorner[3].y - uvcorner[0].y;
      sube.x = uvcorner[2].x - uvcorner[1].x;
      sube.y = uvcorner[2].y - uvcorner[1].y;
    }
      
  for ( int i = 1; i < div; ++i )
    {
      Vec2d uvstart, uvend;
      double ddiv = (double) div;
      Vec2d *sc, *ec;
      if ( type == XPARAM )
	{
	  sc = &(uvcorner[0]); ec = &(uvcorner[3]);
	}
      else
	{
	  sc = &(uvcorner[0]); ec = &(uvcorner[1]);
	}
      uvstart.x = sc->x + (double) i * subs.x / ddiv;
      uvstart.y = sc->y + (double) i * subs.y / ddiv;
      uvend.x   = ec->x + (double) i * sube.x / ddiv;
      uvend.y   = ec->y + (double) i * sube.y / ddiv;
      
      // uvstart.x, uvstart.y �̓����Ă��鋫�E�G�b�W��������
      Sped *edS = ppd_find_boundaryedge( lp, uvstart.x, uvstart.y );
      assert( edS != NULL );

      // ���E�G�b�W�ɗאڂ���ʂ�T��
      Spfc *fcS = NULL;
      if ( edS->lhe != NULL ) fcS = edS->lhe->bpfc;
      else if ( edS->rhe != NULL ) fcS = edS->rhe->bpfc;
      assert( fcS != NULL );
      
      //�p�����[�^���� uvstart, uvend �ɂ�鑪�n�������̌v�Z
      double geo = uv_geodesic_distance_old( &(uvstart), &(uvend), edS, fcS );
//        display("i %d geo %g\n", i, geo );

      // ��_�̃p�����[�^���W
      Vec2d uv; V2Equal( &uv, &(uvstart) );
      // ��_�̎O�������W
      Vec pos; ppdface_uv_to_coord( fcS, &uv, &pos );

      // �m���Ɍ�_���������߂�end �����ւ��������΂�
      Vec2d endL;
      double val = 1.01;
      endL.x = uvstart.x + val * (uvend.x - uvstart.x );
      endL.y = uvstart.y + val * (uvend.y - uvstart.y );
      
      Sped* ed = edS;
      Spfc* fc = fcS;
//        display("chord-length\n");
      for ( int j = 1; j < div; ++j )
	{

	  // �X�e�b�v����
	  double edis = geo / (double) div;
	  
	  while ( TRUE )
	    {
	      Vec2d intsec2;
	      Sped* ned = ppd_find_nextedge_intsec( fc, ed,
						    &(uvstart), &endL,
						    &intsec2 );
	      assert( ned != NULL );

	      // �p�����[�^��_�̍��W�l�ւ̕ϊ�
	      Vec intsec;
	      ppdface_uv_to_coord( fc, &intsec2, &intsec );

	      double len = V3DistanceBetween2Points( &pos, &intsec );

	      if ( edis < len ) // fc �̒��ɓ_�����݂���
		{
		  // �ʏ�̃p�����[�^�_�̌v�Z
		  int n;
		  if ( type == XPARAM )
		    n = i * (div + 1) + j;
		  else
		    n = j * (div + 1) + i;
		  
		  double den = edis / len;
		  duv[n].x = uv.x + den * (intsec2.x - uv.x);
		  duv[n].y = uv.y + den * (intsec2.y - uv.y);

		  // ���̓_�̒T���̂��߂ɔ�����
		  V2Equal( &uv, &(duv[n]) );
		  ppdface_uv_to_coord( fc, &(duv[n]), &pos );
		  V3Equal( &(dvec[n]), &pos );
		  break; // while ���𔲂���
		}
	      else
		{
		  edis -= len;
		}
	      
	      // ���W�̃A�b�v�f�[�g
	      V3Equal( &pos, &intsec );
	      V2Equal( &uv, &intsec2 );
	      // fc �̔��Α��̖�
	      fc = ppdedge_another_fc( ned, fc );
	      ed = ned;
	      
	    }
	}
    }
}

  
  
// �����p�����[�^���␳�֐�
// �p�����[�^�̈ړ��ʂ̕��ϋ�����Ԃ�
//  type: DIFF_GEODIS (���n��) or DIFF_CHORD (����)
//  dir: XPARAM or YPARAM
double uv_isoparam_diff( Splp* lp, int div, Vec2d* uvcorner,
			 Vec2d* ouv, Vec* ovec, int type, int dir )
{
  int vn = (div + 1) * (div + 1);
  // �␳��
  Vec2d* duv  = (Vec2d*) malloc( vn * sizeof(Vec2d) );
  Vec*   dvec = (Vec*)    malloc( vn * sizeof(Vec) );
  // ������
  int k;
  for ( k = 0; k < vn; ++k )
    {
      duv[k].x = ouv[k].x;
      duv[k].y = ouv[k].y;
      dvec[k].x = ovec[k].x;
      dvec[k].y = ovec[k].y;
      dvec[k].z = ovec[k].z;
    }
  
  // �p�����[�^��ԏ�ł̋��E��̎n�_�ƏI�_�̊i�[
  for ( int i = 1; i < div; ++i )
    {
      Vec2d uvstart;
      int p = ( dir == XPARAM ) ? (i * (div + 1)) : (i);
      V2Equal( &(uvstart), &(ouv[p]) );
      
      // uvstart.x, uvstart.y �̓����Ă��鋫�E�G�b�W��������
      Sped *edS = ppd_find_boundaryedge( lp, uvstart.x, uvstart.y );
      assert( edS != NULL );

      // ���E�G�b�W�ɗאڂ���ʂ�T��
      Spfc *fcS = NULL;
      if ( edS->lhe != NULL ) fcS = edS->lhe->bpfc;
      else if ( edS->rhe != NULL ) fcS = edS->rhe->bpfc;
      assert( fcS != NULL );
      
      //�p�����[�^���� uvstart, uvend �Ԃ̑��n�������̌v�Z
      double geo;
      if ( type == DIFF_GEODIS )
	{
	  geo = uv_geodesic_distance( edS, fcS, div, i, ouv, dir );
	}
      else // type == DIFF_CHORD
	{
	  geo = uv_chord_length( div, i, ovec, dir );
	}
	
//        display("type %d distance %g\n", type, geo );
//        printf("i %d geo %g\n", i, geo );

      // ��_�̃p�����[�^���W
      Vec2d uv; V2Equal( &uv, &(uvstart) );
      // ��_�̎O�������W
      Vec pos; ppdface_uv_to_coord( fcS, &uv, &pos );

      // �m���Ɍ�_���������߂� end �����ւ��������΂�

      int k = 0;
      int q  = ( dir == XPARAM ) ? (i * (div + 1) + k) : (k * (div + 1) + i);
      ++k;
      int q1 = ( dir == XPARAM ) ? (i * (div + 1) + k) : (k * (div + 1) + i);
      Vec2d startL, endL;
      startL.x = ouv[q].x;
      startL.y = ouv[q].y;
      endL.x   = ouv[q].x + MUL * (ouv[q1].x - ouv[q].x);
      endL.y   = ouv[q].y + MUL * (ouv[q1].y - ouv[q].y);
      
      Sped* ed = edS;
      Spfc* fc = fcS;
      for ( int j = 1; j < div; ++j )
	{

	  // �X�e�b�v����
	  double len3 = geo / (double) div;
//    	  display("j %d len3(created) %g\n", j, len3 );

	  double tmpdis = 0.0;
	  
	  while ( TRUE )
	    {
//  	      display("\tj %d ed %d %d fc %d\n", j,
//  		      (ed != NULL) ? ed->no : SMDNULL,
//  		      (ed != NULL) ? ed->isBoundary : SMDNULL,
//  		      fc->no );
		  
	      Vec2d intsec2;
	      Sped* ned = ppd_find_nextedge_intsec( fc, ed, &startL, &endL, &intsec2 );
	      assert( ned != NULL );

	      // �p�����[�^��̂��Ƃ̓_�ƌ�_�Ƃ̒���
	      double pdis  = V2DistanceBetween2Points( &uv, &intsec2 );
	      // �p�����[�^��̂��Ƃ̓_�ƏI�_�Ƃ̒���
	      double pdis0 = V2DistanceBetween2Points( &uv, &(ouv[q1]) );

//  	      display("pdis %g pdis0 %g\n", pdis, pdis0 );
//  	      display("\t(in main loop) uv %g %g ouv[%d] %g %g ouv[%d] %g %g\n",
//  		      uv.x, uv.y, q, ouv[q].x, ouv[q].y, q1, ouv[q1].x, ouv[q1].y );

	      if ( pdis0 < pdis ) // param update //
		{
		  // geom update ���ǂ����𒲂ׂ�
		  Vec intsec;
		  ppdface_uv_to_coord( fc, &(ouv[q1]), &intsec );
		  double edis = V3DistanceBetween2Points( &pos, &intsec );
		  
		  if ( len3 < edis ) // param update �̑O�� geom update ���K��� //
		    {
//  		      display("geom update(1)\n");
		      // �ʏ�̃p�����[�^�_�̌v�Z
		      double den3 = len3 / edis;
		      int n = ( dir == XPARAM ) ? (i * (div + 1) + j) : (j * (div + 1) + i);
		      duv[n].x = uv.x + den3 * (ouv[q1].x - uv.x);
		      duv[n].y = uv.y + den3 * (ouv[q1].y - uv.y);
		      // �ʏ�̓_�̌v�Z
		      ppdface_uv_to_coord( fc, &(duv[n]), &(dvec[n]) );
		      

		      //len2 -= (pdis * den3);
		      
		      // ���W�̃A�b�v�f�[�g
		      V2Equal( &uv, &(duv[n]) );

#if defined(_DEBUG)
		      //TEMP
		      Vec pos1;
		      ppdface_uv_to_coord( fc, &(uv), &pos1 );
		      tmpdis += V3DistanceBetween2Points( &pos, &pos1 );
		      V3Equal( &pos, &pos1 );
		      //TEMP
#endif	      
		      ppdface_uv_to_coord( fc, &(uv), &pos );
		      
		      break; // while ���𔲂���
		    }
		  else // �{���� param update //
		    {
//  		      display("param update\n");
		      // �����p�����[�^�̍X�V
		      q  = q1; ++k;
		      q1 = ( dir == XPARAM )
			? (i * (div + 1) + k) : (k * (div + 1) + i);
		      V2Equal( &startL, &(ouv[q]) );
		      endL.x   = ouv[q].x + MUL * (ouv[q1].x - ouv[q].x);
		      endL.y   = ouv[q].y + MUL * (ouv[q1].y - ouv[q].y);
		      
		      // �����̃A�b�v�f�[�g
		      len3 -= edis;
		      assert( len3 > 0 );
		      tmpdis += edis;

		      // ���W�̃A�b�v�f�[�g
		      V2Equal( &uv, &(ouv[q]) );
		      V3Equal( &pos, &intsec );

		      ed = NULL;
//  		      display("\t(in param update) uv %g %g ouv[%d] %g %g ouv[%d] %g %g\n",
//  			      uv.x, uv.y, q, ouv[q].x, ouv[q].y, q1, ouv[q1].x, ouv[q1].y );
			      
		    }
		}
	      else // continue //
		{
		  // geom update ���ǂ����𒲂ׂ� //
		  Vec intsec;
		  ppdface_uv_to_coord( fc, &intsec2, &intsec );
		  double edis = V3DistanceBetween2Points( &pos, &intsec );

		  if ( len3 < edis ) // continue �̑O�� geom update ���K���
		    {
//  		      display("geom update(2)\n");
		      // �ʏ�̃p�����[�^�_�̌v�Z
		      double den3 = len3 / edis;
		      int n = ( dir == XPARAM )
			? (i * (div + 1) + j) : (j * (div + 1) + i);
		      duv[n].x = uv.x + den3 * (intsec2.x - uv.x);
		      duv[n].y = uv.y + den3 * (intsec2.y - uv.y);
		      // �ʏ�̓_�̌v�Z
		      ppdface_uv_to_coord( fc, &(duv[n]), &(dvec[n]) );

		      //len2 -= (pdis * den3);
		      
		      // ���W�̃A�b�v�f�[�g
		      V2Equal( &uv, &(duv[n]) );
#if defined(_DEBUG)
		      //TEMP
		      Vec pos1;
		      ppdface_uv_to_coord( fc, &(uv), &pos1 );
		      tmpdis += V3DistanceBetween2Points( &pos, &pos1 );
		      V3Equal( &pos, &pos1 );
		      //TEMP
#endif	      
		      ppdface_uv_to_coord( fc, &(uv), &pos );
		      
		      
		      break; // while ���𔲂���
		    }
		  else // �{���� continue
		    {
//  		      display("continue\n");
		      // �����̃A�b�v�f�[�g
		      len3 -= edis;
		      assert( len3 > 0 );
		      tmpdis += edis;
		      
		      // ���W�̃A�b�v�f�[�g
		      V2Equal( &uv, &intsec2 );
		      V3Equal( &pos, &intsec );
		  
		      // �G�b�W�����E�G�b�W�̂Ƃ��ɂ̓��[�v�𔲂���
		      if ( ned->isBoundary == TRUE ) break;
		      
		      // fc �̔��Α��̖�
		      fc = ppdedge_another_fc( ned, fc );
		      ed = ned;
		    }
		}
	      
//  	      display("while continue! len3 %g\n", len3);
	      
	    }
	  
//  	  display("broken! tmpdis %g\n", tmpdis);
	  
	}
      
    }
  // �A�b�v�f�[�g
  double val = 0.0;
  for ( k = 0; k < vn; ++k )
    {
      double vx = duv[k].x - ouv[k].x;
      double vy = duv[k].y - ouv[k].y;
      val += sqrt( vx * vx + vy * vy );
      ouv[k].x = duv[k].x;
      ouv[k].y = duv[k].y;
      ovec[k].x = dvec[k].x;
      ovec[k].y = dvec[k].y;
      ovec[k].z = dvec[k].z;
    }
  val /= (double) vn;

  // �������߂�
  free( duv );
  free( dvec );

  return val;
}

Sped *ppd_find_boundaryedge( Splp *lp, double xparam, double yparam )
{
  for ( Splv *lv = lp->splv; lv != NULL; lv = lv->nxt )
    {

      Spvt *vt0 = lv->vt;
      Spvt *vt1 = ( lv == lp->eplv ) ? lp->splv->vt : lv->nxt->vt;
//        double xval = ( xparam - vt0->uvw.x ) / ( vt1->uvw.x - vt0->uvw.x );
//        double yval = ( yparam - vt0->uvw.y ) / ( vt1->uvw.y - vt0->uvw.y );
//        if ( (0.0 <= xval) && (1.0 >= xval) && (0.0 <= yval) && (1.0 >= yval) ) {
      Vec2d v;
      v.x = xparam; v.y = yparam;
      if ( isParamsOnLine( &v, &(vt0->uvw), &(vt1->uvw) ) == TRUE )
	{
	  double dis = V2DistanceBetween2Points( &v, &(vt0->uvw) )
	    / V2DistanceBetween2Points( &(vt1->uvw), &(vt0->uvw) );
//  	  printf("\tdis %g\n", dis );
	  if ( (0.0 < dis) && (1.0 > dis) )
	    {
	      Sped *ed = find_ppdedge_new( vt0, vt1 );
	      return ed;
	    }
	}
    }
  
  return NULL;
}

// �ʂ̃G�b�W�̂����Ced �ł͂Ȃ� uvstart, uvend �ɂ���Ē�`�����
// �����ƌ����G�b�W��T��
Sped *ppd_find_nextedge( Spfc *fc, Sped *ed, Vec2d *v0, Vec2d *v1 )
{
  Sphe *he = fc->sphe;
  do {
    if ( he->ed == ed ) continue;

    Vec2d *sv = &(he->ed->sv->uvw);
    Vec2d *ev = &(he->ed->ev->uvw);
    
    if ( isLineSegmentCrossing( sv, ev, v0, v1 ) == TRUE ) {
      
      return he->ed;
    }
    
  } while ( (he = he->nxt) != fc->sphe );

  return NULL;
}

// �ʂ̃G�b�W�̂����Ced �ł͂Ȃ� uvstart, uvend �ɂ���Ē�`�����
// �����ƌ����G�b�W��T�� (�p�����[�^��_��)
Sped *ppd_find_nextedge_intsec( Spfc *fc, Sped *ed, Vec2d *v0, Vec2d *v1,
				Vec2d* intsec )
{
  Sphe *he = fc->sphe;
  do
    {
      if ( he->ed == ed )
	{
	  continue;
	}

      Vec2d *sv = &(he->ed->sv->uvw);
      Vec2d *ev = &(he->ed->ev->uvw);

#if defined(_DEBUG)
      // TEMP
      Vec2d sub1, sub2;
      V2Sub( sv, ev, &sub1 );
      V2Sub( v1, v0, &sub2 );
//        display("\t\tcross %d value %g\n",
//  	      isLineSegmentCrossing( sv, ev, v0, v1 ),
//  	      V2Sign( &sub1, &sub2 ) );
      // TEMP
#endif	      
      
      if ( isLineSegmentCrossing( sv, ev, v0, v1 ) == TRUE )
	{
	  LineIntersectionPoint( sv, ev, v0, v1, intsec );
	  return he->ed;
	}
    }
  while ( (he = he->nxt) != fc->sphe );

  return NULL;
}

//
// radial shape mesh for ppd
//
Sppd *toradialppd( Sppd *ppd, int div )
{
  if ( ppd == NULL ) return NULL;
  if ( ppd->splp == NULL ) return NULL;
  if ( ppd->splp->lcvn < TRIANGLE ) return NULL;

  // �ӂ̐�
  int ln = ppd->splp->lcvn;
  
  // ppd, ppdsolid, ppdpart
  Sppd *newppd = create_ppd();
  Spso *newso = create_ppdsolid( newppd );
  Sppt *newpt = create_ppdpart( newppd );

  // ppdvertex
  int vn = 1 + div * div * ln;
  Spvt **pvt  = (Spvt **) malloc( vn * sizeof(Spvt *) );

  int i;
  for ( i = 0; i < vn; ++i ) {
    pvt[i] = create_ppdvertex( newppd );
    pvt[i]->vec.x = 0.0;
    pvt[i]->vec.y = 0.0;
    pvt[i]->vec.z = 0.0;
    pvt[i]->bpso = newso;
  }

  // ppdface
  int fid = 0;
  int fn = div * ln * ( 2 * div - 1 );
  Spfc **pfc  = (Spfc **) malloc( fn * sizeof(Spfc *) );
  for ( i = 0; i < fn; ++i ) {
    pfc[i] = create_ppdface( newppd );
    pfc[i]->bpso = newso;
    pfc[i]->bppt = newpt;
  }

  // �Ă��؂�̒��_��ԍ� 0 �Ƃ���
  // vertex-face connectivity
  // center vertex
  Spvt *cv = pvt[0];
//    cv->vec.x = tfc->cv->vec.x;
//    cv->vec.y = tfc->cv->vec.y;
//    cv->vec.z = tfc->cv->vec.z;
//    cv->uvw.x = tfc->cv->uvw.x;
//    cv->uvw.y = tfc->cv->uvw.y;
    
  for ( i = 0; i < ln * div; ++i )
    {
      // �ł������̒��_
      Spvt *v1, *v2, *v3, *v4;
      Spfc *f1, *f2;
      Sphe *he;
      v1 = pvt[ 1 + div * i ];
      v2 = ( i != (ln * div - 1) ) ? pvt[ 1 + div * (i+1) ] : pvt[1];
      f1 = pfc[fid]; ++fid;
      he = create_ppdhalfedge(f1); he->vt = cv;
      he = create_ppdhalfedge(f1); he->vt = v1;
      he = create_ppdhalfedge(f1); he->vt = v2;

      for ( int j = 0; j < div - 1; ++j )
	{
	  v1 = pvt[ 1 + div * i + j];
	  v2 = ( i != (ln * div - 1) )
	    ? pvt[ 1 + div * (i+1) + j] : pvt[1+j];
	  v3 = pvt[ 1 + div * i + j + 1];
	  v4 = ( i != (ln * div - 1) )
	    ? pvt[ 1 + div * (i+1) + j+1] : pvt[1+j+1];
	  f1 = pfc[fid]; ++fid;
	  f2 = pfc[fid]; ++fid;
	  he = create_ppdhalfedge(f1); he->vt = v1;
	  he = create_ppdhalfedge(f1); he->vt = v3;
	  he = create_ppdhalfedge(f1); he->vt = v4;
	  he = create_ppdhalfedge(f2); he->vt = v1;
	  he = create_ppdhalfedge(f2); he->vt = v4;
	  he = create_ppdhalfedge(f2); he->vt = v2;
	}
    }
  //display("fn %d fid %d\n", fn, fid );

  //
  // decide vertex's 3D coordinates
  //

  // ������
  // ���_�ւ� tvector �̔ԍ��̖��ߍ���
//    TVector *tvec;
//    for ( i = 0; i < tfc->n_tvector; ++i )
//      {
//        tvec = &(tfc->tvec[i]);
//        Spvt *vt = tvec->vt;
//        // ���_�ւ� tvector �̔ԍ��̖��ߍ���
//        vt->tvec_id = i;
//      }

  Splp *lp = ppd->splp;

  //
  // �p�̓_�̍��W�l�̌���
  //
  int *nn = (int *) malloc( ln * sizeof(int) );
  for ( i = 0; i < ln; ++i )
    {
      nn[i] = 1 + i * div * div + div - 1;
    }
  
  // �p�̃p�����[�^�l���i�[
  Vec2d *uvcorner = (Vec2d *) malloc( ln * sizeof(Vec2d) );
  int cnt = -1;
  for ( Splv *lv = lp->splv; lv != NULL; lv = lv->nxt ) {
    if ( lv->type == LOOPVT_CORNER )
      {
      
	++cnt;
	Spvt *vt = lv->vt;
	
	uvcorner[cnt].x = vt->uvw.x;
	uvcorner[cnt].y = vt->uvw.y;
//        TVector *tvec = &(tfc->tvec[id]);
//        uvcorner[cnt].x = tvec->uv.x;
//        uvcorner[cnt].y = tvec->uv.y;

//        // ���W�l�̌���
//        pvt[nn[cnt]]->vec.x = vt->vec.x;
//        pvt[nn[cnt]]->vec.y = vt->vec.y;
//        pvt[nn[cnt]]->vec.z = vt->vec.z;
	pvt[nn[cnt]]->uvw.x = uvcorner[cnt].x;
	pvt[nn[cnt]]->uvw.y = uvcorner[cnt].y;
	
    }
  }

  double ddiv = (double) div;

  //
  // ���E��̓_�i�p���̂����j�̍��W�l�̌���
  //

  int bn = div * ln;
  int bid = 0;
  Vec2d *uvboundary = (Vec2d *) malloc( bn * sizeof(Vec2d) );
  for ( i = 0; i < ln; ++i )
    {

      Vec2d *s, *e, sub;
      s = &(uvcorner[i]);
      e = ( i != ln - 1 ) ? &(uvcorner[i+1]) : &(uvcorner[0]);
      sub.x = e->x - s->x;
      sub.y = e->y - s->y;
      uvboundary[bid].x = uvcorner[i].x;
      uvboundary[bid].y = uvcorner[i].y;
//        display("uvboundary %d %g %g\n", bid, uvboundary[bid].x, uvboundary[bid].y );
      ++bid;
    
      for ( int j = 1; j < div; ++j )
	{

	  int n = 1 + (i * div * div) + (div * j) + div - 1;
      
	  double xparam = s->x + (double) j * sub.x / ddiv;
	  double yparam = s->y + (double) j * sub.y / ddiv;
	  uvboundary[bid].x = xparam;
	  uvboundary[bid].y = yparam;
	//        display("uvboundary %d %g %g\n", bid, uvboundary[bid].x, uvboundary[bid].y );
	  ++bid;

#if 0	
	  Sped *ed = remesh_find_boundaryedge( tfc, lp, xparam, yparam );
	  assert( ed != NULL );
	  Spfc *fc = NULL;
	  if ( ed->lhe != NULL ) {
	    if ( ed->lhe->bpfc->tile_id == tfc->no ) fc = ed->lhe->bpfc;
	  }
	  if ( ed->rhe != NULL ) {
	    if ( ed->rhe->bpfc->tile_id == tfc->no ) fc = ed->rhe->bpfc;
	  }
	  assert( fc != NULL );
	  double a1, a2, a3;
	  remesh_ppdface_find_barycentric_coordinate( tfc, fc, xparam, yparam,
						    &a1, &a2, &a3 );
	  ppdface_barycentric_coordinate( fc, a1, a2, a3, &(pvt[n]->vec) );
#endif	

	  // �p�����[�^�̊i�[
	  pvt[n]->uvw.x = xparam;
	  pvt[n]->uvw.y = yparam;
      
	}
    }

  //
  // �����_�i���E���̂����j�̍��W�l�̌���
  //

  //display("cv %d\n", tfc->cv->no );
  Vec2d cuv;
#if 0  
  for ( i = 0; i < ln; ++i )
    {
      cuv.x += uvcorner[i].x;
      cuv.y += uvcorner[i].y;
    }
  cuv.x /= (double) ln;
  cuv.y /= (double) ln;
#endif  
//    cuv.x = pvt[0]->uvw.x;
//    cuv.y = pvt[0]->uvw.y;
//    pvt[0]->uvw.x = cuv.x = tfc->tvec[tfc->cv->tvec_id].uv.x;
//    pvt[0]->uvw.y = cuv.y = tfc->tvec[tfc->cv->tvec_id].uv.y;
  pvt[0]->uvw.x = cuv.x = 0.0;
  pvt[0]->uvw.y = cuv.y = 0.0;
  for ( i = 0; i < ln * div; ++i )
    {

      Vec sub;
//      sub.x = cuv.x - uvboundary[i].x;
//      sub.y = cuv.y - uvboundary[i].y;
      sub.x = uvboundary[i].x - cuv.x;
      sub.y = uvboundary[i].y - cuv.y;

#if 0
      // �ŏ��̖ʂ̒T��
      Spfc *fc = remesh_find_intsecface( tfc, tfc->cv, &cuv, &(uvboundary[i]) );
      assert( fc != NULL );
      Sped *ed = NULL;
//        display("found. fc %d\n", fc->no );

#endif
//      Sped *ed = remesh_find_boundaryedge( tfc, lp, 
//  					 uvboundary[i].x, uvboundary[i].y );
//      assert( ed != NULL );
//      Spfc *fc = NULL;
//      if ( ed->lhe != NULL ) {
//        if ( ed->lhe->bpfc->tile_id == tfc->no ) fc = ed->lhe->bpfc;
//      }
//      if ( ed->rhe != NULL ) {
//        if ( ed->rhe->bpfc->tile_id == tfc->no ) fc = ed->rhe->bpfc;
//      }
//      assert( fc != NULL );
    
      //for ( int j = div - 1; j >= 1; --j ) {
      for ( int j = 1; j < div; ++j )
      {
      
	int n = 1 + (div * i) + (j - 1);
//        double xparam = uvboundary[i].x + (double) j * sub.x / ddiv;
//        double yparam = uvboundary[i].y + (double) j * sub.y / ddiv;
	double xparam = cuv.x + (double) j * sub.x / ddiv;
	double yparam = cuv.y + (double) j * sub.y / ddiv;

#if 0	
	while ( TRUE ) {
	
	  // xparam, yparam ���� fc �̒��ɓ����Ă���
	  if ( remesh_isParamsInFace( tfc, fc, xparam, yparam ) == TRUE ) {
	  
	    // ���_�̍��W�l���v�Z
	    double a1, a2, a3;
	    remesh_ppdface_find_barycentric_coordinate( tfc, fc, xparam, yparam,
						      &a1, &a2, &a3 );
	    ppdface_barycentric_coordinate( fc, a1, a2, a3, &(pvt[n]->vec) );
//  	    display("ok. param %g %g fc %d\n", xparam, yparam, fc->no );
#endif	    
	  
	    // �p�����[�^�̊i�[
	    pvt[n]->uvw.x = xparam;
	    pvt[n]->uvw.y = yparam;

#if 0	    
	    break;
	  
	  } else { // ���̖ʂɂ͂Ȃ��̂Ŏ��̖ʂ�
	  
	    // �ʂ̃G�b�W�̂����Ced �ł͂Ȃ� uvstart, uvend �ɂ���Ē�`�����
	    // �����ƌ����G�b�W��T��
	    if ( ed != NULL ) {
//  	      display("aa\n");
	      ed = remesh_nextedge( tfc, fc, ed, &(cuv), &(uvboundary[i]) );
	    } else {
//  	      display("bb\n");
	      ed = remesh_find_intsecedge( tfc, fc, tfc->cv );
	    }
	    assert( ed != NULL );
//  	    display("ed %d\n", ed->no );
	  
	    // fc �̔��Α��̖�
	    fc = ppdedge_another_fc( ed, fc );
	    assert( fc != NULL );
//  	    display("next fc %d\n", fc->no );
	    
	  }
	
	}
#endif	
      }
    }
    
  free( uvboundary );
  free( uvcorner );
  free( nn );
  free( pvt );
  free( pfc );
  
  return newppd;
}

//
// ppd �̃p�����[�^�̍ăp�����[�^��
// newppd �̃p�����[�^���Cvt->uvw ���� ouv �ɕό`�����Ƃ��� ppd �̃p�����[�^
// �̕ϕ������߂�
//
void remL_reparam( Sppd* ppd, Sppd* refppd, Vec2d* ouv )
{
  for ( Spvt* vt = ppd->spvt; vt != NULL; vt = vt->nxt )
    {
      if ( vt->isBoundary == TRUE ) continue;

      // refppd �̖� rfc ��������
      Spfc* rfc;
      if ( (rfc = remL_find_ppdface( refppd, &(vt->uvw) )) == NULL )
	{
	  fprintf(stderr, "okasii!!\n");
	  continue;
	}

      double a1, a2, a3;
      ppdface_find_barycentric_coordinate( rfc, vt->uvw.x, vt->uvw.y,
					   &a1, &a2, &a3 );

      // �V�������W
      Vec2d *v1, *v2, *v3;
      v1 = &(ouv[rfc->sphe->vt->tvec_id]);
      v2 = &(ouv[rfc->sphe->nxt->vt->tvec_id]);
      v3 = &(ouv[rfc->sphe->nxt->nxt->vt->tvec_id]);
      vt->uvw.x = a1 * v1->x + a2 * v2->x + a3 * v3->x;
      vt->uvw.y = a1 * v1->y + a2 * v2->y + a3 * v3->y;
      
    }
      
}

Spfc* remL_find_ppdface( Sppd* ppd, Vec2d* vec2 )
{
  for ( Spfc* fc = ppd->spfc; fc != NULL; fc = fc->nxt )
    {
      if ( isParamsInFace( fc, vec2->x, vec2->y ) == TRUE )
	return fc;
    }
  return NULL;
}

//
// grid mesh for tfc
//
Sppd *tolatticemesh( TFace *tfc, int div )
{
  if ( tfc == NULL ) return NULL;
  if ( tfc->n_tedge != RECTANGLE ) return NULL;

#if defined(_DEBUG)
  tmpppd = ppd;
#endif

  // ������
  // ���_�ւ� tvector �̔ԍ��̖��ߍ���
  TVector *tvec;
  for ( int j = 0; j < tfc->n_tvector; ++j )
    {
      tvec = &(tfc->tvec[j]);
      Spvt *vt = tvec->vt;
      // ���_�ւ� tvector �̔ԍ��̖��ߍ���
      vt->tvec_id = j;
    }

  Splp *lp = tfc->clp;
  
  // ppdvertex array
  int vn = (div + 1) * (div + 1);
  Spvt **pvt  = (Spvt **) malloc( vn * sizeof(Spvt *) );
  
  //
  // Lattice �^ remesh �̐����C���_�z��C�ʔz��ւ̊i�[
  //
  Sppd *newppd = remL_init( div, pvt );
  
  //
  // �p�̓_�̍��W�l�̌���
  //
  Vec2d uvcorner[RECTANGLE];
  remL_corner( lp, uvcorner, div, pvt );
  
  //
  // ���E��̓_�i�p���̂����j�̍��W�l�̌���
  //
  remL_boundary( lp, uvcorner, div, pvt );
  
  //
  // �����_�i���E���̂����j�̍��W�l�̌��� (�␳�Ȃ�)
  //
  remL_internal( lp, uvcorner, div, pvt );

  //
  // �����_�i���E���̂����j�̍��W�l�̃p�����[�^�␳
  //
  // �I���W�i���̃p�����[�^
  Vec2d* ouv  = (Vec2d*) malloc( vn * sizeof(Vec2d) );
  // �␳���̃p�����[�^�C�x�N�g��
  Vec2d* duv  = (Vec2d*) malloc( vn * sizeof(Vec2d) );
  Vec*   dvec = (Vec*) malloc( vn * sizeof(Vec) );
  for ( int k = 0; k < vn; ++k )
    {
      ouv[k].x = duv[k].x = pvt[k]->uvw.x;
      ouv[k].y = duv[k].y = pvt[k]->uvw.y;
      dvec[k].x = pvt[k]->vec.x;
      dvec[k].y = pvt[k]->vec.y;
      dvec[k].z = pvt[k]->vec.z;

    }

  // �␳
  remL_internal_update( lp, uvcorner, div, duv, dvec, DIFF_GEODIS );

  // ��U�A�b�v�f�[�g
  for ( int i = 0; i < vn; ++i )
    {
      pvt[i]->uvw.x = duv[i].x;
      pvt[i]->uvw.y = duv[i].y;
      pvt[i]->vec.x = dvec[i].x;
      pvt[i]->vec.y = dvec[i].y;
      pvt[i]->vec.z = dvec[i].z;

      // ��̍ăp�����[�^���Ŏg��
      pvt[i]->tvec_id = i;
    }

  //
  // ppd �̃p�����[�^�̍ăp�����[�^��
  // (re-parameterization)
  // newppd �̃p�����[�^���Cvt->uvw ���� ouv �ɕό`�����Ƃ��� ppd �̃p�����[�^
  // �̕ϕ������߂�
  //
  // remL_reparam( ppd, newppd, ouv );
  
  free( ouv );
  free( duv );
  free( dvec );
  
  free( pvt );
  
  return newppd;
}

//
// grid mesh for tfc
//
Sppd *tolatticemesh_bak( TFace *tfc, int div )
{
  if ( tfc == NULL ) return NULL;
  if ( tfc->n_tedge != RECTANGLE ) return NULL;

  
  // ppd, ppdsolid, ppdpart
  Sppd *newppd = create_ppd();
  Spso *newso = create_ppdsolid( newppd );
  Sppt *newpt = create_ppdpart( newppd );

  //int div = INTERVAL;

  int i;
  // ppdvertex
  int vn = (div + 1) * (div + 1);
  Spvt **pvt  = (Spvt **) malloc( vn * sizeof(Spvt *) );
  for ( i = 0; i < vn; ++i ) {
    pvt[i] = create_ppdvertex( newppd );
    pvt[i]->vec.x = 0.0;
    pvt[i]->vec.y = 0.0;
    pvt[i]->vec.z = 0.0;
    pvt[i]->bpso = newso;
  }

  // ppdface
  int fn = div * div * 2;
  Spfc **pfc  = (Spfc **) malloc(fn * sizeof(Spfc *));
  for ( i = 0; i < fn; ++i ) {
    pfc[i] = create_ppdface( newppd );
    pfc[i]->bpso = newso;
    pfc[i]->bppt = newpt;
  }

  // vertex-face connectivity
  int fid = 0;
  Spvt *v1, *v2, *v3, *v4;
  Spfc *f1, *f2;
  Sphe *he;
  int j;
  for ( i = 0; i < div; ++i ) {
    for ( j = 0; j < div; ++j ) {
      v1 = pvt[i * (div + 1) + j];
      v2 = pvt[i * (div + 1) + j + 1];
      v3 = pvt[(i + 1) * (div + 1) + j];
      v4 = pvt[(i + 1) * (div + 1) + j + 1];
      f1 = pfc[fid]; ++fid;
      f2 = pfc[fid]; ++fid;
      
      // create halfedge
      he = create_ppdhalfedge(f1); he->vt = v1;
      he = create_ppdhalfedge(f1); he->vt = v3;
      he = create_ppdhalfedge(f1); he->vt = v2;
      he = create_ppdhalfedge(f2); he->vt = v2;
      he = create_ppdhalfedge(f2); he->vt = v3;
      he = create_ppdhalfedge(f2); he->vt = v4;
    }
  }

  //
  // decide vertex's 3D coordinates
  //

  // ������
  // ���_�ւ� tvector �̔ԍ��̖��ߍ���
  TVector *tvec;
  for ( i = 0; i < tfc->n_tvector; ++i ) {
    tvec = &(tfc->tvec[i]);
    Spvt *vt = tvec->vt;
    // ���_�ւ� tvector �̔ԍ��̖��ߍ���
    vt->tvec_id = i;
  }

  Splp *lp = tfc->clp;
  
  //
  // �p�̓_�̍��W�l�̌���
  //
  int nn[RECTANGLE];
  nn[0] = 0;
  nn[1] = div * (div + 1);
  nn[2] = (div + 1) * (div + 1) - 1;
  nn[3] = div;
  
  // �p�̃p�����[�^�l���i�[
  Vec2d uvcorner[RECTANGLE];
  int cnt = -1;
  for ( Splv *lv = lp->splv; lv != NULL; lv = lv->nxt ) {
    if ( lv->vt->sp_type == SP_VERTEX_TVERTEX ) {
      ++cnt;
      Spvt *vt = lv->vt;
      int id = vt->tvec_id;
      
      TVector *tvec = &(tfc->tvec[id]);
      uvcorner[cnt].x = tvec->uv.x;
      uvcorner[cnt].y = tvec->uv.y;

      // ���W�l�̌���
      pvt[nn[cnt]]->vec.x = vt->vec.x;
      pvt[nn[cnt]]->vec.y = vt->vec.y;
      pvt[nn[cnt]]->vec.z = vt->vec.z;
//        display("corner %d %g %g\n", cnt, uvcorner[cnt].x, uvcorner[cnt].y );
      // �p�����[�^���W�̐ݒ�
      pvt[nn[cnt]]->uvw.x = uvcorner[cnt].x;
      pvt[nn[cnt]]->uvw.y = uvcorner[cnt].y;
    }
  }

  double ddiv = (double) div;

  //
  // ���E��̓_�i�p���̂����j�̍��W�l�̌���
  //
  
  int u = 0;
  for ( i = 0; i < RECTANGLE; ++i ) {

    Vec2d *s = NULL, *e = NULL, sub;
    if ( i == 0 ) {
      u = 0;
      s = &(uvcorner[0]);
      e = &(uvcorner[1]);
    } else if ( i == 1 ) {
      u = div;
      s = &(uvcorner[3]);
      e = &(uvcorner[2]);
    } else if ( i == 2 ) {
      u = 0;
      s = &(uvcorner[0]);
      e = &(uvcorner[3]);
    } else if ( i == 3 ) {
      u = div;
      s = &(uvcorner[1]);
      e = &(uvcorner[2]);
    }
    sub.x = e->x - s->x;
    sub.y = e->y - s->y;
    
    for ( j = 1; j < div; ++j ) {

      int n;
      if ( i < 2 ) {
	n = j * (div + 1) + u;
      } else {
	n = u * (div + 1) + j;
      }
      
      double xparam = s->x + (double) j * sub.x / ddiv;
      double yparam = s->y + (double) j * sub.y / ddiv;
      
      Sped *ed = remesh_find_boundaryedge( tfc, lp, xparam, yparam );
      assert( ed != NULL );
      Spfc *fc = NULL;
      if ( ed->lhe != NULL ) {
	if ( ed->lhe->bpfc->tile_id == tfc->no ) fc = ed->lhe->bpfc;
      }
      if ( ed->rhe != NULL ) {
	if ( ed->rhe->bpfc->tile_id == tfc->no ) fc = ed->rhe->bpfc;
      }
      assert( fc != NULL );
      double a1, a2, a3;
      remesh_ppdface_find_barycentric_coordinate( tfc, fc, xparam, yparam,
						  &a1, &a2, &a3 );
      ppdface_barycentric_coordinate( fc, a1, a2, a3, &(pvt[n]->vec) );
//        display("boundary vertex %d uvw %g %g vec %g %g %g\n",
//  	      pvt[n]->no, xparam, yparam, 
//  	      pvt[n]->vec.x, pvt[n]->vec.y, pvt[n]->vec.z );
//        display("\tfc %d bc %g %g %g\n\n", fc->no, a1, a2, a3 );

      // �p�����[�^���W�̐ݒ�
      pvt[n]->uvw.x = xparam;
      pvt[n]->uvw.y = yparam;
    }
//      display("\n");
  }
  
  //
  // �����_�i���E���̂����j�̍��W�l�̌���
  //
  
  // �p�����[�^��ԏ�ł̋��E��̎n�_�ƏI�_�̊i�[
  Vec2d *uvstart = (Vec2d *) malloc( (div+1) * sizeof(Vec2d) );
  Vec2d *uvend   = (Vec2d *) malloc( (div+1) * sizeof(Vec2d) );
  Vec2d subs, sube;
  subs.x = uvcorner[1].x - uvcorner[0].x;
  subs.y = uvcorner[1].y - uvcorner[0].y;
  sube.x = uvcorner[2].x - uvcorner[3].x;
  sube.y = uvcorner[2].y - uvcorner[3].y;
  for ( i = 0; i <= div; ++i ) {
    uvstart[i].x = uvcorner[0].x + (double) i * subs.x / ddiv;
    uvstart[i].y = uvcorner[0].y + (double) i * subs.y / ddiv;
    uvend[i].x   = uvcorner[3].x + (double) i * sube.x / ddiv;
    uvend[i].y   = uvcorner[3].y + (double) i * sube.y / ddiv;
//      display("%d start %g %g end %g %g\n",
//  	    i, uvstart[i].x, uvstart[i].y, uvend[i].x, uvend[i].y );
  }

  // �n�_�ƏI�_�����Ԓ������T��
  for ( i = 1; i < div; ++i ) {

    //if ( (i == 0) || (i == div) ) continue;

    // uvstart[i].x, uvstart[i].y �̓����Ă��鋫�E�G�b�W��������
    Sped *ed = remesh_find_boundaryedge( tfc, lp, uvstart[i].x, uvstart[i].y );
    assert( ed != NULL );

    // ���E�G�b�W�ɗאڂ���ʂ�T��
    Spfc *fc = NULL;
    if ( ed->lhe != NULL ) {
      if ( ed->lhe->bpfc->tile_id == tfc->no ) fc = ed->lhe->bpfc;
    }
    if ( ed->rhe != NULL ) {
      if ( ed->rhe->bpfc->tile_id == tfc->no ) fc = ed->rhe->bpfc;
    }
    assert( fc != NULL );

    // xparam, yparam �̓����Ă���i�q�_�̎O�������W�l�����߂�
    Vec sub;
    sub.x = uvend[i].x - uvstart[i].x;
    sub.y = uvend[i].y - uvstart[i].y;
    for ( j = 1; j < div; ++j ) {

      int n = i * (div + 1) + j;
      double xparam = uvstart[i].x + (double) j * sub.x / ddiv;
      double yparam = uvstart[i].y + (double) j * sub.y / ddiv;
//        display("param %g %g\n", xparam, yparam );

      // point location algorithm ?
      // xparam, yparam �̓����Ă���ʂ�������
      while ( TRUE ) {
	
	// xparam, yparam ���� fc �̒��ɓ����Ă���
	if ( remesh_isParamsInFace( tfc, fc, xparam, yparam ) == TRUE ) {
	  
	  // ���_�̍��W�l���v�Z
	  double a1, a2, a3;
//  	  display("before n %d vec %g %g %g\n",
//  		  n, pvt[n]->vec.x, pvt[n]->vec.y, pvt[n]->vec.z );
	  remesh_ppdface_find_barycentric_coordinate( tfc, fc, xparam, yparam,
						      &a1, &a2, &a3 );
	  ppdface_barycentric_coordinate( fc, a1, a2, a3,
					  &(pvt[n]->vec) );
//  	  display("i %d j %d param %g %g fc %d\n",
//  		  i, j, xparam, yparam, fc->no );
	  remesh_ppdface_find_barycentric_coordinate( tfc, fc, xparam, yparam,
						      &a1, &a2, &a3 );
//  	  display("after  n %d vec %g %g %g\n",
//  		  n, pvt[n]->vec.x, pvt[n]->vec.y, pvt[n]->vec.z );

	  pvt[n]->uvw.x = xparam;
	  pvt[n]->uvw.y = yparam;
	  
	  break;
	  
	} else { // ���̖ʂɂ͂Ȃ��̂Ŏ��̖ʂ�
	  
	  // �ʂ̃G�b�W�̂����Ced �ł͂Ȃ� uvstart, uvend �ɂ���Ē�`�����
	  // �����ƌ����G�b�W��T��
	  ed = remesh_nextedge( tfc, fc, ed, &(uvstart[i]), &(uvend[i]) );
	  assert( ed != NULL );
	  
	  // fc �̔��Α��̖�
	  fc = ppdedge_another_fc( ed, fc );
	  assert( fc != NULL );
//  	  display("next fc %d\n", fc->no );
	}
	
      }

    }
  }

  free( uvstart );
  free( uvend );
  free( pfc );
  free( pvt );
  
  return newppd;
}

//
// radial shape mesh for tfc
//
Sppd *toradialmesh( TFace *tfc, int div )
{
  if ( tfc == NULL ) return NULL;

  // ppd, ppdsolid, ppdpart
  Sppd *newppd = create_ppd();
  Spso *newso = create_ppdsolid( newppd );
  Sppt *newpt = create_ppdpart( newppd );

  // ppdvertex
  int vn = 1 + div * div * tfc->n_tedge;
  Spvt **pvt  = (Spvt **) malloc( vn * sizeof(Spvt *) );

  int i;
  for ( i = 0; i < vn; ++i ) {
    pvt[i] = create_ppdvertex( newppd );
    pvt[i]->vec.x = 0.0;
    pvt[i]->vec.y = 0.0;
    pvt[i]->vec.z = 0.0;
    pvt[i]->bpso = newso;
  }

  // ppdface
  int fid = 0;
  int fn = div * tfc->n_tedge * ( 2 * div - 1 );
  Spfc **pfc  = (Spfc **) malloc( fn * sizeof(Spfc *) );
  for ( i = 0; i < fn; ++i ) {
    pfc[i] = create_ppdface( newppd );
    pfc[i]->bpso = newso;
    pfc[i]->bppt = newpt;
  }

  // vertex-face connectivity
  // center vertex
  Spvt *cv = pvt[0];
  cv->vec.x = tfc->cv->vec.x;
  cv->vec.y = tfc->cv->vec.y;
  cv->vec.z = tfc->cv->vec.z;
  cv->uvw.x = tfc->cv->uvw.x;
  cv->uvw.y = tfc->cv->uvw.y;
    
  for ( i = 0; i < tfc->n_tedge * div; ++i ) {

    // �ł������̒��_
    Spvt *v1, *v2, *v3, *v4;
    Spfc *f1, *f2;
    Sphe *he;
    v1 = pvt[ 1 + div * i ];
    v2 = ( i != (tfc->n_tedge * div - 1) ) ? pvt[ 1 + div * (i+1) ] : pvt[1];
    f1 = pfc[fid]; ++fid;
    he = create_ppdhalfedge(f1); he->vt = cv;
    he = create_ppdhalfedge(f1); he->vt = v1;
    he = create_ppdhalfedge(f1); he->vt = v2;

    for ( int j = 0; j < div - 1; ++j ) {
      v1 = pvt[ 1 + div * i + j];
      v2 = ( i != (tfc->n_tedge * div - 1) )
	? pvt[ 1 + div * (i+1) + j] : pvt[1+j];
      v3 = pvt[ 1 + div * i + j + 1];
      v4 = ( i != (tfc->n_tedge * div - 1) )
	? pvt[ 1 + div * (i+1) + j+1] : pvt[1+j+1];
      f1 = pfc[fid]; ++fid;
      f2 = pfc[fid]; ++fid;
      he = create_ppdhalfedge(f1); he->vt = v1;
      he = create_ppdhalfedge(f1); he->vt = v3;
      he = create_ppdhalfedge(f1); he->vt = v4;
      he = create_ppdhalfedge(f2); he->vt = v1;
      he = create_ppdhalfedge(f2); he->vt = v4;
      he = create_ppdhalfedge(f2); he->vt = v2;
    }
  }
  //display("fn %d fid %d\n", fn, fid );

  //
  // decide vertex's 3D coordinates
  //

  // ������
  // ���_�ւ� tvector �̔ԍ��̖��ߍ���
  TVector *tvec;
  for ( i = 0; i < tfc->n_tvector; ++i ) {
    tvec = &(tfc->tvec[i]);
    Spvt *vt = tvec->vt;
    // ���_�ւ� tvector �̔ԍ��̖��ߍ���
    vt->tvec_id = i;
  }

  Splp *lp = tfc->clp;

  //
  // �p�̓_�̍��W�l�̌���
  //
  int *nn = (int *) malloc( tfc->n_tedge * sizeof(int) );
  for ( i = 0; i < tfc->n_tedge; ++i ) {
    nn[i] = 1 + i * div * div + div - 1;
  }
  
  // �p�̃p�����[�^�l���i�[
  Vec2d *uvcorner = (Vec2d *) malloc( tfc->n_tedge * sizeof(Vec2d) );
  int cnt = -1;
  for ( Splv *lv = lp->splv; lv != NULL; lv = lv->nxt ) {
    if ( lv->vt->sp_type == SP_VERTEX_TVERTEX ) {
      ++cnt;
      Spvt *vt = lv->vt;
      int id = vt->tvec_id;
      
      TVector *tvec = &(tfc->tvec[id]);
      uvcorner[cnt].x = tvec->uv.x;
      uvcorner[cnt].y = tvec->uv.y;

      // ���W�l�̌���
      pvt[nn[cnt]]->vec.x = vt->vec.x;
      pvt[nn[cnt]]->vec.y = vt->vec.y;
      pvt[nn[cnt]]->vec.z = vt->vec.z;
      pvt[nn[cnt]]->uvw.x = tvec->uv.x;
      pvt[nn[cnt]]->uvw.y = tvec->uv.y;
    }
  }

  double ddiv = (double) div;

  //
  // ���E��̓_�i�p���̂����j�̍��W�l�̌���
  //

  int bn = div * tfc->n_tedge;
  int bid = 0;
  Vec2d *uvboundary = (Vec2d *) malloc( bn * sizeof(Vec2d) );
  for ( i = 0; i < tfc->n_tedge; ++i ) {

    Vec2d *s, *e, sub;
    s = &(uvcorner[i]);
    e = ( i != tfc->n_tedge - 1 ) ? &(uvcorner[i+1]) : &(uvcorner[0]);
    sub.x = e->x - s->x;
    sub.y = e->y - s->y;
    uvboundary[bid].x = uvcorner[i].x;
    uvboundary[bid].y = uvcorner[i].y;
//      display("uvboundary %d %g %g\n", bid, uvboundary[bid].x, uvboundary[bid].y );
    ++bid;
    
    for ( int j = 1; j < div; ++j ) {

      int n = 1 + (i * div * div) + (div * j) + div - 1;
      
      double xparam = s->x + (double) j * sub.x / ddiv;
      double yparam = s->y + (double) j * sub.y / ddiv;
      uvboundary[bid].x = xparam;
      uvboundary[bid].y = yparam;
//        display("uvboundary %d %g %g\n", bid, uvboundary[bid].x, uvboundary[bid].y );
      ++bid;
      
      Sped *ed = remesh_find_boundaryedge( tfc, lp, xparam, yparam );
      assert( ed != NULL );
      Spfc *fc = NULL;
      if ( ed->lhe != NULL ) {
	if ( ed->lhe->bpfc->tile_id == tfc->no ) fc = ed->lhe->bpfc;
      }
      if ( ed->rhe != NULL ) {
	if ( ed->rhe->bpfc->tile_id == tfc->no ) fc = ed->rhe->bpfc;
      }
      assert( fc != NULL );
      double a1, a2, a3;
      remesh_ppdface_find_barycentric_coordinate( tfc, fc, xparam, yparam,
						  &a1, &a2, &a3 );
      ppdface_barycentric_coordinate( fc, a1, a2, a3, &(pvt[n]->vec) );

      // �p�����[�^�̊i�[
      pvt[n]->uvw.x = xparam;
      pvt[n]->uvw.y = yparam;
      
    }
  }

  //
  // �����_�i���E���̂����j�̍��W�l�̌���
  //

//    display("cv %d\n", tfc->cv->no );
  Vec2d cuv;
//    cuv.x = pvt[0]->uvw.x;
//    cuv.y = pvt[0]->uvw.y;
  pvt[0]->uvw.x = cuv.x = tfc->tvec[tfc->cv->tvec_id].uv.x;
  pvt[0]->uvw.y = cuv.y = tfc->tvec[tfc->cv->tvec_id].uv.y;
//    pvt[0]->uvw.x = cuv.x = 0.0;
//    pvt[0]->uvw.y = cuv.y = 0.0;
  for ( i = 0; i < tfc->n_tedge * div; ++i ) {

    Vec sub;
//      sub.x = cuv.x - uvboundary[i].x;
//      sub.y = cuv.y - uvboundary[i].y;
    sub.x = uvboundary[i].x - cuv.x;
    sub.y = uvboundary[i].y - cuv.y;

    // �ŏ��̖ʂ̒T��
    Spfc *fc = remesh_find_intsecface( tfc, tfc->cv, &cuv, &(uvboundary[i]) );
    assert( fc != NULL );
    Sped *ed = NULL;
//      display("found. fc %d\n", fc->no );
    
//      Sped *ed = remesh_find_boundaryedge( tfc, lp, 
//  					 uvboundary[i].x, uvboundary[i].y );
//      assert( ed != NULL );
//      Spfc *fc = NULL;
//      if ( ed->lhe != NULL ) {
//        if ( ed->lhe->bpfc->tile_id == tfc->no ) fc = ed->lhe->bpfc;
//      }
//      if ( ed->rhe != NULL ) {
//        if ( ed->rhe->bpfc->tile_id == tfc->no ) fc = ed->rhe->bpfc;
//      }
//      assert( fc != NULL );
    
    //for ( int j = div - 1; j >= 1; --j ) {
    for ( int j = 1; j < div; ++j ) {
      
      int n = 1 + (div * i) + (j - 1);
//        double xparam = uvboundary[i].x + (double) j * sub.x / ddiv;
//        double yparam = uvboundary[i].y + (double) j * sub.y / ddiv;
      double xparam = cuv.x + (double) j * sub.x / ddiv;
      double yparam = cuv.y + (double) j * sub.y / ddiv;

      while ( TRUE ) {
	
	// xparam, yparam ���� fc �̒��ɓ����Ă���
	if ( remesh_isParamsInFace( tfc, fc, xparam, yparam ) == TRUE ) {
	  
	  // ���_�̍��W�l���v�Z
	  double a1, a2, a3;
	  remesh_ppdface_find_barycentric_coordinate( tfc, fc, xparam, yparam,
						      &a1, &a2, &a3 );
	  ppdface_barycentric_coordinate( fc, a1, a2, a3, &(pvt[n]->vec) );
//  	  display("ok. param %g %g fc %d\n", xparam, yparam, fc->no );
	  
	  // �p�����[�^�̊i�[
	  pvt[n]->uvw.x = xparam;
	  pvt[n]->uvw.y = yparam;
	  
	  break;
	  
	} else { // ���̖ʂɂ͂Ȃ��̂Ŏ��̖ʂ�
	  
	  // �ʂ̃G�b�W�̂����Ced �ł͂Ȃ� uvstart, uvend �ɂ���Ē�`�����
	  // �����ƌ����G�b�W��T��
	  if ( ed != NULL ) {
//  	    display("aa\n");
	    ed = remesh_nextedge( tfc, fc, ed, &(cuv), &(uvboundary[i]) );
	  } else {
//  	    display("bb\n");
	    ed = remesh_find_intsecedge( tfc, fc, tfc->cv );
	  }
	  assert( ed != NULL );
//  	  display("ed %d\n", ed->no );
	  
	  // fc �̔��Α��̖�
	  fc = ppdedge_another_fc( ed, fc );
	  assert( fc != NULL );
//  	  display("next fc %d\n", fc->no );
	  
	}
	
      }
    }
  }
    
  free( uvboundary );
  free( uvcorner );
  free( nn );
  free( pvt );
  free( pfc );
  
  return newppd;
}

Spfc *remesh_find_intsecface( TFace *tfc, Spvt *cv, Vec2d *v0, Vec2d *v1 )
{
  if ( cv == NULL ) return NULL;

  Sphe *he = cv->sphe;
  do {
    Spfc *fc = he->bpfc;

    Sphe *hhe = ppdface_opposite_halfedge( fc, cv );
    Sped *ed = hhe->ed;
    Vec2d *sv = &(tfc->tvec[ ed->sv->tvec_id ].uv);
    Vec2d *ev = &(tfc->tvec[ ed->ev->tvec_id ].uv);
    if ( isLineSegmentCrossing( sv, ev, v0, v1 ) == TRUE ) {
      return fc;
    }
//      do {
//        Sped *ed = hhe->ed;
//        Vec2d *sv = &(tfc->tvec[ ed->sv->tvec_id ].uv);
//        Vec2d *ev = &(tfc->tvec[ ed->ev->tvec_id ].uv);
//        if ( isLineSegmentCrossing( sv, ev, v0, v1 ) == TRUE ) {
//  	return fc;
//        }
//      } while ( (hhe = hhe->nxt) != fc->sphe );
    he = ppdvertex_ccwnext_halfedge( he );
  } while (  (he != cv->sphe) && (he != NULL) );

  return NULL;
}

Sped *remesh_find_intsecedge( TFace *tfc, Spfc *fc, Spvt *root )
{
  Sphe *he = ppdface_opposite_halfedge( fc, root );
  return he->ed;
}

//  Sphe *he = fc->sphe;
//    do {
//      Sped *ed = he->ed;
//      Vec2d *sv = &(tfc->tvec[ ed->sv->tvec_id ].uv);
//      Vec2d *ev = &(tfc->tvec[ ed->ev->tvec_id ].uv);
//      if ( isLineSegmentCrossing( sv, ev, v0, v1 ) == TRUE ) {
//        return ed;
//      }
//    } while ( (he = he->nxt) != fc->sphe );

//    return NULL;
//  }

Sped *remesh_find_boundaryedge( TFace *tfc, Splp *lp, double xparam, double yparam )
{
  for ( Splv *lv = lp->splv; lv != NULL; lv = lv->nxt ) {

    Spvt *vt0 = lv->vt;
    Spvt *vt1 = ( lv == lp->eplv ) ? lp->splv->vt : lv->nxt->vt;
    TVector *s = &(tfc->tvec[vt0->tvec_id]);
    TVector *e = &(tfc->tvec[vt1->tvec_id]);

    double xval = (xparam - s->uv.x) / ( e->uv.x - s->uv.x );
    double yval = (yparam - s->uv.y) / ( e->uv.y - s->uv.y );
//      display("xval %g yval %g\n", xval, yval );

    if ( (0.0 <= xval) && (1.0 >= xval) && (0.0 <= yval) && (1.0 >= yval) ) {
      Sped *ed = find_ppdedge_new( vt0, vt1 );
      return ed;
    }
  }
  return NULL;
}
  
Sped *remesh_nextedge( TFace *tfc, Spfc *fc, Sped *ed, Vec2d *v0, Vec2d *v1 )
{
  Sphe *he = fc->sphe;
  do {
    if ( he->ed == ed ) continue;

    Vec2d *sv = &(tfc->tvec[ he->ed->sv->tvec_id ].uv);
    Vec2d *ev = &(tfc->tvec[ he->ed->ev->tvec_id ].uv);
//      display("sv %g %g ev %g %g v0 %g %g v1 %g %g\n",
//  	    sv->x, sv->y, ev->x, ev->y, 
//  	    v0->x, v0->y, v1->x, v1->y );
    
    if ( isLineSegmentCrossing( sv, ev, v0, v1 ) == TRUE ) {
      
      return he->ed;
    }
    
  } while ( (he = he->nxt) != fc->sphe );

  return NULL;
}

BOOL remesh_isParamsInFace( TFace *tfc, Spfc *fc, double xparam, double yparam )
{
  Vec2d *uvw0, *uvw1, uvw;
  Sphe *he = fc->sphe;
  do {
    uvw0 = &(tfc->tvec[ he->vt->tvec_id ].uv);
    uvw1 = &(tfc->tvec[ he->nxt->vt->tvec_id ].uv);
    uvw.x = xparam; uvw.y = yparam;
    if ( isLeftSide( uvw0, uvw1, &uvw ) == FALSE ) return FALSE;
  } while ( (he = he->nxt) != fc->sphe );
  
  return TRUE;
}

// find barycentric coordinates
void remesh_ppdface_find_barycentric_coordinate( TFace *tfc, Spfc *fc,
						 double xparam, double yparam,
						 double *a1, double *a2, double *a3 )
{
  Vec2d vec;
  vec.x = xparam; vec.y = yparam;
  Vec2d *vec1 = &(tfc->tvec[ fc->sphe->vt->tvec_id].uv);
  Vec2d *vec2 = &(tfc->tvec[ fc->sphe->nxt->vt->tvec_id].uv);
  Vec2d *vec3 = &(tfc->tvec[ fc->sphe->nxt->nxt->vt->tvec_id].uv);

  double area = V2TriArea( vec1, vec2, vec3 );
  *a1 = V2TriArea( &vec, vec2, vec3 ) / area;
  *a2 = V2TriArea( &vec, vec3, vec1 ) / area;
  *a3 = V2TriArea( &vec, vec1, vec2 ) / area;
}


