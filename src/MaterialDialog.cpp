//
// MaterialDialog.cpp
//
// Copyright (c) 1998-1999 Takashi Kanai
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
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
// CMaterialDialog ダイアログ

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
// CMaterialDialog メッセージ ハンドラ

BOOL CMaterialDialog::Create()
{
  // TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
  return CDialog::Create( CMaterialDialog::IDD );
}

BOOL CMaterialDialog::OnInitDialog() 
{
  CDialog::OnInitDialog();
	
  // TODO: この位置に初期化の補足処理を追加してください
	
  for ( int i = MTLNUM - 1; i >= 0; --i ) 
    m_material_list.InsertString( 0, mtltxt[i] );

  return TRUE;  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
  // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void CMaterialDialog::OnCancel() 
{
  // TODO: この位置に特別な後処理を追加してください。
  // CDialog::OnCancel();

  DestroyWindow();
}

void CMaterialDialog::PostNcDestroy() 
{
  // TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
  //CDialog::PostNcDestroy();
  
  md = NULL;
  delete this;
}

void CMaterialDialog::OnMatlPpdCheck()
{
  // TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
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
  // TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
  int sel = m_material_list.GetCurSel();

  if ( m_matl_ppd == TRUE ) {
    Sppd *ppd = swin->screenatr.current_ppd;
    copy_material( &(ppd->matl), sel, (float *) mtlall );
  }

  GetTopLevelParent()->Invalidate();
  
}


