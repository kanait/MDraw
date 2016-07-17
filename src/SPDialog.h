#if !defined(AFX_SPDIALOG_H__61649A10_47BC_11D2_AB66_00E029217E15__INCLUDED_)
#define AFX_SPDIALOG_H__61649A10_47BC_11D2_AB66_00E029217E15__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// SPDialog.h : �w�b�_�[ �t�@�C��
//

/////////////////////////////////////////////////////////////////////////////
// CSPDialog �_�C�A���O

class CSPDialog : public CDialog
{
// �R���X�g���N�V����
public:
  CSPDialog(CWnd* pParent = NULL);   // �W���̃R���X�g���N�^
  BOOL Create( void );
  void ResetPathType( void );
  // 	void esp_progress_setrange( int );
  // 	void esp_progress_stepit( void );

// �_�C�A���O �f�[�^
  //{{AFX_DATA(CSPDialog)
  enum { IDD = IDD_CREATE_SPATH };
  CEdit	m_sp_avrsp;
  CEdit	m_obj_face;
  CEdit	m_obj_edge;
  CEdit	m_obj_vertex;
  CEdit	m_esp_time;
  CEdit	m_asp_time;
  CEdit	m_esp_length;
  CButton	m_exact;
  CButton	m_both;
  CButton	m_approx;
  CEdit	m_sg_vertex;
  CEdit	m_sg_edge;
  CEdit	m_sp_gamma;
  CEdit	m_sp_src;
  CEdit	m_sp_length;
  CEdit	m_sp_dist;
  CString	m_sp_dist_data;
  CString	m_sp_length_data;
  CString	m_sp_src_data;
  CString	m_sp_gamma_data;
  CString	m_sg_edge_data;
  CString	m_sg_vertex_data;
  CString	m_esp_length_data;
  CString	m_asp_time_data;
  CString	m_esp_time_data;
  CString	m_obj_vertex_data;
  CString	m_obj_edge_data;
  CString	m_obj_face_data;
  CString	m_sp_avrsp_data;
  //}}AFX_DATA


// �I�[�o�[���C�h
	// ClassWizard �͉��z�֐��̃I�[�o�[���C�h�𐶐����܂��B

	//{{AFX_VIRTUAL(CSPDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �T�|�[�g
	virtual void PostNcDestroy();
	//}}AFX_VIRTUAL

// �C���v�������e�[�V����
protected:

	// �������ꂽ���b�Z�[�W �}�b�v�֐�
	//{{AFX_MSG(CSPDialog)
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnClear();
	virtual BOOL OnInitDialog();
	afx_msg void OnRadio_Approx();
	afx_msg void OnRadio_Exact();
	afx_msg void OnRadioSpApprox();
	afx_msg void OnRadioSpExact();
	afx_msg void OnRadioSpBoth();
	afx_msg void OnSpGammaSet();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

extern CSPDialog* cspd;

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio �͑O�s�̒��O�ɒǉ��̐錾��}�����܂��B

#endif // !defined(AFX_SPDIALOG_H__61649A10_47BC_11D2_AB66_00E029217E15__INCLUDED_)
