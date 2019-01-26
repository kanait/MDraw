//
// dialog.cpp
// Copyright (c) 1998-1999 Takashi Kanai
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
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

// Dialog input 
//#include "PLDialog.h"
//#include "DPLDialog.h"
//#include "PFDialog.h"
//#include "DPFDialog.h"
//#include "CVDialog.h"
//#include "CMDialog.h"
//#include "DCMDialog.h"
#include "SPDialog.h"
//#include "DSPDialog.h"
//#include "MSPDialog.h"

//extern CPLDialog* cpld;
//extern CDPLDialog* dpld;
//extern CPFDialog* cpfd;
//extern CDPFDialog* dpfd;
//extern CCVDialog* ccvd;
//extern CCMDialog* ccmd;
//extern CDCMDialog* dcmd;
extern CSPDialog* cspd;
//extern CDSPDialog* dspd;
//extern CMSPDialog* mspd;

void SelectToDialog( int edit_type, ScreenAtr *screen )
{
  CWnd* wnd;
  CString string;

  switch ( edit_type ) {

  case EDIT_CREATE_SPATH:

    if ( cspd == NULL ) return;

    if ( screen->n_sellist == 1 ) {
      wnd = cspd->GetDlgItem( IDC_CSP_SRC );
      ASSERT( wnd );
      if ( wnd == NULL ) return;
      string.Format("%d", screen->sel_first->vt->no );
      wnd->SetWindowText( string );
    } else if ( screen->n_sellist == 2 ) {
      wnd = cspd->GetDlgItem( IDC_CSP_DIST );
      ASSERT( wnd );
      if ( wnd == NULL ) return;
      string.Format("%d", screen->sel_first->nxt->vt->no );
      wnd->SetWindowText( string );
    }
    return;

  }
}
