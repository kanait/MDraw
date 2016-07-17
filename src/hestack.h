//
// hestack.h
//
// Copyright (c) 1999-2000 Takashi Kanai; All rights reserved. 
//

#ifndef _HESTACK_H
#define _HESTACK_H

//
// data structure for vertices on halfedges
// (����͂��Ȃ����ȃf�[�^�\���Q�ł�)

// used in tface_subdivide_ppdface
// ��������O�̖ʂ��i�[���邽�߂̃f�[�^�\��
typedef struct _subdivface {

  // �����̑ΏۂƂȂ��
  Spfc *fc;

  // �����ɂ����G�b�W(�ő�� 4 �{)
  int en;
  Sped *ed[4]; // �ő�� 4 �{

  // �������ꂽ�� (�ő�� 5 ��)
  //int fn;
  //Spfc *new_fc[5];

  // ��������O�̖�
  Spfc* prev_fc;
  
} SubdivFace;

// used in tface_subdivide_ppdface
// ��������O�̃G�b�W���i�[���邽�߂̃f�[�^�\��
typedef struct _subdivedge {

  // �����̑ΏۂƂȂ�G�b�W
  Sped *ed;

  // �����Ɏg�����_ (�ő�� 4 ��)
  int vn;
  Spvt *vt[4]; // �ő�� 4 ��

  // �������ꂽ�G�b�W (�ő�� 5 �{)
  //int en;
  //Sped *new_ed[5];

} SubdivEdge;

typedef struct _hestack Hestack;
typedef struct _hevt Hevt;
typedef struct _hevted Hevted;

struct _hestack {
  
  // �ʂ̃n�[�t�G�b�W�̐������� Hevt (�|�C���^�z��)
  Sphe *he;
  Hevt *hevt;
  
  int hevtn;
};
  
struct _hevt {
  // ID
  int hsid;
  int hvid;

  // ���_
  Spvt *vt;

  // ���������G�b�W�ւ̃����N�i����Ȃ��j
  //Sped *ed;
  // �\�[�g�ɕK�v�Ȓl
  double val;
  // �����ɂ����Ďg��ꂽ���ǂ���
  //  BOOL isUsed;

  // �G�b�W�ւ̃����N�i�o�������񃊃X�g�j
  int nhve;
  Hevted *svted, *evted;
};

// ���_����G�b�W�ւ̃����N�ƂȂ�o�������񃊃X�g�m�[�h
// �����v����
struct _hevted {

  // �l (�ꉞ)
  double val;
  
  // �G�b�W
  Sped *ed;

  // �o�b�N�|�C���^
  Hevt *hevt;
  
  // ���C�g
  Hevted *mate;

  // �����ɂ����Ďg��ꂽ���ǂ���
  BOOL isVisited;
  
  // �O��̃m�[�h
  Hevted *nxt, *prv;
};
  
// hestack.cxx
extern void subdivide_ppdface( SubdivFace *, Sppd *, int*, SubdivFace*, int*, SubdivEdge* );
extern void initialize_hestack( SubdivFace *, Hestack *, Sppd *, int*, SubdivEdge* );
extern int find_henum_hestack( Sped *, int, Hestack * );
extern Hevt *next_hevt_triloop( Hevt *, int, Hestack * );
extern void subdivide_ppdface_by_hestack( Spfc *, Hestack *, Sppd *, int*, SubdivFace* );
extern BOOL isvalid_create_ppdface( Hevted *, Hevt * );
extern void initialize_hevt( Hevt * );
extern void insert_hevt_in_hestack( Spvt *, Sped *, Hestack * );
extern Hevt *find_hevt_from_hestack( Spvt *, int, Hestack * );
extern Hevted *create_hevted( Sped *, Hevt * );
extern Hevted *find_hevted( Sped *, Hevt * );
extern Hevted *insert_hevted( double, Sped *, Hevt * );
extern void free_halfedgevertexedge( Hevt * );
extern void free_hevted( Hevted *, Hevt * );
extern void hevted_makemate( int, Hestack * );

extern void init_subdivface( SubdivFace* );
extern SubdivFace* find_subdivface( int, SubdivFace*, Spfc* );
extern void init_subdivedge( SubdivEdge* );
extern SubdivEdge* find_subdivedge( int, SubdivEdge*, Sped* );


#endif // _HESTACK_H
