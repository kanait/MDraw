//
// SRDialog.cpp
//
// Copyright (c) 1998-1999 Takashi Kanai
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
//

#include "stdafx.h"
#include "OpenGL.h"
#include "SRDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#include "smd.h"
#include "screen.h"

/////////////////////////////////////////////////////////////////////////////
// CSRDialog ダイアログ


CSRDialog::CSRDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CSRDialog::IDD, pParent)
{
  //{{AFX_DATA_INIT(CSRDialog)
  //}}AFX_DATA_INIT
}

BOOL CSRDialog::Create( void )
{
  return CDialog::Create( CSRDialog::IDD );
}

BOOL CSRDialog::OnInitDialog()
{
  CDialog::OnInitDialog();
  
  // TODO: この位置に初期化の補足処理を追加してください
  ScreenAtr *screen = &(swin->screenatr);
  m_radius_cyl = screen->rad_cyl;
  m_radius_sph = screen->rad_sph;
  
  return TRUE;
  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
  // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void CSRDialog::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CSRDialog)
  DDX_Text(pDX, IDC_RADIUS_CYL, m_radius_cyl);
  DDX_Text(pDX, IDC_RADIUS_SPH, m_radius_sph);
  //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSRDialog, CDialog)
  //{{AFX_MSG_MAP(CSRDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSRDialog メッセージ ハンドラ

void CSRDialog::OnOK() 
{
  // TODO: この位置にその他の検証用のコードを追加してください
	
  //	CDialog::OnOK();
  UpdateData(TRUE);

  ScreenAtr *screen;
  screen = &(swin->screenatr);
  screen->wnd = GetTopLevelParent();
  screen->rad_sph = m_radius_sph;
  screen->rad_cyl = m_radius_cyl;
  screen->wnd->RedrawWindow();

}

void CSRDialog::OnCancel() 
{
  // TODO: この位置に特別な後処理を追加してください。
	
  //	CDialog::OnCancel();
  DestroyWindow();
}

void CSRDialog::PostNcDestroy() 
{
  // TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
	
  //	CDialog::PostNcDestroy();
  delete this;
}

