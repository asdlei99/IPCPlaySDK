
// TestDisplay.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CTestDisplayApp: 
// �йش����ʵ�֣������ TestDisplay.cpp
//

class CTestDisplayApp : public CWinApp
{
public:
	CTestDisplayApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CTestDisplayApp theApp;