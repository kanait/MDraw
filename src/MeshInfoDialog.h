#if !defined(AFX_MESHINFODIALOG_H__DD826CC0_BDCA_11D3_98FC_0000F45A89CD__INCLUDED_)
#define AFX_MESHINFODIALOG_H__DD826CC0_BDCA_11D3_98FC_0000F45A89CD__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MeshInfoDialog.h : ヘッダー ファイル
//

/////////////////////////////////////////////////////////////////////////////
// CMeshInfoDialog ダイアログ

class CMeshInfoDialog : public CDialog
{
// コンストラクション
public:
	CMeshInfoDialog(CWnd* pParent = NULL);   // 標準のコンストラクタ

// ダイアログ データ
	//{{AFX_DATA(CMeshInfoDialog)
	enum { IDD = IDD_MESHINFO };
	CEdit	m_mesh_info;
	//}}AFX_DATA


// オーバーライド
	// ClassWizard は仮想関数のオーバーライドを生成します。
	//{{AFX_VIRTUAL(CMeshInfoDialog)
	public:
	virtual BOOL Create( void );
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV サポート
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// インプリメンテーション
protected:

	// 生成されたメッセージ マップ関数
	//{{AFX_MSG(CMeshInfoDialog)
	virtual void OnOK();
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ は前行の直前に追加の宣言を挿入します。

#endif // !defined(AFX_MESHINFODIALOG_H__DD826CC0_BDCA_11D3_98FC_0000F45A89CD__INCLUDED_)
