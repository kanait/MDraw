// MaterialDialog.cpp : �C���v�������e�[�V���� �t�@�C��
//

#include "stdafx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "OpenGL.h"

#include "smd.h"
#include "screen.h"
#include "file.h"

#include "mtl.h"

#include "MaterialDialog.h"

extern CMaterialDialog *md;

/////////////////////////////////////////////////////////////////////////////
// CMaterialDialog �_�C�A���O

CMaterialDialog::CMaterialDialog(CWnd* pParent /*=NULL*/)
  : CDialog(CMaterialDialog::IDD, pParent)
{
  //{{AFX_DATA_INIT(CMaterialDialog)
  m_matl_ppd = FALSE;
  //}}AFX_DATA_INIT
}


void CMaterialDialog::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CMaterialDialog)
	DDX_Control(pDX, IDC_MATERIAL_LIST, m_material_list);
	DDX_Check(pDX, IDC_MATL_PPD_CHECK, m_matl_ppd);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMaterialDialog, CDialog)
	//{{AFX_MSG_MAP(CMaterialDialog)
	ON_BN_CLICKED(IDC_MATL_PPD_CHECK, OnMatlPpdCheck)
	ON_LBN_DBLCLK(IDC_MATERIAL_LIST, OnDblclkMaterialList)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMaterialDialog ���b�Z�[�W �n���h��

BOOL CMaterialDialog::Create()
{
  // TODO: ���̈ʒu�ɌŗL�̏�����ǉ����邩�A�܂��͊�{�N���X���Ăяo���Ă�������
  return CDialog::Create( CMaterialDialog::IDD );
}

BOOL CMaterialDialog::OnInitDialog() 
{
  CDialog::OnInitDialog();
	
  // TODO: ���̈ʒu�ɏ������̕⑫������ǉ����Ă�������
	
  for ( int i = MTLNUM - 1; i >= 0; --i ) 
    m_material_list.InsertString( 0, mtltxt[i] );

  return TRUE;  // �R���g���[���Ƀt�H�[�J�X��ݒ肵�Ȃ��Ƃ��A�߂�l�� TRUE �ƂȂ�܂�
  // ��O: OCX �v���p�e�B �y�[�W�̖߂�l�� FALSE �ƂȂ�܂�
}

void CMaterialDialog::OnCancel() 
{
  // TODO: ���̈ʒu�ɓ��ʂȌ㏈����ǉ����Ă��������B
  // CDialog::OnCancel();

  DestroyWindow();
}

void CMaterialDialog::PostNcDestroy() 
{
  // TODO: ���̈ʒu�ɌŗL�̏�����ǉ����邩�A�܂��͊�{�N���X���Ăяo���Ă�������
  //CDialog::PostNcDestroy();
  
  md = NULL;
  delete this;
}

void CMaterialDialog::OnMatlPpdCheck()
{
  // TODO: ���̈ʒu�ɃR���g���[���ʒm�n���h���p�̃R�[�h��ǉ����Ă�������
  //display("matl ppd %d\n", m_matl_ppd );
  if ( m_matl_ppd == TRUE ) {
    m_matl_ppd = FALSE;
  } else {
    m_matl_ppd = TRUE;
  }
  //display("matl ppd %d\n", m_matl_ppd );
}

void CMaterialDialog::OnDblclkMaterialList()
{
  // TODO: ���̈ʒu�ɃR���g���[���ʒm�n���h���p�̃R�[�h��ǉ����Ă�������
  int sel = m_material_list.GetCurSel();

  if ( m_matl_ppd == TRUE ) {
    Sppd *ppd = swin->screenatr.current_ppd;
    copy_material( &(ppd->matl), sel, (float *) mtlall );
  }

  GetTopLevelParent()->Invalidate();
  
}

