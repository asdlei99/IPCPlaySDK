
// SyncPlayer.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CSyncPlayerApp: 
// �йش����ʵ�֣������ SyncPlayer.cpp
//

class CSyncPlayerApp : public CWinApp
{
public:
	CSyncPlayerApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CSyncPlayerApp theApp;