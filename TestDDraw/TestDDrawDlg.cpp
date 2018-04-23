
// TestDDrawDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "TestDDraw.h"
#include "TestDDrawDlg.h"
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


// CTestDDrawDlg �Ի���



CTestDDrawDlg::CTestDDrawDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTestDDrawDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CTestDDrawDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CTestDDrawDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_DISPLAY, &CTestDDrawDlg::OnBnClickedButtonDisplay)
END_MESSAGE_MAP()


// CTestDDrawDlg ��Ϣ�������

BOOL CTestDDrawDlg::OnInitDialog()
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

	SendDlgItemMessage(IDC_COMBO_PIXELFMT, CB_SETCURSEL, 0, 0);

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CTestDDrawDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CTestDDrawDlg::OnPaint()
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
HCURSOR CTestDDrawDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

enum _Pixfmt
{
	FmtYV12 = 0, 
	FmtYUY2, 
	FmtUYVY, 
	FmtPAL8, 
	FmtRGB555, 
	FmtRGB565, 
	FmtBGR24, 
	FmtRGB32, 
	FmtBGR32
};

void CTestDDrawDlg::OnBnClickedButtonDisplay()
{
	if (m_pDDraw)
		delete m_pDDraw;
	m_pDDraw = new CDirectDraw();
	//�������    
	DDSURFACEDESC2 ddsd = { 0 };
	int nWidth = 640;
	int nHeight = 480;
	
	_Pixfmt nPixfmt = (_Pixfmt)SendDlgItemMessage(IDC_COMBO_PIXELFMT, CB_GETCURSEL);
	switch (nPixfmt)
	{
	case FmtYV12:
		FormatYV12::Build(ddsd, nWidth, nHeight);
		m_pDDraw->Create<FormatYV12>(::GetDlgItem(m_hWnd, IDC_STATIC_FRAME), ddsd);
		break;
	case FmtYUY2:
		FormatYUY2::Build(ddsd, nWidth, nHeight);
		m_pDDraw->Create<FormatYUY2>(::GetDlgItem(m_hWnd, IDC_STATIC_FRAME), ddsd);
		break;
	case FmtUYVY:
		FormatUYVY::Build(ddsd, nWidth, nHeight);
		m_pDDraw->Create<FormatUYVY>(::GetDlgItem(m_hWnd, IDC_STATIC_FRAME), ddsd);
		break;
	case FmtPAL8:
		FormatPAL8::Build(ddsd, nWidth, nHeight);
		m_pDDraw->Create<FormatRGB555>(::GetDlgItem(m_hWnd, IDC_STATIC_FRAME), ddsd);
		break;
	case FmtRGB555:
		FormatRGB555::Build(ddsd, nWidth, nHeight);
		m_pDDraw->Create<FormatRGB555>(::GetDlgItem(m_hWnd, IDC_STATIC_FRAME), ddsd);
		break;
	case FmtRGB565:
		FormatRGB565::Build(ddsd, nWidth, nHeight);
		m_pDDraw->Create<FormatRGB565>(::GetDlgItem(m_hWnd, IDC_STATIC_FRAME), ddsd);
		break;
	case FmtBGR24:
		FormatBGR24::Build(ddsd, nWidth, nHeight);
		m_pDDraw->Create<FormatBGR24>(::GetDlgItem(m_hWnd, IDC_STATIC_FRAME), ddsd);
		break;
	case FmtRGB32:
		FormatRGB32::Build(ddsd, nWidth, nHeight);
		m_pDDraw->Create<FormatRGB32>(::GetDlgItem(m_hWnd, IDC_STATIC_FRAME), ddsd);
		break;
	case FmtBGR32:
		FormatBGR32::Build(ddsd, nWidth, nHeight);
		m_pDDraw->Create<FormatBGR32>(::GetDlgItem(m_hWnd, IDC_STATIC_FRAME), ddsd);
		break;
	default:
		break;
	}
}
