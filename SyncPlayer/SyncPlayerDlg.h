
// SyncPlayerDlg.h : ͷ�ļ�
//

#pragma once

struct ThreadParam
{
	void *pDialog;
	CString strFile;
	IPC_PLAYHANDLE hPlayHandle;
	UINT	nID;
};

// CSyncPlayerDlg �Ի���
class CSyncPlayerDlg : public CDialogEx
{
// ����
public:
	CSyncPlayerDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_SYNCPLAYER_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��
	CVideoFrame *m_pVideoFrame = nullptr;
	CWndSizeManger* m_pWndSizeManager = nullptr;

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
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBrowse(UINT nID);
	
	CString m_strFile[4];
	HANDLE  m_hReadFile[4];
	bool	m_bThreadRun = false;
	IPC_PLAYHANDLE	m_hSyncSource = nullptr;
	static  UINT __stdcall ThreadReadFile(void *p)
	{
		ThreadParam *TP = (ThreadParam*)p;
		shared_ptr<ThreadParam> TPPtr(TP);
		return ((CSyncPlayerDlg *)(TP->pDialog))->ReadFileRun(TP->nID);
	}
	UINT	ReadFileRun(UINT nIdex);

	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedButtonTest();
	UINT_PTR	m_nTestTimer = 0;
	
	int m_nTestTimes;
	afx_msg void OnBnClickedButtonStoptest();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
