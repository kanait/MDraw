// TileDialog.cpp : インプリメンテーション ファイル
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
#include "ppd.h"
#include "ppdlist.h"
#include "ppdface.h"
#include "tile.h"
#include "mtl.h"

#include "edit.h"
#include "sellist.h"

#include "sgraph.h"
#include "linbcg.h"
#include "spm.h"
#include "remesh.h"
#include "tfcremesh.h"
#include "floater.h"
#include "uvwsigproc.h"

#include "mdw.h"

#include "TileDialog.h"

extern CTileDialog *td;

/////////////////////////////////////////////////////////////////////////////
// CTileDialog ダイアログ


CTileDialog::CTileDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CTileDialog::IDD, pParent)
{
  //{{AFX_DATA_INIT(CTileDialog)
  //}}AFX_DATA_INIT
}

BOOL CTileDialog::Create()
{
  // TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください

  return CDialog::Create( CTileDialog::IDD );
}

BOOL CTileDialog::OnInitDialog()
{
  CDialog::OnInitDialog();
	
  // TODO: この位置に初期化の補足処理を追加してください
  swin->edit_type = EDIT_NONE;
  
  // pattern type combo initialize
  CString string;
  if ( string.LoadString( IDS_PATTERN_LATTICE ) ) {
    m_remesh_pattern_combo.AddString( (LPCTSTR) string );
  }
  if ( string.LoadString( IDS_PATTERN_RADIAL ) ) {
    m_remesh_pattern_combo.AddString( (LPCTSTR) string );
  }

  m_remesh_pattern_combo.SetCurSel(0);

  stored_edit_type = EDIT_NONE;

  swin->param_type = SP_CONVEX;
  m_rbutton_paramconv.SetCheck(1);
  
  // number of subdivision

  return TRUE;
  // コントロールにフォーカスを設定しないとき、戻り値は TRUE となります
  // 例外: OCX プロパティ ページの戻り値は FALSE となります
}

void CTileDialog::PostNcDestroy()
{
  // TODO: この位置に固有の処理を追加するか、または基本クラスを呼び出してください
	
  //CDialog::PostNcDestroy();

  td = NULL;
  delete this;
}

// Dialog のクローズ
void CTileDialog::OnCancel()
{
  // TODO: この位置に特別な後処理を追加してください。
  //CDialog::OnCancel();
  ListClearStrings();
  ScreenAtr *screen = &(swin->screenatr);
  FreeSelectList( screen );

  Sppd *ppd = screen->current_ppd;
  if ( ppd->sg != NULL ) {
    free_sgraph( ppd->sg );
    ppd->sg = NULL;
  }
  swin->edit_type = EDIT_NONE;

  DestroyWindow();
}

void CTileDialog::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  //{{AFX_DATA_MAP(CTileDialog)
  DDX_Control(pDX, IDC_TILE_TXT_SGV, m_tile_sgv);
  DDX_Control(pDX, IDC_TILE_TXT_SGE, m_tile_sge);
  DDX_Control(pDX, IDC_TILE_TXT_GAMMA, m_tile_gamma);
  DDX_Control(pDX, IDC_TILE_TXT_AVRSP, m_tile_avrsp);
  DDX_Control(pDX, IDC_EDIT_REMESH_NDIV, m_remesh_ndiv_txt);
  DDX_Control(pDX, IDC_REMESH_PATTERN_COMBO, m_remesh_pattern_combo);
  DDX_Control(pDX, IDC_SELECT_CENTER_VERTEX_TXT, m_selected_center_vertex_txt);
  DDX_Control(pDX, IDC_SELECT_TILE_TXT, m_selected_tile_txt);
  DDX_Control(pDX, IDC_TILEFACE_LIST, m_tileface_list);
  DDX_Control(pDX, IDC_PARAMCONV_RB, m_rbutton_paramconv);
  //}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTileDialog, CDialog)
  //{{AFX_MSG_MAP(CTileDialog)
  ON_BN_CLICKED(IDC_TILEEDGE_BUTTON_OK, OnTileedgeButtonOk)
  ON_BN_CLICKED(IDC_TILEEDGE_BUTTON_CANCEL, OnTileedgeButtonCancel)
  ON_BN_CLICKED(IDC_TILEEDGE_BUTTON_CLEAR, OnTileedgeButtonClear)
  ON_BN_CLICKED(IDC_MAKEEDGE_RB, OnMakeedgeRb)
  ON_BN_CLICKED(IDC_MAKEFACE_RB, OnMakefaceRb)
  ON_BN_CLICKED(IDC_REMESHING_RB, OnRemeshingRb)
  ON_BN_CLICKED(IDC_REMESH_BUTTON_OK, OnRemeshButtonOk)
  ON_BN_CLICKED(IDC_REMESH_BUTTON_CANCEL, OnRemeshButtonCancel)
  ON_BN_CLICKED(IDC_REMESH_BUTTON_CLEAR, OnRemeshButtonClear)
  ON_BN_CLICKED(IDC_DELEDGE_RB, OnDeledgeRb)
  ON_CBN_SELCHANGE(IDC_REMESH_PATTERN_COMBO, OnSelchangeRemeshPatternCombo)
  ON_BN_CLICKED(IDC_TILE_SP_GAMMA_SET, OnTileSpGammaSet)
  ON_BN_CLICKED(IDC_SELECT_CENTER_VERTEX_BUTTON, OnSelectCenterVertexButton)
  ON_BN_CLICKED(IDC_PARAMORG_RB, OnParamorgRb)
  ON_BN_CLICKED(IDC_PARAMTP_RB, OnParamtpRb)
  ON_BN_CLICKED(IDC_PARAMEW_RB, OnParamewRb)
  ON_BN_CLICKED(IDC_PARAMFW_RB, OnParamfwRb)
  ON_BN_CLICKED(IDC_RECALC_PARAM_CB, OnRecalcParamCb)
  ON_BN_CLICKED(IDC_PARAMCONV_RB, OnParamconvRb)
  ON_BN_CLICKED(IDC_DELFACE_RB, OnDelfaceRb)
  ON_BN_CLICKED(IDC_MAKECFACE_RB, OnMakecfaceRb)
  ON_BN_CLICKED(IDC_RADIO_PC_GEODIS, OnRadioPcGeodis)
  ON_BN_CLICKED(IDC_RADIO_PC_CHORD, OnRadioPcChord)
  ON_BN_CLICKED(IDC_RADIO_PC_NO, OnRadioPcNo)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTileDialog メッセージ ハンドラ

void CTileDialog::OnTileedgeButtonOk() 
{
  // TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
  switch ( swin->edit_type ) {

  case EDIT_MAKE_LINES:
    MakeTileEdge();
    break;
    
  case EDIT_DEL_LINES:
    DeleteTileEdge();
    break;
    
  case EDIT_MAKE_TILE_FACES:
    MakeTileFace();
    break;
    
  case EDIT_MAKE_TILE_CYL_FACES:
    MakeTileCylFace();
    break;
    
  default:
    break;
  }

  ScreenAtr *screen = &(swin->screenatr);
  
  //#if defined(_DEBUG)
  write_mdw_v10( "__save_temp.mdw", screen->current_ppd, screen->current_tile );
  //#endif  

  ListClearStrings();
  FreeSelectList( screen );
  GetTopLevelParent()->RedrawWindow();
}

void CTileDialog::MakeTileEdge()
{
  ScreenAtr *screen = &(swin->screenatr);

  if ( screen->n_sellist < 2 ) {
    AfxMessageBox("Can't create a line.");
    GetTopLevelParent()->Invalidate();
    return;
  }

  if ( asp_create_spath( screen ) == TRUE ) {
    
    Tile *tile = swin->screenatr.current_tile;
    if ( tile == NULL ) {
      tile = create_tile();
      swin->screenatr.current_tile = tile;
      tile->ppd = screen->current_ppd;
    }

    // create tile edge
    TEdge *ted = create_tedge( tile );
    ted->lp = screen->alp; screen->alp = NULL;
    
    // sp_gamma をエッジごとに保持
    ted->sp_gamma = swin->screenatr.sp_gamma;

    // create tile vertex
    TVertex *tvt;
    if ( (tvt = find_tvertex( tile, screen->sel_first->vt ))
	 == NULL ) {
      tvt = create_tvertex( tile );
      tvt->vt = ted->lp->splv->vt;
      //tvt->vt = screen->sel_first->vt;
    }
    ted->sv = tvt;
    
    if ( (tvt = find_tvertex( tile, screen->sel_first->nxt->vt ))
	 == NULL ) {
      tvt = create_tvertex( tile );
      tvt->vt = ted->lp->eplv->vt;
      //tvt->vt = screen->sel_first->nxt->vt;
    }
    ted->ev = tvt;

    // 頂点に sp_type をつける 
    for ( Splv *lv = ted->lp->splv; lv != NULL; lv = lv->nxt ) {

      // spn を１つ追加
      ++(lv->vt->spn);
      
      if ( lv->type == LOOPVT_CORNER ) {
	lv->vt->sp_type = SP_VERTEX_TEDGE_BP;
      } else {
	lv->vt->sp_type = SP_VERTEX_TEDGE;
      }
      
    }
    ted->sv->vt->sp_type = SP_VERTEX_TVERTEX;
    ted->ev->vt->sp_type = SP_VERTEX_TVERTEX;

  } else {
    AfxMessageBox("Can't create a line.");
  }
}

void CTileDialog::DeleteTileEdge()
{
  ScreenAtr *screen = &(swin->screenatr);
  Tile *tile = screen->current_tile;
  for ( SelList *sl = screen->sel_first; sl != NULL; sl = sl->nxt ) {

    TEdge *ted = sl->ted;
    if ( isusedtedge( ted, tile ) == TRUE ) continue;

    // delete
    sl->ted = NULL;
    delete_tedge( ted, tile );
  }
}

void CTileDialog::MakeTileFace()
{
  ScreenAtr *screen = &(swin->screenatr);
  if ( screen->n_sellist < TRIANGLE ) {
    AfxMessageBox("Can't create a tile face.");
    return;
  }

  // make tile face
  if ( create_tile_face( screen ) == FALSE ) {
    AfxMessageBox("Fail to create a tile face.");
  }
}

void CTileDialog::MakeTileCylFace()
{
  // make tile cylindrical face
  ScreenAtr *screen = &(swin->screenatr);
  if ( create_tile_cylindrical_face( screen ) == FALSE ) {
    AfxMessageBox("Fail to create a tile face.");
  }
}

void CTileDialog::OnTileedgeButtonCancel() 
{
  // TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
	
  ListClearStrings();
  FreeSelectList( &(swin->screenatr) );
  GetTopLevelParent()->RedrawWindow();
}

void CTileDialog::OnTileedgeButtonClear()
{
  // TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
	
  ListClearStrings();
  FreeSelectList( &(swin->screenatr) );
  GetTopLevelParent()->RedrawWindow();
}

//
// Remeshing functions
//

void CTileDialog::OnRemeshButtonOk()
{
  // TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
  UpdateData(TRUE);

  ScreenAtr *screen = &(swin->screenatr);
  if ( !(screen->n_sellist) ) {
    AfxMessageBox("Tile is not selected.\n");
    FreeSelectList( screen );
    clear_selected_tile_txt();
    clear_selected_center_vertex_txt();
    return;
  }

  // 分割数
  CString string;
  m_remesh_ndiv_txt.GetWindowText( string );
  LPTSTR lpszbuffer= string.GetBuffer( BUFSIZ );
  int ndiv = atoi( lpszbuffer );

  // リメッシュパターンの選択
  int n = m_remesh_pattern_combo.GetCurSel();
  
  TFace *tfc = NULL;
  for ( SelList *sl = screen->sel_first; sl != NULL; sl = sl->nxt )
    {
      tfc = sl->tf;
      // 以前のメッシュを削除
      if ( tfc->rppd != NULL ) {
	free_ppd( tfc->rppd );
	tfc->rppd = NULL;
      }
      
      //
      // 番号付け
      //
      rename_tvector( tfc );

      //
      // パラメータ化 (チェックがオンのときのみ)
      //
      if ( swin->recalc_param == TRUE )
	{
#if defined(_DEBUG)
	  write_tfc_param_file("__save_before_prm.ppd", tfc, screen->current_ppd );
#endif  
      
	  // shape preserving mapping
	  tfcspm( tfc );

#if defined(_DEBUG)
	  write_tfc_param_file("__save_mid_prm.ppd", tfc, screen->current_ppd );
#endif  
      
	  // relaxation of internal vertices
	  if ( swin->param_type != SP_CONVEX )
	    {
	      tfcparam( tfc, swin->param_type );
	    }
      
#if defined(_DEBUG)
	  write_tfc_param_file("__save_after_prm.ppd", tfc, screen->current_ppd );
	  write_ppd_tfc("__save_tile.ppd", tfc, screen->current_ppd );
#endif  
  
	}

      //
      // 分割数の設定
      //
      tfc->ndiv = ndiv;

      //
      // リメッシュ
      //
      if ( n == 0 )
	{
	  tfc->tptype = TILE_PATTERN_LATTICE;
	  tfc->rppd = tolatticemesh( tfc, tfc->ndiv, swin->paramcorr_type );
	}
      else if ( n == 1 )
	{
	  
	  if ( tfc->cv == NULL ) {
	    AfxMessageBox("Center vertex is not specified.");
	    FreeSelectList( screen );
	    clear_selected_tile_txt();
	    clear_selected_center_vertex_txt();
	    //GetTopLevelParent()->RedrawWindow();
	    continue;
	  }
	  tfc->tptype = TILE_PATTERN_RADIAL;
	  tfc->rppd = toradialmesh( tfc, tfc->ndiv );
	}

      // remeshing
      ppdfacenormal( tfc->rppd );
      copy_material( &(tfc->rppd->matl), 6, (float *) mtlall );

#if defined(_DEBUG)
      {
	write_ppd_file( "__save_remesh.ppd", tfc->rppd, FALSE );
	write_ppd_uvw( "__save_remesh_prm.ppd", tfc->rppd );
      }
#endif      

      // 一枚ごとに再描画
      GetTopLevelParent()->RedrawWindow();
    }

  //FreeSelectList( screen );
  //clear_selected_tile_txt();
  //clear_selected_center_vertex_txt();
  
}

void CTileDialog::OnRemeshButtonCancel()
{
  // TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
  FreeSelectList( &(swin->screenatr) );
  clear_selected_tile_txt();
  clear_selected_center_vertex_txt();
  GetTopLevelParent()->RedrawWindow();
}

void CTileDialog::OnRemeshButtonClear() 
{
  // TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
  FreeSelectList( &(swin->screenatr) );
  clear_selected_tile_txt();
  clear_selected_center_vertex_txt();
  GetTopLevelParent()->RedrawWindow();
}

void CTileDialog::OnSelectCenterVertexButton() 
{
  // TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
  if ( swin->screenatr.sel_first == NULL ) {
    AfxMessageBox("tile is not selected.");
    return;
  }
  if ( swin->screenatr.sel_first->tf == NULL ) {
    AfxMessageBox("tile is not selected.");
    return;
  }

  TFace *tfc = swin->screenatr.sel_first->tf;
  if ( tfc->cv != NULL ) {
    tfc->cv->col = PNTGREEN;
  }
  stored_edit_type = swin->edit_type;
  swin->edit_type = EDIT_SEL_CVERTEX;
}


// ラジオボタン

void CTileDialog::OnMakeedgeRb()
{
  // TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
  ListClearStrings();
  ScreenAtr *screen = &(swin->screenatr);
  FreeSelectList( screen );
  
  // GAMMA is in sgraph.h
  Sppd *ppd = screen->current_ppd;
  //double gamma = SP_GAMMA * ppd->scale;
  // create sub-graph
  if ( ppd != NULL ) {
    if ( ppd->sg == NULL ) {
      ppd->sg = initialize_sgraph( ppd, screen->sp_gamma );
    }
  }

  set_sptext( ppd->sg->sgvtn, ppd->sg->sgedn,
	      screen->sp_gamma, ppd->sg->avrsp );
  
  swin->edit_type = EDIT_MAKE_LINES;
}

void CTileDialog::OnDeledgeRb()
{
  // TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
  ListClearStrings();
  ScreenAtr *screen = &(swin->screenatr);
  FreeSelectList( screen );
  
  Sppd *ppd = screen->current_ppd;
  if ( ppd->sg != NULL ) {
    free_sgraph( ppd->sg );
    ppd->sg = NULL;
  }
  reset_sptext();
  
  swin->edit_type = EDIT_DEL_LINES;
}

void CTileDialog::OnDelfaceRb()
{
  // TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
  ListClearStrings();
  ScreenAtr *screen = &(swin->screenatr);
  FreeSelectList( screen );
  
  Sppd *ppd = screen->current_ppd;
  if ( ppd->sg != NULL ) {
    free_sgraph( ppd->sg );
    ppd->sg = NULL;
  }
  reset_sptext();
  
  swin->edit_type = EDIT_DEL_TILES;
}

void CTileDialog::OnMakefaceRb() 
{
  // TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
  ListClearStrings();
  ScreenAtr *screen = &(swin->screenatr);
  FreeSelectList( screen );

  Sppd *ppd = screen->current_ppd;
  if ( ppd->sg != NULL ) {
    free_sgraph( ppd->sg );
    ppd->sg = NULL;
  }
  reset_sptext();
  
  swin->edit_type = EDIT_MAKE_TILE_FACES;
}

void CTileDialog::OnMakecfaceRb()
{
  // TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
  ListClearStrings();
  ScreenAtr *screen = &(swin->screenatr);
  FreeSelectList( screen );

  Sppd *ppd = screen->current_ppd;
  if ( ppd->sg != NULL ) {
    free_sgraph( ppd->sg );
    ppd->sg = NULL;
  }
  reset_sptext();
  
  swin->edit_type = EDIT_MAKE_TILE_CYL_FACES;
}

void CTileDialog::OnRemeshingRb() 
{
  // TODO: この位置にコントロール通知ハンドラ用のコードを追加してください

  ListClearStrings();
  ScreenAtr *screen = &(swin->screenatr);
  FreeSelectList( screen );

  Sppd *ppd = screen->current_ppd;
  if ( ppd->sg != NULL ) {
    free_sgraph( ppd->sg );
    ppd->sg = NULL;
  }
  reset_sptext();
  
  swin->edit_type = EDIT_REMESH;
}

// pattern combo functions
void CTileDialog::OnSelchangeRemeshPatternCombo() 
{
  // TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
  //display("%d -\n", m_remesh_pattern_combo.GetCurSel());
}

// SP gamma
void CTileDialog::OnTileSpGammaSet()
{
  // TODO: この位置にコントロール通知ハンドラ用のコードを追加してください

  CString string;
  m_tile_gamma.GetWindowText( string );
  LPTSTR lpszbuffer= string.GetBuffer( BUFSIZ );
  double ngamma = atof( lpszbuffer );

  Sppd *ppd = swin->screenatr.current_ppd;
  if ( ppd->sg != NULL ) {
    free_sgraph( ppd->sg );
    ppd->sg = NULL;
  }
  ppd->sg = initialize_sgraph( ppd, ngamma );
  swin->screenatr.sp_gamma = ngamma;
  
  set_sptext( ppd->sg->sgvtn, ppd->sg->sgedn,
	      ngamma, ppd->sg->avrsp );
  
}

//
// user-defined functions
//

void CTileDialog::set_selected_tile_txt( int id )
{
  CString string;
  string.Format("%d", id );
  m_selected_tile_txt.SetWindowText( string );
}

void CTileDialog::clear_selected_tile_txt( void )
{
  CString string;
  string.Format("");
  m_selected_tile_txt.SetWindowText( string );
}

void CTileDialog::set_selected_center_vertex_txt( int id )
{
  CString string;
  string.Format("%d", id );
  m_selected_center_vertex_txt.SetWindowText( string );
}

void CTileDialog::clear_selected_center_vertex_txt( void )
{
  CString string;
  string.Format("");
  m_selected_center_vertex_txt.SetWindowText( string );
}

void CTileDialog::set_center_vertex( Spvt *vt )
{
  if ( swin->screenatr.sel_first != NULL ) {
    TFace *tf = swin->screenatr.sel_first->tf;
    tf->cv = vt;
  }
}

void CTileDialog::reset_edit_type( void )
{
  swin->edit_type = stored_edit_type;
  stored_edit_type = EDIT_NONE;
}

// list functions
void CTileDialog::ListInsertString( int num, char *string )
{
  m_tileface_list.InsertString( num, string );
}

void CTileDialog::ListClearStrings( void )
{
  int num = m_tileface_list.GetCount();
  for ( int i = 0; i < num; ++i ) {
    m_tileface_list.DeleteString( 0 );
  }
}

void CTileDialog::set_remesh_ndiv( int ndiv )
{
  //m_remesh_ndiv = ndiv;
  CString string;
  string.Format("%d", ndiv );
  m_remesh_ndiv_txt.SetWindowText( string );
}

void CTileDialog::set_remesh_combo_cursel( int tptype )
{
  if ( tptype == TILE_PATTERN_LATTICE ) {
    m_remesh_pattern_combo.SetCurSel(0);
  } else if ( tptype == TILE_PATTERN_RADIAL ) {
    m_remesh_pattern_combo.SetCurSel(1);
  }
}

//
// sp text functions
//
void CTileDialog::set_sptext( int vn, int en, double gamma, double avrsp )
{
  CString string;
  // set the number of vertices in the sub-graph
  string.Format( "%d", vn );
  m_tile_sgv.SetWindowText( string );
  // set the number of edges in the sub-graph
  string.Format( "%d", en );
  m_tile_sge.SetWindowText( string );
  // set gamma value
  string.Format( "%g", gamma );
  m_tile_gamma.SetWindowText( string );
  // set average number of SP per edge
  string.Format( "%g", avrsp );
  m_tile_avrsp.SetWindowText( string );
}

void CTileDialog::reset_sptext( void )
{
  CString string;
  string.Format("");
  m_tile_sgv.SetWindowText( string );
  m_tile_sge.SetWindowText( string );
  m_tile_gamma.SetWindowText( string );
  m_tile_avrsp.SetWindowText( string );
}

// convex combination
void CTileDialog::OnParamconvRb()
{
  // TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
  swin->param_type = SP_CONVEX;
}

// membrane
void CTileDialog::OnParamorgRb()
{
  // TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
  swin->param_type = SP_MEMBRANE;
}

void CTileDialog::OnParamtpRb() 
{
  // TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
  swin->param_type = SP_TPWEIGHT;
}

void CTileDialog::OnParamewRb() 
{
  // TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
  swin->param_type = SP_EWEIGHT;
}

void CTileDialog::OnParamfwRb() 
{
  // TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
  swin->param_type = SP_FWEIGHT;
}

void CTileDialog::OnRecalcParamCb()
{
  // TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
  if ( swin->recalc_param == TRUE ) 
    swin->recalc_param = FALSE;
  else 
    swin->recalc_param = TRUE;
}

//
// parameter correction radio buttons
//
void CTileDialog::OnRadioPcGeodis() 
{
  // TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
  swin->paramcorr_type = DIFF_GEODIS;
}

void CTileDialog::OnRadioPcChord() 
{
  // TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
  swin->paramcorr_type = DIFF_CHORD;
}

void CTileDialog::OnRadioPcNo() 
{
  // TODO: この位置にコントロール通知ハンドラ用のコードを追加してください
  swin->paramcorr_type = DIFF_NO;
}
