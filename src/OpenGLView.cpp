//
// OpenGLView.cpp
//
// Copyright (c) 1998-1999 Takashi Kanai
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
//

#include "stdafx.h"
#include "OpenGL.h"

#include "OpenGLDoc.h"
#include "OpenGLView.h"

#include "Material.h"

// Include the OpenGL headers
#include "gl\gl.h"
#include "gl\glu.h"

// from C 
#include "smd.h"
#include "screen.h"

#include "draw.h"
#include "pick.h"
#include "dialog.h"

#include "Bitmap.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const char* const COpenGLView::_ErrorStrings[]= {
  {"No Error"}, // 0
  {"Unable to get a DC"}, // 1
  {"ChoosePixelFormat failed"}, // 2
  {"SelectPixelFormat failed"}, // 3
  {"wglCreateContext failed"}, // 4
  {"wglMakeCurrent failed"}, // 5
  {"wglDeleteContext failed"}, // 6
  {"SwapBuffers failed"},  // 7
};

/////////////////////////////////////////////////////////////////////////////
// COpenGLView

IMPLEMENT_DYNCREATE(COpenGLView, CView)

  BEGIN_MESSAGE_MAP(COpenGLView, CView)
  //{{AFX_MSG_MAP(COpenGLView)
  ON_WM_CREATE()
  ON_WM_DESTROY()
  ON_WM_ERASEBKGND()
  ON_WM_SIZE()
  ON_WM_LBUTTONDOWN()
  ON_WM_LBUTTONUP()
  ON_WM_MOUSEMOVE()
  ON_WM_CHAR()
  //}}AFX_MSG_MAP
  ON_COMMAND(ID_FILE_PRINT, CView::OnFilePrint)
  ON_COMMAND(ID_FILE_PRINT_DIRECT, CView::OnFilePrint)
  ON_COMMAND(ID_FILE_PRINT_PREVIEW, CView::OnFilePrintPreview)
  END_MESSAGE_MAP()

  /////////////////////////////////////////////////////////////////////////////
  // COpenGLView クラスの構築/消滅

  COpenGLView::COpenGLView() :
    m_hRC(0), m_pDC(0), m_ErrorString(_ErrorStrings[0])
{
  // TODO: この場所に構築用のコードを追加してください。

  //	swin->screenatr[SCREEN_SRC].wnd = GetTopLevelParent();
}

COpenGLView::~COpenGLView()
{
}

BOOL COpenGLView::PreCreateWindow(CREATESTRUCT& cs)
{
  // TODO: この位置で CREATESTRUCT cs を修正して Window クラスまたはスタイルを
  //  修正してください。

  cs.style |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

  //	cs.cx = swin->screenatr[SCREEN_SRC].width;
  //	cs.cy = swin->screenatr[SCREEN_SRC].height;

  moving	 = FALSE;
  rotating = FALSE;
  scaling	 = FALSE;

  return CView::PreCreateWindow(cs);

}

/////////////////////////////////////////////////////////////////////////////
// COpenGLView クラスの描画

void COpenGLView::OnDraw(CDC* pDC)
{
  COpenGLDoc* pDoc = GetDocument();
  ASSERT_VALID(pDoc);

  if ( ::wglMakeCurrent( m_pDC->GetSafeHdc(), m_hRC ) == FALSE ) {
    SetError(5);
    return;
  }

  // TODO: この場所にネイティブ データ用の描画コードを追加します。

  redraw();

  if ( ::SwapBuffers( m_pDC->GetSafeHdc() ) == FALSE ) {
    SetError(7);
  }

}

/////////////////////////////////////////////////////////////////////////////
// COpenGLView クラスの印刷

BOOL COpenGLView::OnPreparePrinting(CPrintInfo* pInfo)
{
  // デフォルトの印刷準備
  return DoPreparePrinting(pInfo);
}

void COpenGLView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
  // TODO: 印刷前の特別な初期化処理を追加してください。
}

void COpenGLView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
  // TODO: 印刷後の後処理を追加してください。
}

/////////////////////////////////////////////////////////////////////////////
// COpenGLView クラスの診断

#ifdef _DEBUG
void COpenGLView::AssertValid() const
{
  CView::AssertValid();
}

void COpenGLView::Dump(CDumpContext& dc) const
{
  CView::Dump(dc);
}

COpenGLDoc* COpenGLView::GetDocument() // 非デバッグ バージョンはインラインです。
{
  ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(COpenGLDoc)));
  return (COpenGLDoc*)m_pDocument;
}
#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// COpenGLView クラスのメッセージ ハンドラ

int COpenGLView::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
  if (CView::OnCreate(lpCreateStruct) == -1)
    return -1;
	
  // TODO: この位置に固有の作成用コードを追加してください
	
  InitializeOpenGL();

  return 0;
}

//////////////////////////////////////////////////////////////////////
// GL Helper functions

void COpenGLView::SetError( int e )
{
  // if there was no previous error, 
  // then save this one
  if ( _ErrorStrings[0] == m_ErrorString ) {
    m_ErrorString = _ErrorStrings[e];
  }
}

BOOL COpenGLView::InitializeOpenGL()
{
  m_pDC = new CClientDC( this );

  if ( m_pDC == NULL ) { // failure to get DC
    SetError(1);
    return FALSE;
  }

  if ( !SetupPixelFormat() ) {
    return FALSE;
  }

  if ( (m_hRC = ::wglCreateContext(m_pDC->GetSafeHdc()) )
       == 0 ) {
    SetError(4);
    return FALSE;
  }

  if ( ::wglMakeCurrent( m_pDC->GetSafeHdc(), m_hRC ) == FALSE ) {
    SetError(5);
    return FALSE;
  }

  // fonts initialize
  ScreenAtr *screen;
  screen = &(swin->screenatr);
  screen->fontOffset = ::glGenLists( 256 );

  screen->wnd = GetTopLevelParent();
  
  CFont *fontInfo;
  fontInfo = (CFont*) m_pDC->SelectStockObject( SYSTEM_FONT );
  ::wglUseFontBitmaps( m_pDC->m_hDC, 0, 256, screen->fontOffset );
  m_pDC->SelectObject( fontInfo );

  // window size
//    RECT rect;
//    rect.left = 0;
//    rect.right = 700;
//    rect.top = 0;
//    rect.bottom = 700;
  //MoveWindow( 0, 0, 700, 700, FALSE );
  
  init_gl3d();
	
  return TRUE;
}

BOOL COpenGLView::SetupPixelFormat()
{
  PIXELFORMATDESCRIPTOR pfd = {
    sizeof(PIXELFORMATDESCRIPTOR),	// size of this pfd
    1,					// version number
    PFD_DRAW_TO_WINDOW |		// support window
    PFD_SUPPORT_OPENGL |		// support OpenGL
    PFD_DOUBLEBUFFER,			// double buffered
    PFD_TYPE_RGBA,			// RGBA type
    24,					// 24-bit color depth
    0, 0, 0, 0, 0, 0,			// color bits ignored
    0,					// no alpha buffer
    0,					// shift bit ignored
    0,					// no accumulation buffer
    0, 0, 0, 0,				// accum bits ignored
    16,					// 16-bit z-buffer
    0,					// no stencil buffer
    0,					// no auxiliary buffer
    PFD_MAIN_PLANE,			// main layer
    0,					// reserved
    0, 0, 0				// layer masks ignored
  };
  int pixelformat;

  if ( (pixelformat = ::ChoosePixelFormat(m_pDC->GetSafeHdc(), &pfd))
       == 0 ) {
    SetError(2);
    return FALSE;
  }

  if ( ::SetPixelFormat( m_pDC->GetSafeHdc(), pixelformat, &pfd )
       == FALSE ) {
    SetError(3);
    return FALSE;
  }

  return TRUE;
}

void COpenGLView::OnDestroy() 
{
  CView::OnDestroy();
	
  // TODO: この位置にメッセージ ハンドラ用のコードを追加してください
	
  if ( ::wglMakeCurrent( 0, 0 ) == FALSE ) {
    SetError(2);
  }
  if ( ::wglDeleteContext( m_hRC ) == FALSE ) {
    SetError(6);
  }
  if ( m_pDC ) {
    delete m_pDC;
  }
}

BOOL COpenGLView::OnEraseBkgnd(CDC* pDC) 
{
  // TODO: この位置にメッセージ ハンドラ用のコードを追加するかまたはデフォルトの処理を呼び出してください
	
  // return CView::OnEraseBkgnd(pDC);
  return TRUE; // tell Windows not to erase the Background
}

void COpenGLView::OnSize(UINT nType, int cx, int cy) 
{
  CView::OnSize(nType, cx, cy);
	
  // TODO: この位置にメッセージ ハンドラ用のコードを追加してください

  //	GLdouble aspect_ratio; // width/height ratio
	
  if ( (cx <= 0) || (cy <= 0) )  {
    return;
  }

  ScreenAtr *screen;
  screen = &(swin->screenatr);
  screen->width = cx; screen->height = cy;

  ::glViewport((GLint) 0, (GLint) 0,
	       (GLsizei) screen->width, (GLsizei) screen->height);
  InvalidateRect( NULL, FALSE );
}

//static int scrn_x, scrn_y; 
//static BOOL moving = FALSE, scaling = FALSE;

void COpenGLView::OnLButtonDown(UINT nFlags, CPoint point) 
{
  // TODO: この位置にメッセージ ハンドラ用のコードを追加するかまたはデフォルトの処理を呼び出してください
  CView::OnLButtonDown(nFlags, point);

  scrn_x = point.x;
  scrn_y = point.y;

	// move
  if ( (GetKeyState( VK_SHIFT ) < 0) && 
       (GetKeyState( VK_CONTROL ) < 0) ) { 
    moving = TRUE;
    return;
  }

  // rotate
  if ( GetKeyState( VK_SHIFT ) < 0 ) { 
    rotating = TRUE;
    return;
  }

  if ( GetKeyState( VK_CONTROL ) < 0 ) {
    scaling = TRUE;
    return;
  }

  moving   = FALSE;
  rotating = FALSE;
  scaling  = FALSE;

  ScreenAtr *screen = &(swin->screenatr);

  COpenGLDoc* pDoc = GetDocument();
  CRect rectSave;
  pDoc->m_tracker.GetTrueRect(rectSave);
  if (pDoc->m_tracker.HitTest(point) < 0) {

    // just to demonstrate CRectTracker::TrackRubberBand
    CRectTracker tracker;
    // (8,8) 以上のサイズにならないとラバーバンドにならない
#if 0    
    if (tracker.TrackRubberBand(this, point, pDoc->m_bAllowInvert)) {

      // pick area
      if ( edit_pick_area( 
			  scrn_x, 
			  scrn_y, 
			  scrn_x + tracker.m_rect.Width(),
			  scrn_y + tracker.m_rect.Height(),
			  swin->edit_type, 
			  screen
			  ) == TRUE ) {

				// input vertex ID to src edit box
	SelectToDialog( swin->edit_type, screen );

	InvalidateRect( NULL, FALSE );
      }
      // see if rubber band intersects with the doc's tracker

    } else {
#endif

      // pick only
      if ( edit_pick( point.x, point.y, swin->edit_type, screen )
	   == TRUE ) {

	// input vertex ID to src edit box
	SelectToDialog( swin->edit_type, screen );

	InvalidateRect( NULL, FALSE );
      }
		  
#if 0    
    }
#endif

  } else if ( pDoc->m_tracker.Track(this, point, pDoc->m_bAllowInvert) ) {

    // normal tracking action, when tracker is "hit"
    pDoc->SetModifiedFlag();
    pDoc->UpdateAllViews(NULL, (LPARAM)(LPCRECT)rectSave);
    pDoc->UpdateAllViews(NULL);

  } else {

  }
}

void COpenGLView::OnLButtonUp(UINT nFlags, CPoint point) 
{
  // TODO: この位置にメッセージ ハンドラ用のコードを追加するかまたはデフォルトの処理を呼び出してください
	
  CView::OnLButtonUp(nFlags, point);

  moving   = FALSE;
  rotating = FALSE;
  scaling  = FALSE;
}

void COpenGLView::OnMouseMove(UINT nFlags, CPoint point) 
{
  // TODO: この位置にメッセージ ハンドラ用のコードを追加するかまたはデフォルトの処理を呼び出してください
	
  CView::OnMouseMove(nFlags, point);

  // rotate
  if ( rotating && (GetKeyState(VK_SHIFT) < 0 ) ) { 

    int dx = point.x - scrn_x;
    int dy = point.y - scrn_y;
    update_screen3d_rotate( &(swin->screenatr), dx, dy );
    scrn_x = point.x;
    scrn_y = point.y;
    InvalidateRect( NULL, FALSE );

  } else if ( moving && (GetKeyState(VK_SHIFT) < 0 ) 
	      && (GetKeyState(VK_CONTROL) < 0 ) ) {  // trans

    int dx = point.x - scrn_x;
    int dy = point.y - scrn_y;
    update_screen3d_translate( &(swin->screenatr), dx, dy );
    scrn_x = point.x;
    scrn_y = point.y;
    InvalidateRect( NULL, FALSE );

  } else if ( scaling && (GetKeyState(VK_CONTROL) < 0 ) ) { 

    int dx = point.x - scrn_x;
    int dy = point.y - scrn_y;
    update_screen3d_zoom( &(swin->screenatr), dx, dy );
    scrn_x = point.x;
    scrn_y = point.y;
    InvalidateRect( NULL, FALSE );

  }

}

//
// キーボード入力
//
void COpenGLView::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
  // TODO: この位置にメッセージ ハンドラ用のコードを追加するかまたはデフォルトの処理を呼び出してください
  CView::OnChar(nChar, nRepCnt, nFlags);

  switch (nChar) {
  case 'z': // shading ON/OFF
    { 
      if ( swin->dis3d.shading == TRUE )
	swin->dis3d.shading = FALSE;
      else
	swin->dis3d.shading = TRUE;
    }
    break;
  case 'x': // wireframe ON/OFF
    { 
      if ( swin->dis3d.wire == TRUE )
	swin->dis3d.wire = FALSE;
      else
	swin->dis3d.wire = TRUE;
    }
    break;
  case 'c': // vertex ON/OFF
    { 
      if ( swin->dis3d.vertex == TRUE )
	swin->dis3d.vertex = FALSE;
      else
	swin->dis3d.vertex = TRUE;
    }
    break;
  default:
    break;
  }
  
  InvalidateRect( NULL, FALSE );
}

