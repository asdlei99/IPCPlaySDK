
// SyncPlayerDlg.h : ͷ�ļ�
//

#pragma once
#include "IPCPlaySDK.h"
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
	UINT	ReadFileRun2(UINT nIdex);

	IPC_PLAYHANDLE	m_hAsyncPlayHandle = nullptr;
	time_t	m_tFrameOffset = 0;
	MMRESULT	m_nTimeEvent = 0;
	static void  MMTIMECALLBACK(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
	{
		CSyncPlayerDlg* pThis = (CSyncPlayerDlg *)dwUser;
		if (pThis->m_hAsyncPlayHandle && pThis->m_tFrameOffset)
		{
			int nStatus = ipcplay_AsyncSeekFrame(pThis->m_hAsyncPlayHandle, pThis->m_tFrameOffset);
			if (nStatus != IPC_Succeed)
				TraceMsgA("%s ipcplay_AsyncSeekFrame(%d).\n", __FUNCTION__, nStatus);
			pThis->m_tFrameOffset += 200;
		}
	}
	

	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedButtonTest();
	UINT_PTR	m_nTestTimer = 0;
	
	int m_nTestTimes;
	afx_msg void OnBnClickedButtonStoptest();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
