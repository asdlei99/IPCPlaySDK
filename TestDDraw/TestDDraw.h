
// TestDDraw.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CTestDDrawApp: 
// �йش����ʵ�֣������ TestDDraw.cpp
//

class CTestDDrawApp : public CWinApp
{
public:
	CTestDDrawApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CTestDDrawApp theApp;