
#pragma once
#include "Common.h"
/// @brief ���ܵ�һ�ļ򵥴����࣬������Ҫ���ڣ�ȴ���ܻ���Ҫ��ʾ���ڵĳ��ϣ���DirectX��DirectSound�ĳ�ʼ��
class  CSimpleWnd
{
public:
	CSimpleWnd(int nWidth = 0, int nHeight = 0)
	{
		m_hWnd = CreateSimpleWindow(nullptr, nWidth, nHeight);
		if (!m_hWnd)
		{
			m_hWnd = nullptr;
			assert(false);
		}
		ShowWindow(m_hWnd, SW_HIDE);
	}

	~CSimpleWnd()
	{
		if (m_hWnd)
		{
			DestroyWindow(m_hWnd);
			m_hWnd = nullptr;
		}
	}
	// ����һ���򵥵Ĳ��ɼ��Ĵ���
	HWND CreateSimpleWindow(IN HINSTANCE hInstance = NULL, int nWidth = 0, int nHeight = 0)
	{
		WNDCLASSEX wcex;

		TCHAR *szWindowClass = _T("SimpleWnd");
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
		wcex.lpfnWndProc = DefWindowProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hIcon = NULL;
		wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wcex.lpszMenuName = NULL;
		wcex.lpszClassName = szWindowClass;
		wcex.hIconSm = NULL;
		RegisterClassEx(&wcex);
		if (!nWidth || !nHeight)
		{
			nWidth = GetSystemMetrics(SM_CXSCREEN);
			nHeight = GetSystemMetrics(SM_CYSCREEN);
		}
		//TraceMsgA("%s nWidth = %d\tnHeight = %d", __FUNCTION__, nWidth, nHeight);
		return ::CreateWindow(szWindowClass,			// ������
			szWindowClass,			// ���ڱ��� 
			WS_EX_TOPMOST | WS_POPUP,	// ���ڷ��
			CW_USEDEFAULT, 			// �������Ͻ�X����
			0, 						// �������Ͻ�Y����
			nWidth, 					// ���ڿ��
			nHeight, 					// ���ڸ߶�
			NULL, 					// �����ھ��
			NULL,						// �˵����
			hInstance,
			NULL);
	}
	const HWND &GetSafeHwnd()
	{
		if (!m_hWnd)
			assert(false);
		return m_hWnd;
	}
private:
	HWND m_hWnd;
};