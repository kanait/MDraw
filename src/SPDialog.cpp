// SPDialog.cpp : �C���v�������e�[�V���� �t�@�C��
//

#include "stdafx.h"
#include "OpenGL.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "smd.h"
#include "screen.h"

#include "ppdvertex.h"
#include "ppdedge.h"
#include "ppdface.h"
#include "ppdloop.h"
#include "edit.h"
#include "sellist.h"
#include "sgraph.h"
#include "esp.h"

#include "SPDialog.h"

/////////////////////////////////////////////////////////////////////////////
// CSPDialog �_�C�A���O

CSPDialog::CSPDialog(CWnd* pParent /*=NULL*/)
  : CDialog(CSPDialog::IDD, pParent)
{

  //{{AFX_DATA_INIT(CSPDialog)
  m_sp_dist_data = _T("");
  m_sp_length_data = _T("");
  m_sp_src_data = _T("");
  m_sp_gamma_data = _T("");
  m_sg_edge_data = _T("");
  m_sg_vertex_data = _T("");
  m_esp_length_data = _T("");
  m_asp_time_data = _T("");
  m_esp_time_data = _T("");
  m_obj_vertex_data = _T("");
  m_obj_edge_data = _T("");
  m_obj_face_data = _T("");
  m_sp_avrsp_data = _T("");
  //}}AFX_DATA_INIT
}

// �_�C�A���O���J��
BOOL CSPDialog::Create( void )
{
  // TODO: ���̈ʒu�ɌŗL�̏�����ǉ����邩�A�܂��͊�{�N���X���Ăяo���Ă�������
	
  ScreenAtr *screen;
  screen = &(swin->screenatr);
  Sppd *ppd;
  ppd = screen->current_ppd;

  double gamma = GAMMA * ppd->scale;
  // create sub-graph
  if ( ppd != NULL ) {
    if ( ppd->sg == NULL ) {
      ppd->sg = initialize_sgraph( ppd, gamma );
    }
  }

  return CDialog::Create( CSPDialog::IDD );
}

// �����Ń_�C�A���O�̃{�b�N�X�̒l������������
BOOL CSPDialog::OnInitDialog() 
{
  CDialog::OnInitDialog();
	
  // TODO: ���̈ʒu�ɏ������̕⑫������ǉ����Ă�������
	
  ScreenAtr *screen;
  screen = &(swin->screenatr);
  Sppd *ppd;
  ppd = screen->current_ppd;

  //
  // ppd numbers
  //
  // vertex
  CString string;
  string.Format( "%d", num_ppdvertex( ppd ) );
  m_obj_vertex.SetWindowText( string );
  string.Format( "%d", num_ppdedge( ppd ) );
  m_obj_edge.SetWindowText( string );
  string.Format( "%d", num_ppdface( ppd ) );
  m_obj_face.SetWindowText( string );

  // sub-graph informations
  // set the number of vertices in the sub-graph 
  string.Format( "%d", ppd->sg->sgvtn );
  m_sg_vertex.SetWindowText( string );
  // set the number of edges in the sub-graph
  string.Format( "%d", ppd->sg->sgedn );
  m_sg_edge.SetWindowText( string );
  // set gamma value
  string.Format( "%g", GAMMA * ppd->scale );
  m_sp_gamma.SetWindowText( string );
  // set average number of SP per edge
  string.Format( "%g", ppd->sg->avrsp );
  m_sp_avrsp.SetWindowText( string );

  swin->edit_type = EDIT_CREATE_SPATH;

  // path type initialize
  ResetPathType();
  m_approx.SetCheck(1);
  swin->path_type = PATH_APPROX;

  // �R���g���[���Ƀt�H�[�J�X��ݒ肵�Ȃ��Ƃ��A�߂�l�� TRUE �ƂȂ�܂�
  // ��O: OCX �v���p�e�B �y�[�W�̖߂�l�� FALSE �ƂȂ�܂�
  return TRUE;
}


void CSPDialog::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  
  //{{AFX_DATA_MAP(CSPDialog)
  DDX_Control(pDX, IDC_TXT_AVRSP, m_sp_avrsp);
  DDX_Control(pDX, IDC_OBJ_FACE, m_obj_face);
  DDX_Control(pDX, IDC_OBJ_EDGE, m_obj_edge);
  DDX_Control(pDX, IDC_OBJ_VTX, m_obj_vertex);
  DDX_Control(pDX, IDC_ESP_TIME, m_esp_time);
  DDX_Control(pDX, IDC_ASP_TIME, m_asp_time);
  DDX_Control(pDX, IDC_ESP_LENGTH, m_esp_length);
  DDX_Control(pDX, IDC_RADIO_SP_EXACT, m_exact);
  DDX_Control(pDX, IDC_RADIO_SP_BOTH, m_both);
  DDX_Control(pDX, IDC_RADIO_SP_APPROX, m_approx);
  DDX_Control(pDX, IDC_TXT_SGV, m_sg_vertex);
  DDX_Control(pDX, IDC_TXT_SGE, m_sg_edge);
  DDX_Control(pDX, IDC_TXT_GAMMA, m_sp_gamma);
  DDX_Control(pDX, IDC_CSP_SRC, m_sp_src);
  DDX_Control(pDX, IDC_CSP_LENGTH, m_sp_length);
  DDX_Control(pDX, IDC_CSP_DIST, m_sp_dist);
  DDX_Text(pDX, IDC_CSP_DIST, m_sp_dist_data);
  DDX_Text(pDX, IDC_CSP_LENGTH, m_sp_length_data);
  DDX_Text(pDX, IDC_CSP_SRC, m_sp_src_data);
  DDX_Text(pDX, IDC_TXT_GAMMA, m_sp_gamma_data);
  DDX_Text(pDX, IDC_TXT_SGE, m_sg_edge_data);
  DDX_Text(pDX, IDC_TXT_SGV, m_sg_vertex_data);
  DDX_Text(pDX, IDC_ESP_LENGTH, m_esp_length_data);
  DDX_Text(pDX, IDC_ASP_TIME, m_asp_time_data);
  DDX_Text(pDX, IDC_ESP_TIME, m_esp_time_data);
  DDX_Text(pDX, IDC_OBJ_VTX, m_obj_vertex_data);
  DDX_Text(pDX, IDC_OBJ_EDGE, m_obj_edge_data);
  DDX_Text(pDX, IDC_OBJ_FACE, m_obj_face_data);
  DDX_Text(pDX, IDC_TXT_AVRSP, m_sp_avrsp_data);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSPDialog, CDialog)
  //{{AFX_MSG_MAP(CSPDialog)
  ON_BN_CLICKED(IDCLEAR, OnClear)
  ON_BN_CLICKED(IDC_RADIO_SP_APPROX, OnRadioSpApprox)
  ON_BN_CLICKED(IDC_RADIO_SP_EXACT, OnRadioSpExact)
  ON_BN_CLICKED(IDC_RADIO_SP_BOTH, OnRadioSpBoth)
	ON_BN_CLICKED(IDC_SP_GAMMA_SET, OnSpGammaSet)
	//}}AFX_MSG_MAP
  END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSPDialog ���b�Z�[�W �n���h��

void CSPDialog::OnOK() 
{
  // TODO: ���̈ʒu�ɂ��̑��̌��ؗp�̃R�[�h��ǉ����Ă�������

  // �_�C�A���O�������Ȃ�
  // CDialog::OnOK();

  ScreenAtr *screen;
  screen = &(swin->screenatr);

  if ( screen->n_sellist != 2 ) {
    AfxMessageBox("Can't create a shortest path.");
    FreeSelectList( screen );
    screen->wnd->RedrawWindow();
    return;
  }

  CString string;
  Sppd *ppd;
  ppd = screen->current_ppd;

  switch ( swin->path_type ) {

    // approximate shortest path
  case PATH_APPROX:

    //
    // Get path
    //
    if ( swin->dis3d.spath_anim == SMD_ON ) {
      screen->wnd = GetTopLevelParent();
    }
    if ( asp_create_spath( screen ) == FALSE ) {
      AfxMessageBox("Can't create an approximate shortest path.");
    }

    //
    // length
    //
    string.Format( "%g", screen->alp->length );
    m_sp_length.SetWindowText( string );

    //
    // time
    //
    string.Format( "%g", screen->alp->usertime );
    m_asp_time.SetWindowText( string );
    break;

    // exact shortest path (Chen & Han algorithm)
  case PATH_EXACT:

    //
    // Get path
    //
    if ( esp_create_spath( screen ) == FALSE ) {
      AfxMessageBox("Can't create an exact shortest path.");
      break;
    }

    //
    // length
    //
    string.Format( "%g", screen->elp->length );
    m_esp_length.SetWindowText( string );

    //
    // time
    //
    string.Format( "%g", screen->elp->usertime );
    m_esp_time.SetWindowText( string );
    
    break;

    // both
  case PATH_BOTH:

    //
    // Get path
    //
    // Approx. 
    if ( swin->dis3d.spath_anim == SMD_ON ) {
      screen->wnd = GetTopLevelParent();
    }
    if ( asp_create_spath( screen ) == FALSE ) {
      AfxMessageBox("Can't create an approximate shortest path.");
    }
    // Exact
    if ( esp_create_spath( screen ) == FALSE ) {
      AfxMessageBox("Can't create an exact shortest path.");
      break;
    }

    // 
    // length
    //
    // Approx.
    string.Format( "%g", screen->alp->length );
    m_sp_length.SetWindowText( string );
    // Exact
    string.Format( "%g", screen->elp->length );
    m_esp_length.SetWindowText( string );
    //	}

    //
    // time
    //
    // Approx.
    string.Format( "%g", screen->alp->usertime );
    m_asp_time.SetWindowText( string );
    // Exact
    string.Format( "%g", screen->elp->usertime );
    m_esp_time.SetWindowText( string );
    
    break;

  }

  FreeSelectList( screen );
  GetTopLevelParent()->RedrawWindow();

  // src, dist
  // m_sp_src.SetWindowText("");
  // m_sp_dist.SetWindowText("");
}

void CSPDialog::OnCancel()
{
  // TODO: ���̈ʒu�ɓ��ʂȌ㏈����ǉ����Ă��������B
	
  //	CDialog::OnCancel();
  FreeSelectList( &(swin->screenatr) );
  //	FreeSelectList( &(swin->screenatr[SCREEN_DIST]) );

  Sppd *ppd;
  ppd = swin->screenatr.current_ppd;
  if ( ppd != NULL ) {
    if ( ppd->sg != NULL ) {
      free_sgraph( ppd->sg );
    }
    ppd->sg = NULL;
  }

  swin->edit_type = EDIT_NONE;

  DestroyWindow();

}

void CSPDialog::PostNcDestroy() 
{
  // TODO: ���̈ʒu�ɌŗL�̏�����ǉ����邩�A�܂��͊�{�N���X���Ăяo���Ă�������
	
  //	CDialog::PostNcDestroy();
  FreeSelectList( &(swin->screenatr) );
  //	FreeSelectList( &(swin->screenatr[SCREEN_DIST]) );

  cspd = NULL;

  ScreenAtr *screen;
  screen = &(swin->screenatr);

  if ( screen->view_sg != NULL ) {
	free_sgraph( screen->view_sg );
    screen->view_sg = NULL;
    screen->view_sglp = NULL;
  }

  Sppd *ppd;
  ppd = screen->current_ppd;
  if ( ppd != NULL ) {
    if ( ppd->sg != NULL ) {
      free_sgraph( ppd->sg );
    }
    ppd->sg = NULL;
  }

  delete this;
}

void CSPDialog::OnClear() 
{
  // TODO: ���̈ʒu�ɃR���g���[���ʒm�n���h���p�̃R�[�h��ǉ����Ă�������
	
  FreeSelectList( &(swin->screenatr) );

  m_sp_src.SetWindowText("");
  m_sp_dist.SetWindowText("");
  //	m_sp_length.SetSel(0, -1);
  //	m_sp_length.SetWindowText("");

  GetTopLevelParent()->Invalidate();
}

//
// Path-type radio button functions
//
void CSPDialog::ResetPathType( void )
{
  m_approx.SetCheck(0);
  m_exact.SetCheck(0);
  m_both.SetCheck(0);
}

void CSPDialog::OnRadioSpApprox() 
{
  // TODO: ���̈ʒu�ɃR���g���[���ʒm�n���h���p�̃R�[�h��ǉ����Ă�������
  ResetPathType();
  swin->path_type = PATH_APPROX;
  m_approx.SetCheck(1);
}

void CSPDialog::OnRadioSpExact() 
{
  // TODO: ���̈ʒu�ɃR���g���[���ʒm�n���h���p�̃R�[�h��ǉ����Ă�������
  ResetPathType();
  swin->path_type = PATH_EXACT;
  m_exact.SetCheck(1);
}

void CSPDialog::OnRadioSpBoth() 
{
  // TODO: ���̈ʒu�ɃR���g���[���ʒm�n���h���p�̃R�[�h��ǉ����Ă�������
  ResetPathType();
  swin->path_type = PATH_BOTH;
  m_both.SetCheck(1);
}

// Progress Control
// void CSPDialog::esp_progress_setrange( int fn )
// {
//   m_esp_progress.SetRange( 0, fn - 1 );
//   m_esp_progress.SetStep( 1 );
//   m_esp_progress.SetPos( 0 );
// }

// void CSPDialog::esp_progress_stepit( void )
// {
//   m_esp_progress.StepIt();
// }

void CSPDialog::OnSpGammaSet() 
{
  // TODO: ���̈ʒu�ɃR���g���[���ʒm�n���h���p�̃R�[�h��ǉ����Ă�������

  ScreenAtr *screen;
  screen = &(swin->screenatr);
  Sppd *ppd;
  ppd = screen->current_ppd;
  if ( ppd == NULL ) return;
  
  CString string;
  m_sp_gamma.GetWindowText( string );
  LPTSTR lpszbuffer= string.GetBuffer( BUFSIZ );
  double ngamma = atof( lpszbuffer );
  
//   string.Format( "gamma_new %g", gamma_new );
//   AfxMessageBox( string );

  if ( ppd->sg != NULL ) {
    free_sgraph( ppd->sg );
    ppd->sg = NULL;
  }
  ppd->sg = initialize_sgraph( ppd, ngamma );

  // set the number of vertices in the sub-graph 
  string.Format( "%d", ppd->sg->sgvtn );
  m_sg_vertex.SetWindowText( string );
  // set the number of edges in the sub-graph
  string.Format( "%d", ppd->sg->sgedn );
  m_sg_edge.SetWindowText( string );

  string.Format( "%g", ppd->sg->avrsp );
  m_sp_avrsp.SetWindowText( string );  
}
