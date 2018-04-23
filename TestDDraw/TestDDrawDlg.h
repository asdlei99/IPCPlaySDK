
// TestDDrawDlg.h : ͷ�ļ�
//

#pragma once

#include "DirectDraw.h"

// CTestDDrawDlg �Ի���
class CTestDDrawDlg : public CDialogEx
{
// ����
public:
	CTestDDrawDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_TESTDDRAW_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

	CDirectDraw *m_pDDraw = nullptr;
// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonDisplay();
};
