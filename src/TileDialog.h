//
// TileDialog.h
//
// Copyright (c) 1998-1999 Takashi Kanai
//
// This software is released under the MIT License.
// http://opensource.org/licenses/mit-license.php
//

#if !defined(AFX_TILEDIALOG_H__86FEEB01_9E4D_11D3_8824_0800690F3984__INCLUDED_)
#define AFX_TILEDIALOG_H__86FEEB01_9E4D_11D3_8824_0800690F3984__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CTileDialog ダイアログ

class CTileDialog : public CDialog
{
// コンストラクション
public:
  CTileDialog(CWnd* pParent = NULL);   // 標準のコンストラクタ

// ダイアログ データ
	//{{AFX_DATA(CTileDialog)
	enum { IDD = IDD_TILE };
	CEdit	m_tile_sgv;
	CEdit	m_tile_sge;
	CEdit	m_tile_gamma;
	CEdit	m_tile_avrsp;
	CEdit	m_remesh_ndiv_txt;
	CComboBox m_remesh_pattern_combo;
	CEdit	m_selected_center_vertex_txt;
	CEdit	m_selected_tile_txt;
	CListBox m_tileface_list;
	CButton m_rbutton_paramconv;
	//}}AFX_DATA

	// ユーザ定義変数
	int stored_edit_type;
	
// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CTileDialog)
	public:
	virtual BOOL Create();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// ユーザ定義関数
 public:
	// remesh text
	void set_selected_tile_txt( int id );
	void clear_selected_tile_txt( void );
	void set_selected_center_vertex_txt( int id );
	void clear_selected_center_vertex_txt( void );
	void set_center_vertex( Spvt * );
	void reset_edit_type( void );

	// remesh ndiv text
	void set_remesh_ndiv( int );
	void set_remesh_combo_cursel( int );
	
	// list string operations
	void ListInsertString( int, char * );
	void ListClearStrings( void );

	
	// make tile edge/face
	void MakeTileEdge( void );
	void DeleteTileEdge( void );
	void MakeTileFace( void );
	void MakeTileCylFace( void );

	// sp text functions
	void set_sptext( int, int, double, double );
	void reset_sptext( void );
	
// インプリメンテーション
protected:

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CTileDialog)
	afx_msg void OnTileedgeButtonOk();
	afx_msg void OnTileedgeButtonCancel();
	afx_msg void OnTileedgeButtonClear();
	virtual BOOL OnInitDialog();
	afx_msg void OnMakeedgeRb();
	afx_msg void OnMakefaceRb();
	virtual void OnCancel();
	afx_msg void OnRemeshingRb();
	afx_msg void OnRemeshButtonOk();
	afx_msg void OnRemeshButtonCancel();
	afx_msg void OnRemeshButtonClear();
	afx_msg void OnDeledgeRb();
	afx_msg void OnSelchangeRemeshPatternCombo();
	afx_msg void OnTileSpGammaSet();
	afx_msg void OnSelectCenterVertexButton();
	afx_msg void OnParamorgRb();
	afx_msg void OnParamtpRb();
	afx_msg void OnParamewRb();
	afx_msg void OnParamfwRb();
	afx_msg void OnRecalcParamCb();
	afx_msg void OnParamconvRb();
	afx_msg void OnDelfaceRb();
	afx_msg void OnMakecfaceRb();
	afx_msg void OnRadioPcGeodis();
	afx_msg void OnRadioPcChord();
	afx_msg void OnRadioPcNo();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_TILEDIALOG_H__86FEEB01_9E4D_11D3_8824_0800690F3984__INCLUDED_)
