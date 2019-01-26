//
// MeshInfoDialog.cpp
//
// Copyright (c) 1998-1999 Takashi Kanai
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
//

#include "stdafx.h"
#include "OpenGL.h"

#include "smd.h"
#include "screen.h"

#include "MeshInfoDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CMeshInfoDialog *midlg;

/////////////////////////////////////////////////////////////////////////////
// CMeshInfoDialog ダイアログ


CMeshInfoDialog::CMeshInfoDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CMeshInfoDialog::IDD, pParent)
{
  //{{AFX_DATA_INIT(CMeshInfoDialog)
  //}}AFX_DATA_INIT

}


void CMeshInfoDialog::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CMeshInfoDialog)
  DDX_Control(pDX, IDC_MESH_INFO_EDIT, m_mesh_info);
  //}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMeshInfoDialog, CDialog)
	//{{AFX_MSG_MAP(CMeshInfoDialog)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMeshInfoDialog メッセージ ハンドラ

BOOL CMeshInfoDialog::Create( void )
{
  // TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
  return CDialog::Create( CMeshInfoDialog::IDD );
}

BOOL CMeshInfoDialog::OnInitDialog() 
{
  CDialog::OnInitDialog();
	
  // TODO: この位置に初期化の補足処理を追加してください

  Sppd *ppd = swin->screenatr.current_ppd;
  CString string;
  string.Format("ppd\n\nvertices:\t%d\nfaces:\t%d\nedges:\t%d\n",
		ppd->vn, ppd->fn, ppd->en); 
  m_mesh_info.FmtLines( TRUE );
  m_mesh_info.SetWindowText( string );
  

  return TRUE;
  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
  // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void CMeshInfoDialog::PostNcDestroy() 
{
  // TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
	
  //CDialog::PostNcDestroy();

  midlg = NULL;
  delete this;
}

void CMeshInfoDialog::OnOK() 
{
  // TODO: この位置にその他の検証用のコードを追加してください
	
  //CDialog::OnOK();
  DestroyWindow();
}

