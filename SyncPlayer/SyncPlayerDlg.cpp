
// SyncPlayerDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "SyncPlayer.h"
#include "SyncPlayerDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSyncPlayerDlg �Ի���


CSyncPlayerDlg::CSyncPlayerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSyncPlayerDlg::IDD, pParent)
	, m_nTestTimes(200)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pVideoFrame = new CVideoFrame;
	m_pWndSizeManager = new CWndSizeManger;
	
	m_strFile[0] = _T("E:\\DVORecord\\TestDAV\\SaveRecord_1.dav");
	m_strFile[1] = _T("E:\\DVORecord\\TestDAV\\SaveRecord_2.dav");
	m_strFile[2] = _T("E:\\DVORecord\\TestDAV\\SaveRecord_3.dav");
	m_strFile[3] = _T("E:\\DVORecord\\TestDAV\\SaveRecord_4.dav");
	m_nTestTimes = 200;
}

void CSyncPlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_FILE1, m_strFile[0]);
	DDX_Text(pDX, IDC_EDIT_FILE2, m_strFile[1]);
	DDX_Text(pDX, IDC_EDIT_FILE3, m_strFile[2]);
	DDX_Text(pDX, IDC_EDIT_FILE4, m_strFile[3]);
	DDX_Text(pDX, IDC_EDIT_TEST, m_nTestTimes);
}

BEGIN_MESSAGE_MAP(CSyncPlayerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_START, &CSyncPlayerDlg::OnBnClickedButtonStart)
	ON_WM_SIZE()
	ON_COMMAND_RANGE(IDC_BUTTON_BROWSE1, IDC_BUTTON_BROWSE4, &CSyncPlayerDlg::OnBrowse)
	ON_BN_CLICKED(IDC_BUTTON_STOP, &CSyncPlayerDlg::OnBnClickedButtonStop)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BUTTON_TEST, &CSyncPlayerDlg::OnBnClickedButtonTest)
	ON_BN_CLICKED(IDC_BUTTON_STOPTEST, &CSyncPlayerDlg::OnBnClickedButtonStoptest)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CSyncPlayerDlg ��Ϣ�������

BOOL CSyncPlayerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	CRect rtClient;
	GetDlgItemRect(IDC_STATIC_FRAME, rtClient);
	m_pVideoFrame->Create(1024, rtClient, 4, this);

	UINT nIDArrayBottom[] = { IDC_STATIC1, IDC_EDIT_FILE1, IDC_BUTTON_BROWSE1,
		IDC_STATIC2, IDC_EDIT_FILE2, IDC_BUTTON_BROWSE2,
		IDC_STATIC3, IDC_EDIT_FILE3, IDC_BUTTON_BROWSE3,
		IDC_STATIC4, IDC_EDIT_FILE4, IDC_BUTTON_BROWSE4,
		IDC_BUTTON_START,IDC_BUTTON_STOP };

// 	UINT nIDArrayRight[] = {  };
// 	UINT nIDArrayRightBottom[] = {};
	UINT nIDArrayCenter[] = { IDC_STATIC_FRAME };

	RECT rtDialog;
	GetClientRect(&rtDialog);

	// 	SaveWndPosition(nIDArreayTop, sizeof(nIDArreayTop) / sizeof(UINT), DockTop, rtDialog);
	// 	SaveWndPosition(nIDArrayRight, sizeof(nIDArrayRight) / sizeof(UINT), DockRigth, rtDialog);
	m_pWndSizeManager->SetParentWnd(this);
	m_pWndSizeManager->SaveWndPosition(nIDArrayBottom, sizeof(nIDArrayBottom) / sizeof(UINT), DockBottom);
	m_pWndSizeManager->SaveWndPosition(nIDArrayCenter, sizeof(nIDArrayCenter) / sizeof(UINT), DockCenter, false);
	//m_pWndSizeManager->SaveWndPosition(nIDArrayRight, sizeof(nIDArrayRight) / sizeof(UINT), DockRight);
	//m_pWndSizeManager->SaveWndPosition(nIDArrayRightBottom, sizeof(nIDArrayRightBottom) / sizeof(UINT), (DockType)(DockRight | DockBottom));

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CSyncPlayerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CSyncPlayerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CSyncPlayerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CSyncPlayerDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);

	if (GetDlgItem(IDC_BUTTON_START)->GetSafeHwnd())
	{
		m_pWndSizeManager->OnSize(nType, cx, cy);
		RECT rt;
		m_pWndSizeManager->GetWndCurrentPostion(IDC_STATIC_FRAME, rt);
		m_pVideoFrame->MoveWindow(&rt, true);

	}
}

void CSyncPlayerDlg::OnBrowse(UINT nID)
{
	int nIndex = nID - IDC_BUTTON_BROWSE1;
	if (nIndex >= 0 && nIndex <= 3)
	{
		UINT nEditID[] = { IDC_EDIT_FILE1, IDC_EDIT_FILE2, IDC_EDIT_FILE3, IDC_EDIT_FILE4 };
		TCHAR szText[1024] = { 0 };
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT;
		TCHAR  szFilter[] = _T("Video File(*.dav)|*.dav|Viedo File (*.mp4)|*.mp4|H.264 Video File(*.H264)|*.H264|H.265 Video File(*.H265)|*.H265|All Files (*.*)|*.*||");
		TCHAR szExportLog[MAX_PATH] = { 0 };
		CTime tNow = CTime::GetCurrentTime();
		CFileDialog OpenFile(TRUE, _T("*.mp4"), _T(""), dwFlags, (LPCTSTR)szFilter);
		OpenFile.m_ofn.lpstrTitle = _T("Please Select the files to play");
		CString strFilePath;

		if (OpenFile.DoModal() == IDOK)
		{
			SetDlgItemText(nEditID[nIndex], OpenFile.GetPathName());
		}
	}
}

void CSyncPlayerDlg::OnBnClickedButtonStart()
{
	UpdateData();
	bool bExistSameFile = false;
	CString strText;
	for (int i = 0; i < 4; i++)
	{
		CString strTemp = m_strFile[i];
		for (int k = 0; k < 0; k++)
		{
			if (k == i)
				continue;
			if (strTemp = m_strFile[k])
			{
				bExistSameFile = true;
				strText.Format(_T("%d�ź�%d���ļ�����ͬ��ͬ�����Ų��ܲ���ͬһ���ļ���"), i, k);
				
				break;
			}
		}
	}
	if (bExistSameFile)
		AfxMessageBox(strText);
	else
	{
		for (int i = 0; i < 4; i++)
		{
			ThreadParam *pTP = new ThreadParam;
			pTP->pDialog = this;
			pTP->nID = i;
			m_bThreadRun = true;
			m_hReadFile[i] = (HANDLE)_beginthreadex(nullptr, 0, ThreadReadFile, pTP, CREATE_SUSPENDED, 0);
		}
		for (int i = 0; i < 4; i++)
			ResumeThread(m_hReadFile[i]);
		EnableDlgItem(IDC_BUTTON_START, false);
		EnableDlgItem(IDC_BUTTON_STOP, true);
	}
}

UINT CSyncPlayerDlg::ReadFileRun(UINT nIndex)
{
	IPC_PLAYHANDLE	hPlayHandle = ipcplay_OpenRTStream(m_pVideoFrame->GetPanelWnd(nIndex));
	if (nIndex == 0)
		m_hSyncSource = hPlayHandle;
	else
	{
		while (!m_hSyncSource)
		{
			Sleep(10);
		} ;
	}
	
	int nBufferSize = 16 * 1024;
	byte *pFileBuffer = new byte[nBufferSize];
	ZeroMemory(pFileBuffer, nBufferSize);
	
	try
	{
		CFile file(m_strFile[nIndex], CFile::modeRead | CFile::shareDenyWrite);
		int nWidth = 0, nHeight = 0, nFramerate = 0;
		int nOffsetStart = 0;
		int nOffsetMatched = 0;
		
		IPC_MEDIAINFO MediaHeader;
		MediaHeader.nVideoCodec = CODEC_H264;
		MediaHeader.nAudioCodec = CODEC_UNKNOWN;
		MediaHeader.nVideoWidth = 1920;
		MediaHeader.nVideoHeight = 1080;
		MediaHeader.nFps = 200;
		ipcplay_SetStreamHeader(hPlayHandle, (byte *)&MediaHeader, sizeof(IPC_MEDIAINFO));
		//ipcplay_EnableStreamParser(hPlayHandle, CODEC_H264);

		//ipcplay_SetDecodeDelay(hPlayHandle, 0);
		if (nIndex == 0)
			ipcplay_StartSyncPlay(hPlayHandle, true,nullptr,25);
		else
			ipcplay_StartSyncPlay(hPlayHandle, true, m_hSyncSource, 25);

		while (m_bThreadRun)
		{
			int nReadLength = file.Read(pFileBuffer, nBufferSize);
			if (nReadLength > 0)
			{
				do 
				{
					if (ipcplay_InputDHStream(hPlayHandle, pFileBuffer, nReadLength) == IPC_Succeed)
						break;
				} while (m_bThreadRun);
			}
			Sleep(10);
		}
		ipcplay_Stop(hPlayHandle);
		ipcplay_Close(hPlayHandle);
	}
	catch (CFileException* e)
	{
	}
	delete[]pFileBuffer;
	return 0;
}

void CSyncPlayerDlg::OnBnClickedButtonStop()
{
	CWaitCursor Wait;
	m_bThreadRun = false;
	::MsgWaitForMultipleObjects(4, m_hReadFile, true, INFINITE, QS_ALLINPUT);
	for (int i = 0; i < 4; i++)
	{
		CloseHandle(m_hReadFile[i]);
		m_hReadFile[i] = nullptr;
	}
	EnableDlgItem(IDC_BUTTON_START, true);
	EnableDlgItem(IDC_BUTTON_STOP, false);
}


void CSyncPlayerDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	delete m_pVideoFrame;
	delete m_pWndSizeManager;
}


void CSyncPlayerDlg::OnBnClickedButtonTest()
{
	EnableDlgItem(IDC_EDIT_TEST, FALSE);
	UpdateData();
	m_nTestTimer = SetTimer(1024, 1000, nullptr);
}


void CSyncPlayerDlg::OnBnClickedButtonStoptest()
{
	if (m_nTestTimer)
		KillTimer(m_nTestTimer);
	EnableDlgItem(IDC_EDIT_TEST, TRUE);
}


void CSyncPlayerDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1024)
	{
		if (m_nTestTimes > 0)
		{
			if (m_bThreadRun)
			{
				OnBnClickedButtonStop();
			}
			else
				OnBnClickedButtonStart();
			m_nTestTimes--;
			UpdateData(FALSE);
		}
	}

	CDialogEx::OnTimer(nIDEvent);
}
