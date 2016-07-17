#if !defined(AFX_MATERIALDIALOG_H__0F8F5B21_9FFB_11D3_98D2_0000F45A89CD__INCLUDED_)
#define AFX_MATERIALDIALOG_H__0F8F5B21_9FFB_11D3_98D2_0000F45A89CD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MaterialDialog.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CMaterialDialog ダイアログ

class CMaterialDialog : public CDialog
{
// コンストラクション
public:
	CMaterialDialog(CWnd* pParent = NULL);   // 標準のコンストラクタ

// ダイアログ データ
	//{{AFX_DATA(CMaterialDialog)
	enum { IDD = IDD_MATERIAL_DIALOG };
	CListBox	m_material_list;
	BOOL	m_matl_ppd;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CMaterialDialog)
	public:
	virtual BOOL Create();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CMaterialDialog)
	afx_msg void OnMatlPpdCheck();
	afx_msg void OnDblclkMaterialList();
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_MATERIALDIALOG_H__0F8F5B21_9FFB_11D3_98D2_0000F45A89CD__INCLUDED_)
