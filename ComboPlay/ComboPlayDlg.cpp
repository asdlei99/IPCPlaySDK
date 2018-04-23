
// ComboPlayDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ComboPlay.h"
#include "ComboPlayDlg.h"
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


// CComboPlayDlg �Ի���



CComboPlayDlg::CComboPlayDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CComboPlayDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CComboPlayDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CComboPlayDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_BROWSE1, &CComboPlayDlg::OnBnClickedButtonBrowse1)
	ON_BN_CLICKED(IDC_BUTTON_STARTPLAY, &CComboPlayDlg::OnBnClickedButtonStartplay)
	ON_BN_CLICKED(IDC_BUTTON_STOPPLAY, &CComboPlayDlg::OnBnClickedButtonStopplay)
	ON_BN_CLICKED(IDC_BUTTON_MOVEUP, &CComboPlayDlg::OnBnClickedButtonMoveup)
	ON_BN_CLICKED(IDC_BUTTON_MOVEDOWN, &CComboPlayDlg::OnBnClickedButtonMovedown)
END_MESSAGE_MAP()


// CComboPlayDlg ��Ϣ�������

BOOL CComboPlayDlg::OnInitDialog()
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

	m_rtBorder.left = 0;
	m_rtBorder.top = 0;
	m_rtBorder.right = 0;
	m_rtBorder.bottom = 50;

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CComboPlayDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CComboPlayDlg::OnPaint()
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
HCURSOR CComboPlayDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CComboPlayDlg::OnBnClickedButtonBrowse1()
{
	TCHAR szText[1024] = { 0 };
	int nFreePanel = 0;
	DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
	TCHAR  szFilter[] = _T("¼����Ƶ�ļ� (*.mp4)|*.mp4|H.264¼���ļ�(*.H264)|*.H264|H.265¼���ļ�(*.H265)|*.H265|All Files (*.*)|*.*||");
	TCHAR szExportLog[MAX_PATH] = { 0 };
	CTime tNow = CTime::GetCurrentTime();
	CFileDialog OpenDataBase(TRUE, _T("*.mp4"), _T(""), dwFlags, (LPCTSTR)szFilter);
	OpenDataBase.m_ofn.lpstrTitle = _T("��ѡ�񲥷ŵ��ļ�");
	CString strFilePath;
		
	if (OpenDataBase.DoModal() == IDOK)
	{
		strFilePath = OpenDataBase.GetPathName();
		try
		{
			CFile fpMedia((LPCTSTR)strFilePath, CFile::modeRead);
			IPC_MEDIAINFO MediaHeader;
			if (fpMedia.Read(&MediaHeader, sizeof(IPC_MEDIAINFO)) < sizeof(IPC_MEDIAINFO) ||
				(MediaHeader.nMediaTag != IPC_TAG &&	// ͷ��־ �̶�Ϊ   0x44564F4D ���ַ���"MOVD"
				MediaHeader.nMediaTag != GSJ_TAG))
			{
				AfxMessageBox(_T("ָ�����ļ�����һ����Ч��IPC¼���ļ�"));
				return;
			}
			fpMedia.GetPosition();
			fpMedia.Close();
			
			SetDlgItemText(IDC_EDIT_FILEPATH, strFilePath);
		}
		catch (CFileException* e)
		{
			TCHAR szErrorMsg[255] = { 0 };
			e->GetErrorMessage(szErrorMsg, 255);
			AfxMessageBox(szErrorMsg);
			return;
		}
	}
	EnableDlgItem(IDC_BUTTON_STARTPLAY, true);
}


void CComboPlayDlg::OnBnClickedButtonStartplay()
{
	TCHAR szText[1024] = { 0 };
	m_hWndView  = GetDlgItem(IDC_STATIC_FRAME)->GetSafeHwnd();
	CString strFilePath;
	GetDlgItemText(IDC_EDIT_FILEPATH, strFilePath);
	m_hPlayer = ipcplay_OpenFile(m_hWndView, (TCHAR *)(LPCTSTR)strFilePath, NULL, NULL);
	if (!m_hPlayer)
	{
		_stprintf_s(szText, 1024, _T("�޷�����%s�ļ�."), strFilePath);
		AfxMessageBox(szText);
		return;
	}
	ipcplay_SetBorderRect(m_hPlayer, m_hWndView, &m_rtBorder, true);
	if (ipcplay_Start(m_hPlayer, false, true, false) != IPC_Succeed)
	{
		AfxMessageBox(_T("�޷�����������"));
		return;
	}
	EnableDlgItems(m_hWnd, true, 3, IDC_BUTTON_STOPPLAY, IDC_BUTTON_MOVEUP, IDC_BUTTON_MOVEDOWN);
	EnableDlgItem(IDC_BUTTON_BROWSE1, false);
}


void CComboPlayDlg::OnBnClickedButtonStopplay()
{
	if (m_hPlayer)
	{
		ipcplay_Close(m_hPlayer);
		m_hPlayer = nullptr;
		EnableDlgItems(m_hWnd, false, 3, IDC_BUTTON_STOPPLAY, IDC_BUTTON_MOVEUP, IDC_BUTTON_MOVEDOWN);
		EnableDlgItem(IDC_BUTTON_BROWSE1, true);
	}
}


void CComboPlayDlg::OnBnClickedButtonMoveup()
{
	if (m_rtBorder.top > 0)
	{
		m_rtBorder.top--;
		m_rtBorder.bottom = 50 - m_rtBorder.top;
		ipcplay_SetBorderRect(m_hPlayer, m_hWndView, &m_rtBorder, true);
		TraceMsgA("%s Rect.top = %d\tRect.bottom = %d,Height = %d.\n",__FUNCTIONW__, m_rtBorder.top, m_rtBorder.bottom, m_rtBorder.top + m_rtBorder.bottom);
	}
	else
		MessageBeep(MB_ICONEXCLAMATION);
}


void CComboPlayDlg::OnBnClickedButtonMovedown()
{
	if (m_rtBorder.top < 50)
	{
		m_rtBorder.top++;
		m_rtBorder.bottom = 50 - m_rtBorder.top;
		ipcplay_SetBorderRect(m_hPlayer, m_hWndView, &m_rtBorder, true);
		TraceMsgA("%s Rect.top = %d\tRect.bottom = %d,Height = %d.\n",__FUNCTIONW__, m_rtBorder.top, m_rtBorder.bottom, m_rtBorder.top + m_rtBorder.bottom);
	}
	else
		MessageBeep(MB_ICONEXCLAMATION);
}
