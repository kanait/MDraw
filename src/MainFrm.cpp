//
// MainFrm.cpp : CMainFrame クラスの動作の定義を行います。
//

#include "stdafx.h"
#include "OpenGL.h"

#include "MainFrm.h"
#include "OpenGLDoc.h"
#include "OpenGLView.h"
//#include "DistView.h"

#include "smd.h"
#include "screen.h"
#include "mtl.h"

#include "ppd.h"
#include "ppdlist.h"
#include "ppdface.h"
#include "ppdnormal.h"
#include "ppdloop.h"
#include "smf.h"
#include "mdw.h"
#include "file.h"
#include "hidden.h"
#include "esp.h"
#include "sgraph.h"
#include "veclib.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNCREATE(CMainFrame, CFrameWnd)

  BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
  //{{AFX_MSG_MAP(CMainFrame)
  ON_WM_CREATE()
  ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
  ON_UPDATE_COMMAND_UI(ID_MENU_SCREEN_COAXIS, OnUpdateMenuScreenCoaxis)
  ON_UPDATE_COMMAND_UI(ID_MENU_SCREEN_LOOP, OnUpdateMenuScreenLoop)
  ON_UPDATE_COMMAND_UI(ID_MENU_SCREEN_SHADING, OnUpdateMenuScreenShading)
  ON_UPDATE_COMMAND_UI(ID_MENU_SCREEN_SPATH, OnUpdateMenuScreenSpath)
  ON_UPDATE_COMMAND_UI(ID_MENU_SCREEN_WIREFRAME, OnUpdateMenuScreenWireframe)
  ON_COMMAND(ID_MENU_SCREEN_SHADING, OnMenuScreenShading)
  ON_COMMAND(ID_MENU_SCREEN_WIREFRAME, OnMenuScreenWireframe)
  ON_COMMAND(ID_MENU_SCREEN_COAXIS, OnMenuScreenCoaxis)
  ON_COMMAND(ID_MENU_SCREEN_VERT, OnMenuScreenVert)
  ON_UPDATE_COMMAND_UI(ID_MENU_SCREEN_VERT, OnUpdateMenuScreenVert)
  ON_COMMAND(ID_FILE_SAVE_SRC, OnFileSaveSrc)
  ON_COMMAND(ID_MENU_SCREEN_LOOP, OnMenuScreenLoop)
  ON_COMMAND(ID_MENU_SCREEN_SPATH, OnMenuScreenSpath)
  ON_UPDATE_COMMAND_UI(ID_MENU_SCREEN_ENHANCED, OnUpdateMenuScreenEnhanced)
  ON_COMMAND(ID_MENU_SCREEN_ENHANCED, OnMenuScreenEnhanced)
  ON_COMMAND(ID_MENU_SCREEN_SMOOTH, OnMenuScreenSmooth)
  ON_UPDATE_COMMAND_UI(ID_MENU_SCREEN_SMOOTH, OnUpdateMenuScreenSmooth)
  ON_COMMAND(ID_MENU_SCREEN_RADIUS, OnMenuScreenRadius)
  ON_COMMAND(ID_VIEW_BACKGROUND, OnViewBackgroundColor)
  ON_COMMAND(ID_MENU_SPATH_ANIM, OnMenuSpathAnim)
  ON_UPDATE_COMMAND_UI(ID_MENU_SPATH_ANIM, OnUpdateMenuSpathAnim)
  ON_COMMAND(ID_MENU_SCREEN_VERTID, OnMenuScreenVertid)
  ON_UPDATE_COMMAND_UI(ID_MENU_SCREEN_VERTID, OnUpdateMenuScreenVertid)
  ON_COMMAND(ID_MENU_SCREEN_EDGEID, OnMenuScreenEdgeid)
  ON_UPDATE_COMMAND_UI(ID_MENU_SCREEN_EDGEID, OnUpdateMenuScreenEdgeid)
  ON_COMMAND(ID_FILE_OPEN_SP, OnFileOpenSp)
  ON_COMMAND(ID_MENU_SCREEN_SUBGRAPH, OnMenuScreenSubgraph)
  ON_UPDATE_COMMAND_UI(ID_MENU_SCREEN_SUBGRAPH, OnUpdateMenuScreenSubgraph)
  ON_COMMAND(ID_FILE_OPEN_VW, OnFileOpenVw)
  ON_COMMAND(ID_FILE_SAVE_VW, OnFileSaveVw)
  ON_COMMAND(ID_MENU_SPATH_CREATE, OnMenuSpathCreate)
  ON_COMMAND(ID_MENU_EDIT_TILE, OnMenuEditTile)
  ON_COMMAND(ID_VIEW_MATERIAL, OnViewMaterial)
  ON_COMMAND(ID_MENU_FAIRING_SP, OnMenuFairingSp)
  ON_COMMAND(ID_MESH_INFO, OnMeshInfo)
  ON_COMMAND(ID_VIEW_GRADIENT, OnViewGradient)
  ON_UPDATE_COMMAND_UI(ID_VIEW_GRADIENT, OnUpdateViewGradient)
  ON_COMMAND(ID_MENU_SCREEN_LIGHT0, OnMenuScreenLight0)
  ON_UPDATE_COMMAND_UI(ID_MENU_SCREEN_LIGHT0, OnUpdateMenuScreenLight0)
  ON_COMMAND(ID_MENU_SCREEN_LIGHT1, OnMenuScreenLight1)
  ON_UPDATE_COMMAND_UI(ID_MENU_SCREEN_LIGHT1, OnUpdateMenuScreenLight1)
  ON_COMMAND(ID_MENU_SCREEN_LIGHT2, OnMenuScreenLight2)
  ON_UPDATE_COMMAND_UI(ID_MENU_SCREEN_LIGHT2, OnUpdateMenuScreenLight2)
  ON_COMMAND(ID_MENU_SCREEN_LIGHT3, OnMenuScreenLight3)
  ON_UPDATE_COMMAND_UI(ID_MENU_SCREEN_LIGHT3, OnUpdateMenuScreenLight3)
  ON_COMMAND(ID_FILE_OPEN_SMF, OnFileOpenSmf)
  ON_COMMAND(ID_FILE_SAVE_SMF, OnFileSaveSmf)
  ON_COMMAND(ID_FILE_OPEN_MDW, OnFileOpenMdw)
  ON_COMMAND(ID_FILE_SAVE_MDW, OnFileSaveMdw)
  ON_COMMAND(ID_FILE_SAVEAS_MDW, OnFileSaveasMdw)
  ON_COMMAND(ID_MENU_SCREEN_REMESH, OnMenuScreenRemesh)
  ON_UPDATE_COMMAND_UI(ID_MENU_SCREEN_REMESH, OnUpdateMenuScreenRemesh)
  ON_COMMAND(ID_MENU_SCREEN_REMESHBOUN, OnMenuScreenRemeshboun)
  ON_UPDATE_COMMAND_UI(ID_MENU_SCREEN_REMESHBOUN, OnUpdateMenuScreenRemeshboun)
  ON_COMMAND(ID_MENU_SCREEN_GRPMESH, OnMenuScreenGrpmesh)
  ON_UPDATE_COMMAND_UI(ID_MENU_SCREEN_GRPMESH, OnUpdateMenuScreenGrpmesh)
  ON_COMMAND(ID_MENU_SCREEN_HIDDEN, OnMenuScreenHidden)
  ON_UPDATE_COMMAND_UI(ID_MENU_SCREEN_HIDDEN, OnUpdateMenuScreenHidden)
  //}}AFX_MSG_MAP

  END_MESSAGE_MAP()

  static UINT indicators[] =
{
  ID_SEPARATOR,           // ステータス ライン インジケータ
  ID_INDICATOR_KANA,
  ID_INDICATOR_CAPS,
  ID_INDICATOR_NUM,
  ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame クラスの構築/消滅

CMainFrame::CMainFrame()
{
  // TODO: この位置にメンバの初期化処理コードを追加してください。
  Swin *create_swin( void );

  swin = create_swin();
}

CMainFrame::~CMainFrame()
{
  void free_swin( Swin * );

  free_swin( swin );
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
  if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
    return -1;
	
  // StatusBar
  if (!m_wndStatusBar.Create(this) ||
      !m_wndStatusBar.SetIndicators(indicators,
				    sizeof(indicators)/sizeof(UINT)))
    {
      TRACE0("Failed to create status bar\n");
      return -1;      // 作成に失敗
    }

  // TODO: もしツール チップスが必要ない場合、ここを削除してください。
  //	m_wndToolBar.SetBarStyle(m_wndToolBar.GetBarStyle() |
  //		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);

  // TODO: ツール バーをドッキング可能にしない場合は以下の３行を削除
  //       してください。
  //	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
  //	EnableDocking( CBRS_ALIGN_ANY );
  //	DockControlBar( &m_wndToolBar );

  return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
  // TODO: この位置で CREATESTRUCT cs を修正して、Window クラスやスタイルを
  //       修正してください。

  // window size
  cs.cx = swin->width;
  cs.cy = swin->height;

// リサイズ不可
//	cs.style &= ~WS_MAXIMIZEBOX;
//	cs.style &= ~WS_THICKFRAME;

  return CFrameWnd::PreCreateWindow( cs );
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame クラスの診断

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
  CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
  CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame メッセージ ハンドラ

static char BASED_CODE szppdFilter[] = "PPD Files (*.ppd)|*.ppd|All Files (*.*)|*.*||";

//
// open PPD file
//
void CMainFrame::OnFileOpen() 
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください

  // 第一引数は OPEN のとき TRUE
  // 第二、三引数はファイルの種類
  CFileDialog filedlg( TRUE, NULL, NULL,
		       OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
		       szppdFilter, NULL );

  if( filedlg.DoModal() == IDOK) {
    //		CWaitCursor wait;
    CString filename;
    filename = filedlg.GetPathName();
    //filenameに選択したファイルのフルパスが入る

    char *lp;
    lp = filename.GetBuffer( BUFSIZ );
    Sppd *ppd;
    if ( (ppd = open_ppd( lp, TRUE )) == NULL )
      {
	AfxMessageBox("Filename was bad!");
	return;
      }

    // set material (default)
    copy_material( &(ppd->matl), 0, (float *) mtlall );
		
    if ( swin->screenatr.current_ppd != NULL ) {
      free_ppd( swin->screenatr.current_ppd );
      swin->screenatr.view_ppd = NULL;
    }

    swin->screenatr.current_ppd = ppd;
    swin->screenatr.view_ppd = ppd;
			
    // normalization
    // enhanced
    swin->screenatr.rad_sph *= ppd->scale;
    swin->screenatr.rad_cyl *= ppd->scale;

    // gamma の決定
    swin->screenatr.sp_gamma = GAMMA * ppd->scale;
    
    InvalidateRect( NULL, FALSE );
  }
	
}

//
// save PPD file
//
void CMainFrame::OnFileSaveSrc()
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください
  CFileDialog filedlg( FALSE, NULL, NULL,
		       OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
		       szppdFilter, NULL );

  if( filedlg.DoModal() == IDOK) {
    //		CWaitCursor wait;
    CString filename;
    filename = filedlg.GetPathName();
    //filenameに選択したファイルのフルパスが入る

    char *lp;
    lp = filename.GetBuffer( BUFSIZ );
    if ( swin->screenatr.current_ppd != (Sppd *) NULL ) {
      write_ppd_file( lp, swin->screenatr.current_ppd, FALSE );
      strcpy( swin->screenatr.current_ppd->filename, lp );
    }
  }
}

static char BASED_CODE szmdwFilter[] = "MDW Files (*.mdw)|*.mdw|All Files (*.*)|*.*||";

void CMainFrame::OnFileOpenMdw()
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください

  // TODO: この位置にコマンド ハンドラ用のコードを追加してください

  // 第一引数は OPEN のとき TRUE
  // 第二、三引数はファイルの種類
  CFileDialog filedlg( TRUE, NULL, NULL,
		       OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
		       szmdwFilter, NULL );

  if( filedlg.DoModal() == IDOK) {
    //		CWaitCursor wait;
    CString filename;
    filename = filedlg.GetPathName();
    //filenameに選択したファイルのフルパスが入る

    char *lp;
    lp = filename.GetBuffer( BUFSIZ );
    Tile *tile;
    if ( (tile = open_mdw( lp, TRUE )) == NULL ) {
      AfxMessageBox("Filename was bad!");
      return;
    }
    strcpy( tile->filename, lp );
    
    Sppd *ppd = tile->ppd;
    // set material (default)
    copy_material( &(ppd->matl), 0, (float *) mtlall );

    if ( swin->screenatr.current_tile != NULL ) {
      free_tile( swin->screenatr.current_tile );
      swin->screenatr.current_tile = NULL;
    }
    if ( swin->screenatr.current_ppd != NULL ) {
      free_ppd( swin->screenatr.current_ppd );
      swin->screenatr.view_ppd = NULL;
    }

    swin->screenatr.current_tile = tile;
    swin->screenatr.current_ppd  = ppd;
    swin->screenatr.view_ppd = ppd;
			
    // normalization
    // enhanced
    swin->screenatr.rad_sph *= ppd->scale;
    swin->screenatr.rad_cyl *= ppd->scale;

    // gamma の決定
    swin->screenatr.sp_gamma = GAMMA * ppd->scale;
    // TEMP (あとで消す)
    for ( TEdge* ted = tile->sted; ted != NULL; ted = ted->nxt )
      ted->sp_gamma = swin->screenatr.sp_gamma;
    
    CString strTitle = _T( lp );
    SetWindowText( strTitle );
    ShowWindow( SW_SHOW );
    UpdateWindow();
    
    InvalidateRect( NULL, FALSE );
  }
}

void CMainFrame::OnFileSaveMdw()
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください

  Sppd *ppd = swin->screenatr.current_ppd;
  Tile *tile = swin->screenatr.current_tile;
  if ( tile == NULL ) return;
  if ( ppd == NULL ) return;
  
  if ( strcpy(tile->filename, "") ) {
    
    write_mdw_v10( tile->filename, ppd, tile );
    
  } else {
    
    CFileDialog filedlg( FALSE, NULL, NULL,
			 OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
			 szmdwFilter, NULL );

    if( filedlg.DoModal() == IDOK ) {
      //		CWaitCursor wait;
      CString filename;
      filename = filedlg.GetPathName();
      //filenameに選択したファイルのフルパスが入る

      char *lp;
      lp = filename.GetBuffer( BUFSIZ );
    

      write_mdw_v10( lp, ppd, tile );
      
      strcpy( tile->filename, lp );
    
      CString strTitle = _T( lp );
      SetWindowText( strTitle );
      ShowWindow( SW_SHOW );
      UpdateWindow();
    }
    
  }
}

void CMainFrame::OnFileSaveasMdw() 
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください

  CFileDialog filedlg( FALSE, NULL, NULL,
		       OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
		       szmdwFilter, NULL );

  if( filedlg.DoModal() == IDOK) {
    //		CWaitCursor wait;
    CString filename;
    filename = filedlg.GetPathName();
    //filenameに選択したファイルのフルパスが入る

    char *lp;
    lp = filename.GetBuffer( BUFSIZ );
    if ( swin->screenatr.current_ppd == NULL ) {
      return;
    }
    if ( swin->screenatr.current_tile == NULL ) {
      return;
    }

    Sppd *ppd = swin->screenatr.current_ppd;
    Tile *tile = swin->screenatr.current_tile;
    write_mdw_v10( lp, ppd, tile );

    strcpy( tile->filename, lp );
    
    CString strTitle = _T( lp );
    SetWindowText( strTitle );
    ShowWindow( SW_SHOW );
    UpdateWindow();
  }
}

static char BASED_CODE szsmfFilter[] = "SMF Files (*.smf)|*.smf|All Files (*.*)|*.*||";

//
// open SMF file
//
void CMainFrame::OnFileOpenSmf() 
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください

  // 第一引数は OPEN のとき TRUE
  // 第二、三引数はファイルの種類
  CFileDialog filedlg( TRUE, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
		       szsmfFilter, NULL );

  if( filedlg.DoModal() == IDOK) {
    //		CWaitCursor wait;
    CString filename;
    filename = filedlg.GetPathName();
    //filenameに選択したファイルのフルパスが入る

    char *lp;
    lp = filename.GetBuffer( BUFSIZ );
    Sppd *ppd;
    if ( (ppd = open_smf( lp )) == NULL ) {
      AfxMessageBox("Filename was bad!");
      return;
    }

    // set material (default)
    copy_material( &(ppd->matl), 0, (float *) mtlall );
		
    if ( swin->screenatr.current_ppd != NULL ) {
      free_ppd( swin->screenatr.current_ppd );
      swin->screenatr.view_ppd = NULL;
    }

    swin->screenatr.current_ppd = ppd;
    swin->screenatr.view_ppd = ppd;
			
    // normalization
    // enhanced
    swin->screenatr.rad_sph *= ppd->scale;
    swin->screenatr.rad_cyl *= ppd->scale;
    
    InvalidateRect( NULL, FALSE );
  }
}

//
// save SMF file
//
void CMainFrame::OnFileSaveSmf() 
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください
  CFileDialog filedlg( FALSE, NULL, NULL,
		       OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
		       szsmfFilter, NULL );

  if( filedlg.DoModal() == IDOK ) {
    //	CWaitCursor wait;
    CString filename;
    filename = filedlg.GetPathName();
    //filenameに選択したファイルのフルパスが入る

    char *lp;
    lp = filename.GetBuffer( BUFSIZ );
    if ( swin->screenatr.current_ppd != (Sppd *) NULL ) {
      write_smf( lp, swin->screenatr.current_ppd );
      strcpy( swin->screenatr.current_ppd->filename, lp );
    }
  }
}

static char BASED_CODE szvwFilter[] = "VW Files (*.vw)|*.vw||";

//
// open VW file
//
void CMainFrame::OnFileOpenVw() 
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください
  CFileDialog filedlg( TRUE, NULL, NULL,
		       OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
		       szvwFilter, NULL );

  if( filedlg.DoModal() == IDOK) {
    //		CWaitCursor wait;
    CString filename;
    filename = filedlg.GetPathName();
    //filenameに選択したファイルのフルパスが入る

    char *lp;
    lp = filename.GetBuffer( BUFSIZ );

    double mmat[16], pmat[16];
    open_vw( lp, (double *) mmat, (double *) pmat );

    InvalidateRect( NULL, FALSE );
  }
	
}

//
// save VW file
//
void CMainFrame::OnFileSaveVw()
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください
  CFileDialog filedlg( TRUE, NULL, NULL,
		       OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
		       szvwFilter, NULL );

  if( filedlg.DoModal() == IDOK) {
    CString filename;
    filename = filedlg.GetPathName();
    //filenameに選択したファイルのフルパスが入る

    char *lp;
    lp = filename.GetBuffer( BUFSIZ );

    InvalidateRect( NULL, FALSE );
    write_vw_file( lp );

    //		write_gmh_file( lp, swin->hppd );
  }

}

static char BASED_CODE szspFilter[] = "SP Files (*.sp)|*.sp|All Files (*.*)|*.*||";

//
// open SP file
//
void CMainFrame::OnFileOpenSp()
{
  ScreenAtr *screen = &(swin->screenatr);
  Sppd *ppd = screen->current_ppd;
  if ( ppd == NULL ) {
    AfxMessageBox("ppd file is not open.");
    return;
  }
  
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください
  CFileDialog filedlg( TRUE, NULL, NULL,
		       OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 
		       szspFilter, NULL );

  if( filedlg.DoModal() == IDOK ) {
    
    CString filename;
    filename = filedlg.GetPathName();
    //filenameに選択したファイルのフルパスが入る

    char *lp, fname[BUFSIZ];
    lp = filename.GetBuffer( BUFSIZ );
    find_filename( lp, fname );
    
    // esp
    if ( screen->elp != NULL ) {
      free_splp( screen->elp );
    }
    
    if ( (screen->elp = open_sp( fname, ppd )) == NULL ) {
      AfxMessageBox("Filename was bad!");
      return;
    }

    if ( screen->elp != NULL ) {
      double edis = 0.0;
      for ( Splv *lv = screen->elp->splv; lv != NULL; lv = lv->nxt ) {
	Spvt *vt0, *vt1;
	if ( lv->nxt != NULL ) {
	  vt0 = lv->vt;
	  vt1 = lv->nxt->vt;
	  edis += V3DistanceBetween2Points( &(vt0->vec), &(vt1->vec) );
	}
      }
      char string[BUFSIZ];
      sprintf(string, "edge length: %g\n", edis );
      AfxMessageBox( string );
    }
      	
    InvalidateRect( NULL, FALSE );
  }
}

////////////////////////////////////////////////////////////////////////
// Menu Screen Functions
////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
// Update Menu Screen Functions
////////////////////////////////////////////////////////////////////////
void CMainFrame::OnUpdateMenuScreenShading(CCmdUI* pCmdUI) 
{
  // TODO: この位置に command update UI ハンドラ用のコードを追加してください
  if ( swin->dis3d.shading == FALSE ) {
    //swin->dis3d.shading = FALSE;
    pCmdUI->SetCheck( 0 );
  } else {
    //		swin->dis3d.shading = TRUE;
    pCmdUI->SetCheck( 1 );
  }
}

void CMainFrame::OnUpdateMenuScreenWireframe( CCmdUI* pCmdUI ) 
{
  // TODO: この位置に command update UI ハンドラ用のコードを追加してください
  if ( swin->dis3d.wire == FALSE ) {
    pCmdUI->SetCheck( 0 );
  } else {
    pCmdUI->SetCheck( 1 );
  }
}

void CMainFrame::OnUpdateMenuScreenHidden(CCmdUI* pCmdUI) 
{
  // TODO: この位置に command update UI ハンドラ用のコードを追加してください
  if ( swin->dis3d.hidden == FALSE ) {
    pCmdUI->SetCheck( 0 );
  } else {
    pCmdUI->SetCheck( 1 );
  }
}

void CMainFrame::OnUpdateMenuScreenEdgeid(CCmdUI* pCmdUI) 
{
  // TODO: この位置に command update UI ハンドラ用のコードを追加してください
  if ( swin->dis3d.edgeid == FALSE ) {
    pCmdUI->SetCheck( 0 );
  } else {
    pCmdUI->SetCheck( 1 );
  }
}

void CMainFrame::OnUpdateMenuScreenCoaxis(CCmdUI* pCmdUI) 
{
  // TODO: この位置に command update UI ハンドラ用のコードを追加してください
  if ( swin->dis3d.coaxis == FALSE ) {
    pCmdUI->SetCheck( 0 );
  } else {
    pCmdUI->SetCheck( 1 );
  }
}

void CMainFrame::OnUpdateMenuScreenVert(CCmdUI* pCmdUI) 
{
  // TODO: この位置に command update UI ハンドラ用のコードを追加してください
  // TODO: この位置に command update UI ハンドラ用のコードを追加してください
  if ( swin->dis3d.vertex == FALSE ) {
    pCmdUI->SetCheck( 0 );
  } else {
    pCmdUI->SetCheck( 1 );
  }
}

void CMainFrame::OnUpdateMenuScreenVertid(CCmdUI* pCmdUI) 
{
  // TODO: この位置に command update UI ハンドラ用のコードを追加してください
  if ( swin->dis3d.vertexid == FALSE ) {
    pCmdUI->SetCheck( 0 );
  } else {
    pCmdUI->SetCheck( 1 );
  }
}

void CMainFrame::OnUpdateMenuScreenLoop(CCmdUI* pCmdUI) 
{
  // TODO: この位置に command update UI ハンドラ用のコードを追加してください
  if ( swin->dis3d.loop == FALSE ) {
    pCmdUI->SetCheck( 0 );
  } else {
    pCmdUI->SetCheck( 1 );
  }
}

void CMainFrame::OnUpdateMenuScreenSpath(CCmdUI* pCmdUI) 
{
  // TODO: この位置に command update UI ハンドラ用のコードを追加してください
  if ( swin->dis3d.spath == FALSE ) {
    pCmdUI->SetCheck( 0 );
  } else {
    pCmdUI->SetCheck( 1 );
  }
}

void CMainFrame::OnMenuScreenShading() 
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください
  if ( swin->dis3d.shading == TRUE ) 
    swin->dis3d.shading = FALSE;
  else 
    swin->dis3d.shading = TRUE;

  InvalidateRect( NULL, FALSE );
}

void CMainFrame::OnMenuScreenWireframe() 
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください
  if ( swin->dis3d.wire == TRUE ) 
    swin->dis3d.wire = FALSE;
  else 
    swin->dis3d.wire = TRUE;

  InvalidateRect( NULL, FALSE );
}

void CMainFrame::OnMenuScreenHidden() 
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください
  if ( swin->dis3d.hidden == TRUE ) 
    swin->dis3d.hidden = FALSE;
  else 
    swin->dis3d.hidden = TRUE;

  InvalidateRect( NULL, FALSE );
}

void CMainFrame::OnMenuScreenEdgeid() 
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください
  if ( swin->dis3d.edgeid == TRUE ) 
    swin->dis3d.edgeid = FALSE;
  else 
    swin->dis3d.edgeid = TRUE;

  InvalidateRect( NULL, FALSE );
}

void CMainFrame::OnMenuScreenCoaxis() 
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください
  if ( swin->dis3d.coaxis == TRUE ) 
    swin->dis3d.coaxis = FALSE;
  else 
    swin->dis3d.coaxis = TRUE;

  InvalidateRect( NULL, FALSE );
}

void CMainFrame::OnMenuScreenVert() 
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください
  if ( swin->dis3d.vertex == TRUE ) 
    swin->dis3d.vertex = FALSE;
  else 
    swin->dis3d.vertex = TRUE;

  InvalidateRect( NULL, FALSE );
}

void CMainFrame::OnMenuScreenVertid() 
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください
  if ( swin->dis3d.vertexid == TRUE ) 
    swin->dis3d.vertexid = FALSE;
  else 
    swin->dis3d.vertexid = TRUE;

  InvalidateRect( NULL, FALSE );
}

void CMainFrame::OnMenuScreenLoop() 
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください
  if ( swin->dis3d.loop == TRUE ) 
    swin->dis3d.loop = FALSE;
  else 
    swin->dis3d.loop = TRUE;

  InvalidateRect( NULL, FALSE );
}

void CMainFrame::OnMenuScreenSpath() 
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください
  if ( swin->dis3d.spath == TRUE ) 
    swin->dis3d.spath = FALSE;
  else 
    swin->dis3d.spath = TRUE;

  InvalidateRect( NULL, FALSE );
}

void CMainFrame::OnMenuScreenRemesh() 
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください
  if ( swin->dis3d.remesh == TRUE ) 
    swin->dis3d.remesh = FALSE;
  else 
    swin->dis3d.remesh = TRUE;

  InvalidateRect( NULL, FALSE );
}

void CMainFrame::OnUpdateMenuScreenRemesh(CCmdUI* pCmdUI) 
{
  // TODO: この位置に command update UI ハンドラ用のコードを追加してください
  if ( swin->dis3d.remesh == FALSE ) {
    pCmdUI->SetCheck( 0 );
  } else {
    pCmdUI->SetCheck( 1 );
  }
}

void CMainFrame::OnMenuScreenRemeshboun()
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください
  if ( swin->dis3d.remeshb == TRUE )
    swin->dis3d.remeshb = FALSE;
  else 
    swin->dis3d.remeshb = TRUE;

  InvalidateRect( NULL, FALSE );
}

void CMainFrame::OnUpdateMenuScreenRemeshboun(CCmdUI* pCmdUI)
{
  // TODO: この位置に command update UI ハンドラ用のコードを追加してください
  if ( swin->dis3d.remeshb == FALSE ) {
    pCmdUI->SetCheck( 0 );
  } else {
    pCmdUI->SetCheck( 1 );
  }
}

void CMainFrame::OnMenuScreenGrpmesh()
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください
  if ( swin->dis3d.group_mesh == TRUE )
    swin->dis3d.group_mesh = FALSE;
  else 
    swin->dis3d.group_mesh = TRUE;

  InvalidateRect( NULL, FALSE );
}

void CMainFrame::OnUpdateMenuScreenGrpmesh(CCmdUI* pCmdUI) 
{
  // TODO: この位置に command update UI ハンドラ用のコードを追加してください
  if ( swin->dis3d.group_mesh == FALSE ) {
    pCmdUI->SetCheck( 0 );
  } else {
    pCmdUI->SetCheck( 1 );
  }
}

void CMainFrame::OnMenuScreenSubgraph()
{
  // TODO: この位置に command update UI ハンドラ用のコードを追加してください
  if ( swin->dis3d.subgraph == TRUE ) 
    swin->dis3d.subgraph = FALSE;
  else 
    swin->dis3d.subgraph = TRUE;

  InvalidateRect( NULL, FALSE );
}

void CMainFrame::OnUpdateMenuScreenSubgraph(CCmdUI* pCmdUI)
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください
  if ( swin->dis3d.subgraph == FALSE ) {
    pCmdUI->SetCheck( 0 );
  } else {
    pCmdUI->SetCheck( 1 );
  }
}

////////////////////////////////////////////////////////////////////////
// smooth Display.
////////////////////////////////////////////////////////////////////////
void CMainFrame::OnUpdateMenuScreenEnhanced(CCmdUI* pCmdUI) 
{
  // TODO: この位置に command update UI ハンドラ用のコードを追加してください
  if ( swin->dis3d.smooth == FALSE ) {
    pCmdUI->SetCheck( 0 );
  } else {
    pCmdUI->SetCheck( 1 );
  }
}

void CMainFrame::OnMenuScreenEnhanced() 
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください
  if ( swin->dis3d.enhanced == TRUE ) 
    swin->dis3d.enhanced = FALSE;
  else 
    swin->dis3d.enhanced = TRUE;

  InvalidateRect( NULL, FALSE );
}

////////////////////////////////////////////////////////////////////////
// Smooth Shading
////////////////////////////////////////////////////////////////////////
void CMainFrame::OnMenuScreenSmooth() 
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください
  if ( swin->dis3d.smooth == TRUE ) {
    swin->dis3d.smooth = FALSE;
    
    freeppdnorm( swin->screenatr.current_ppd );
  } else {
    swin->dis3d.smooth = TRUE;
    ppdnorm( swin->screenatr.current_ppd, 40.0 );
  }

  InvalidateRect( NULL, FALSE );
}

void CMainFrame::OnUpdateMenuScreenSmooth(CCmdUI* pCmdUI) 
{
  // TODO: この位置に command update UI ハンドラ用のコードを追加してください
  if ( swin->dis3d.smooth == FALSE ) {
    pCmdUI->SetCheck( 0 );
  } else {
    pCmdUI->SetCheck( 1 );
  }
}


void CMainFrame::OnMenuSpathAnim() 
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください
  if ( swin->dis3d.spath_anim == TRUE ) {
    swin->dis3d.spath_anim = FALSE;
  } else {
    swin->dis3d.spath_anim = TRUE;
  }
}

void CMainFrame::OnUpdateMenuSpathAnim(CCmdUI* pCmdUI) 
{
  // TODO: この位置に command update UI ハンドラ用のコードを追加してください
	
  if ( swin->dis3d.spath_anim == FALSE ) {
    pCmdUI->SetCheck( 0 );
  } else {
    pCmdUI->SetCheck( 1 );
  }
}

void CMainFrame::drawwindow()
{
  RedrawWindow();
}

#include "SRDialog.h"

void CMainFrame::OnMenuScreenRadius() 
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください

  CSRDialog* srd = new CSRDialog;
  ASSERT_VALID( srd );

  BOOL bResult = srd->Create();
  ASSERT( bResult );
}

void CMainFrame::OnViewBackgroundColor()
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください

  ScreenAtr *screen = &(swin->screenatr);
  CColorDialog dlg;
	
  unsigned short r = (unsigned short) (screen->bgrgb[0] * 255);
  unsigned short g = (unsigned short) (screen->bgrgb[1] * 255);
  unsigned short b = (unsigned short) (screen->bgrgb[2] * 255);

  //CString string;
  //string.Format("%d %d %d", r, g, b );
  //AfxMessageBox( string );

  //dlg.SetCurrentColor( RGB( r, g, b ) );
  //dlg.m_cc.rgbResult = RGB( r, g, b );

  if ( dlg.DoModal() == IDOK ) {
    COLORREF color = dlg.GetColor();
    screen->bgrgb[0] = (float) (GetRValue( color ) / 255.0);
    screen->bgrgb[1] = (float) (GetGValue( color ) / 255.0);
    screen->bgrgb[2] = (float) (GetBValue( color ) / 255.0);
    InvalidateRect( NULL, FALSE );
  }
}

#include "MaterialDialog.h"
CMaterialDialog *md = NULL;

void CMainFrame::OnViewMaterial()
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください
  if ( swin->screenatr.current_ppd == NULL ) {
    AfxMessageBox("ppd is not found.");
    return;
  }

  if ( md != NULL ) return;

  md = new CMaterialDialog;
  ASSERT_VALID( md );

  BOOL bResult = md->Create();
  ASSERT( bResult );

}

#include "TileDialog.h"
CTileDialog *td = NULL;

void CMainFrame::OnMenuEditTile()
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください
  if ( swin->screenatr.current_ppd == NULL ) {
    AfxMessageBox("ppd is not found.");
    return;
  }

  if ( td != NULL ) return;

  td = new CTileDialog;
  ASSERT_VALID( td );

  BOOL bResult = td->Create();
  ASSERT( bResult );
  
}

#include "SPDialog.h"

CSPDialog *cspd;

void CMainFrame::OnMenuSpathCreate()
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください
  if ( swin->screenatr.current_ppd == NULL ) {
    AfxMessageBox("ppd is not found.");
    return;
  }

  if ( cspd != NULL ) {
    return;
  }

  cspd = new CSPDialog;
  ASSERT_VALID( cspd );

  BOOL bResult = cspd->Create();
  ASSERT( bResult );
}

#include "sigproc.h"

void CMainFrame::OnMenuFairingSp() 
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください

  ScreenAtr *screen = &(swin->screenatr);
  Sppd *ppd = screen->current_ppd;
  
  if ( ppd == NULL ) {
    AfxMessageBox("ppd is not found.\n");
    return;
  }

  // fairing
  double kpb = 0.1;
  double lambda = 0.7;
  double mu = 1.0 / ( kpb - (1.0 / lambda) );
  int nrepeat = 50;
  ppd_signal_processing( ppd, lambda, mu, nrepeat );

  // Redraw
  InvalidateRect( NULL, FALSE );
}

#include "MeshInfoDialog.h"
CMeshInfoDialog *midlg;

//
// Help Menu Command
//
void CMainFrame::OnMeshInfo()
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください

  if ( swin->screenatr.current_ppd == NULL ) {
    AfxMessageBox("ppd is not found.");
    return;
  }

  midlg = new CMeshInfoDialog;
  ASSERT_VALID( midlg );

  BOOL bResult = midlg->Create();
  ASSERT( bResult );
}

////////////////////////////////////////////////////////////////////////
// Gradient Background
////////////////////////////////////////////////////////////////////////
void CMainFrame::OnViewGradient()
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください
  if ( swin->dis3d.gradient == TRUE ) 
    swin->dis3d.gradient = FALSE;
  else 
    swin->dis3d.gradient = TRUE;

  InvalidateRect( NULL, FALSE );
}

void CMainFrame::OnUpdateViewGradient(CCmdUI* pCmdUI)
{
  // TODO: この位置に command update UI ハンドラ用のコードを追加してください
  if ( swin->dis3d.gradient == FALSE ) {
    pCmdUI->SetCheck( 0 );
  } else {
    pCmdUI->SetCheck( 1 );
  }
}

//
// Light
// 

void CMainFrame::OnMenuScreenLight0() 
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください
  if ( swin->dis3d.light0 == TRUE ) 
    swin->dis3d.light0 = FALSE;
  else 
    swin->dis3d.light0 = TRUE;

  InvalidateRect( NULL, FALSE );
}

void CMainFrame::OnUpdateMenuScreenLight0(CCmdUI* pCmdUI) 
{
  // TODO: この位置に command update UI ハンドラ用のコードを追加してください
  if ( swin->dis3d.light0 == FALSE ) {
    pCmdUI->SetCheck( 0 );
  } else {
    pCmdUI->SetCheck( 1 );
  }
}

void CMainFrame::OnMenuScreenLight1() 
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください
  if ( swin->dis3d.light1 == TRUE ) 
    swin->dis3d.light1 = FALSE;
  else 
    swin->dis3d.light1 = TRUE;

  InvalidateRect( NULL, FALSE );
}

void CMainFrame::OnUpdateMenuScreenLight1(CCmdUI* pCmdUI) 
{
  // TODO: この位置に command update UI ハンドラ用のコードを追加してください
  if ( swin->dis3d.light1 == FALSE ) {
    pCmdUI->SetCheck( 0 );
  } else {
    pCmdUI->SetCheck( 1 );
  }
}

void CMainFrame::OnMenuScreenLight2() 
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください
  if ( swin->dis3d.light2 == TRUE ) 
    swin->dis3d.light2 = FALSE;
  else 
    swin->dis3d.light2 = TRUE;

  InvalidateRect( NULL, FALSE );
	
}

void CMainFrame::OnUpdateMenuScreenLight2(CCmdUI* pCmdUI) 
{
  // TODO: この位置に command update UI ハンドラ用のコードを追加してください
  if ( swin->dis3d.light2 == FALSE ) {
    pCmdUI->SetCheck( 0 );
  } else {
    pCmdUI->SetCheck( 1 );
  }
}

void CMainFrame::OnMenuScreenLight3() 
{
  // TODO: この位置にコマンド ハンドラ用のコードを追加してください
  if ( swin->dis3d.light3 == TRUE ) 
    swin->dis3d.light3 = FALSE;
  else 
    swin->dis3d.light3 = TRUE;

  InvalidateRect( NULL, FALSE );
}

void CMainFrame::OnUpdateMenuScreenLight3(CCmdUI* pCmdUI) 
{
  // TODO: この位置に command update UI ハンドラ用のコードを追加してください
  if ( swin->dis3d.light3 == FALSE ) {
    pCmdUI->SetCheck( 0 );
  } else {
    pCmdUI->SetCheck( 1 );
  }
}


