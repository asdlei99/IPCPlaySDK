
// ComboPlay.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CComboPlayApp: 
// �йش����ʵ�֣������ ComboPlay.cpp
//

class CComboPlayApp : public CWinApp
{
public:
	CComboPlayApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CComboPlayApp theApp;