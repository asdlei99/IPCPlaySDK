
// ComboPlayDlg.h : ͷ�ļ�
//

#pragma once


// CComboPlayDlg �Ի���
class CComboPlayDlg : public CDialogEx
{
// ����
public:
	CComboPlayDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_COMBOPLAY_DIALOG };
	IPC_PLAYHANDLE  m_hPlayer = nullptr;
	HWND m_hWndView = nullptr;
	RECT m_rtBorder;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


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
	afx_msg void OnBnClickedButtonBrowse1();
	afx_msg void OnBnClickedButtonStartplay();
	afx_msg void OnBnClickedButtonStopplay();
	afx_msg void OnBnClickedButtonMoveup();
	afx_msg void OnBnClickedButtonMovedown();
};
