/////////////////////////////////////////////////////////////////////////
/// @file  IPCPlaySDK.cpp
/// Copyright (c) 2015, xionggao.lee
/// All rights reserved.  
/// @brief IPC����SDKʵ��
/// @version 1.0  
/// @author  xionggao.lee
/// @date  2015.12.17
///   
/// �޶�˵��������汾 
/////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <list>
#include <assert.h>
#include <memory>
#include <Shlwapi.h>
#include "IPCPlaySDK.h"
#include "IPCPlayer.hpp"

CAvRegister CIPCPlayer::avRegister;
//CTimerQueue CIPCPlayer::m_TimeQueue;
CCriticalSectionProxyPtr CIPCPlayer::m_csDsoundEnum = make_shared<CCriticalSectionProxy>();
shared_ptr<CDSoundEnum> CIPCPlayer::m_pDsoundEnum = nullptr;/*= make_shared<CDSoundEnum>()*/;	///< ��Ƶ�豸ö����
#ifdef _DEBUG
int	CIPCPlayer::m_nGloabalCount = 0;
CCriticalSectionProxyPtr CIPCPlayer::m_pCSGlobalCount = make_shared<CCriticalSectionProxy>();
#endif

#ifdef _DEBUG
extern CCriticalSectionProxy g_csPlayerHandles;
extern UINT	g_nPlayerHandles;
#endif
//shared_ptr<CDSound> CDvoPlayer::m_pDsPlayer = make_shared<CDSound>(nullptr);
shared_ptr<CSimpleWnd> CIPCPlayer::m_pWndDxInit = make_shared<CSimpleWnd>();	///< ��Ƶ��ʾʱ�����Գ�ʼ��DirectX�����ش��ڶ���

using namespace std;
using namespace std::tr1;

//IPC IPC ��������ͷ
struct IPC_StreamHeader
{
	UINT		    chn;	            //��Ƶ����ͨ���ţ�ȡֵ0~MAX-1.
	UINT		    stream;             //������ţ�0:��������1����������2������������
	UINT		    frame_type;         //֡���ͣ���Χ�ο�APP_NET_TCP_STREAM_TYPE
	UINT		    frame_num;          //֡��ţ���Χ0~0xFFFFFFFF.
	UINT		    sec;	            //ʱ���,��λ���룬Ϊ1970��������������
	UINT		    usec;               //ʱ���,��λ��΢�롣
	UINT		    rev[2];
};

///	@brief			���ڲ���IPC˽�и�ʽ��¼���ļ�
///	@param [in]		szFileName		Ҫ���ŵ��ļ���
///	@param [in]		hWnd			��ʾͼ��Ĵ���
/// @param [in]		pPlayCallBack	����ʱ�Ļص�����ָ��
/// @param [in]		pUserPtr		��pPlayCallBack���ص��û��Զ���ָ��
/// @param [in]		szLogFile		��־�ļ���,��Ϊnull���򲻿�����־
///	@return			�������ɹ�������һ��IPC_PLAYHANDLE���͵Ĳ��ž�������к�����
///	�ź�����Ҫʹ��Щ�ӿڣ�������ʧ���򷵻�NULL,����ԭ��ɲο�
///	GetLastError�ķ���ֵ
IPCPLAYSDK_API IPC_PLAYHANDLE	ipcplay_OpenFileA(IN HWND hWnd, IN char *szFileName, FilePlayProc pPlayCallBack, void *pUserPtr,char *szLogFile)
{
	if (!szFileName)
	{
		SetLastError(IPC_Error_InvalidParameters);
		return nullptr;
	}
	if (!PathFileExistsA(szFileName))
	{
		SetLastError(ERROR_FILE_NOT_FOUND);
		return nullptr;
	}
	CIPCPlayer *pPlayer = nullptr;
	try
	{
		CIPCPlayer *pPlayer = _New CIPCPlayer(hWnd, szFileName, szLogFile);
		if (pPlayer)
		{
			pPlayer->SetCallBack(FilePlayer, pPlayCallBack, pUserPtr);
#if _DEBUG
			g_csPlayerHandles.Lock();
			g_nPlayerHandles++;
			g_csPlayerHandles.Unlock();
#endif
			return pPlayer;
		}
		else
			return nullptr;
	}
	catch (std::exception &e)
	{
		if (pPlayer)
			delete pPlayer;
		return nullptr;
	}
}

///	@brief			���ڲ���IPC˽�и�ʽ��¼���ļ�
///	@param [in]		szFileName		Ҫ���ŵ��ļ���
///	@param [in]		hWnd			��ʾͼ��Ĵ���
/// @param [in]		pPlayCallBack	����ʱ�Ļص�����ָ��
/// @param [in]		pUserPtr		��pPlayCallBack���ص��û��Զ���ָ��
/// @param [in]		szLogFile		��־�ļ���,��Ϊnull���򲻿�����־
///	@return			�������ɹ�������һ��IPC_PLAYHANDLE���͵Ĳ��ž�������к�����
///	�ź�����Ҫʹ��Щ�ӿڣ�������ʧ���򷵻�NULL,����ԭ��ɲο�
///	GetLastError�ķ���ֵ
IPCPLAYSDK_API IPC_PLAYHANDLE	ipcplay_OpenFileW(IN HWND hWnd, IN WCHAR *szFileName, FilePlayProc pPlayCallBack, void *pUserPtr, char *szLogFile)
{
	if (!szFileName || !PathFileExistsW(szFileName))
		return nullptr;
	char szFilenameA[MAX_PATH] = { 0 };
	WideCharToMultiByte(CP_ACP, 0, szFileName, -1, szFilenameA, MAX_PATH, NULL, NULL);
	return ipcplay_OpenFileA(hWnd, szFilenameA, pPlayCallBack,pUserPtr,szLogFile);
}

///	@brief			��ʼ�������ž��,������������
///	@param [in]		hWnd			��ʾͼ��Ĵ���
/// @param [in]		nBufferSize	������ʱ���������Ƶ֡����������,��ֵ�������0�����򽫷���null
/// @param [in]		szLogFile		��־�ļ���,��Ϊnull���򲻿�����־
///	@return			�������ɹ�������һ��IPC_PLAYHANDLE���͵Ĳ��ž�������к�����
///	�ź�����Ҫʹ��Щ�ӿڣ�������ʧ���򷵻�NULL,����ԭ��ɲο�GetLastError�ķ���ֵ
IPCPLAYSDK_API IPC_PLAYHANDLE	ipcplay_OpenRTStream(IN HWND hWnd, IN int nBufferSize , char *szLogFile)
{
	if ((hWnd && !IsWindow(hWnd))/* || !szStreamHeader || !nHeaderSize*/)
		return nullptr;
	if (0 == nBufferSize)
	{
		SetLastError(IPC_Error_InvalidCacheSize);
		return nullptr;
	}
	try
	{
		CIPCPlayer *pPlayer = _New CIPCPlayer(hWnd, nBufferSize, szLogFile);
		if (!pPlayer)
			return nullptr;
		if (szLogFile)
			TraceMsgA("%s %s.\n", __FUNCTION__, szLogFile);
#if _DEBUG
		g_csPlayerHandles.Lock();
		g_nPlayerHandles++;
		g_csPlayerHandles.Unlock();
#endif
		return pPlayer;
	}
	catch (...)
	{
		return nullptr;
	}
}

///	@brief			��ʼ�������ž��,������������
/// @param [in]		hUserHandle		�������Ӿ��
///	@param [in]		hWnd			��ʾͼ��Ĵ���
/// @param [in]		nMaxFramesCache	������ʱ���������Ƶ֡����������
/// @param [in]		szLogFile		��־�ļ���,��Ϊnull���򲻿�����־
///	@return			�������ɹ�������һ��IPC_PLAYHANDLE���͵Ĳ��ž�������к�����
///	�ź�����Ҫʹ��Щ�ӿڣ�������ʧ���򷵻�NULL,����ԭ��ɲο�GetLastError�ķ���ֵ
IPCPLAYSDK_API IPC_PLAYHANDLE	ipcplay_OpenStream(IN HWND hWnd, byte *szStreamHeader, int nHeaderSize, IN int nMaxFramesCache, char *szLogFile)
{
 	if ((hWnd && !IsWindow(hWnd))/* || !szStreamHeader || !nHeaderSize*/)
 		return nullptr;
	if (0 == nMaxFramesCache)
	{
		SetLastError(IPC_Error_InvalidCacheSize);
		return nullptr;
	}
	CIPCPlayer *pPlayer = _New CIPCPlayer(hWnd,nullptr,szLogFile);
	if (!pPlayer)
		return nullptr;
	if (szLogFile)
		TraceMsgA("%s %s.\n", __FUNCTION__, szLogFile);
	if (szStreamHeader && nHeaderSize)
	{
		pPlayer->SetMaxFrameCache(nMaxFramesCache);
		int nDvoError = pPlayer->SetStreamHeader((CHAR *)szStreamHeader, nHeaderSize);
		if (nDvoError == IPC_Succeed)
		{
#if _DEBUG
			g_csPlayerHandles.Lock();
			g_nPlayerHandles++;
			g_csPlayerHandles.Unlock();
#endif
			return pPlayer;
		}
		else
		{
			SetLastError(nDvoError);
			delete pPlayer;
			return nullptr;
		}
	}
	else
	{
#if _DEBUG
		g_csPlayerHandles.Lock();
		g_nPlayerHandles++;
		g_csPlayerHandles.Unlock();
#endif
		return pPlayer;
	}
}

IPCPLAYSDK_API int	ipcplay_SetStreamHeader(IN IPC_PLAYHANDLE hPlayHandle, byte *szStreamHeader, int nHeaderSize)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	return pPlayer->SetStreamHeader((CHAR *)szStreamHeader, nHeaderSize);
}

/// @brief			�رղ��ž��
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [in]		bAsyncClose		�Ƿ����첽�ر�
///	-# true			ˢ�»����Ա���ɫ���
///	-# false			��ˢ�»���,�������һ֡��ͼ��
/// @retval			0	�����ɹ�
/// @retval			-1	���������Ч
/// @remark			�رղ��ž���ᵼ�²��Ž�����ȫ��ֹ������ڴ�ȫ�����ͷ�,Ҫ�ٶȲ��ű������´��ļ���������
IPCPLAYSDK_API int ipcplay_Close(IN IPC_PLAYHANDLE hPlayHandle, bool bAsyncClose/* = true*/)
{
	TraceFunction();
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
#ifdef _DEBUG
	if (strlen(pPlayer->m_szLogFileName) > 0)
		TraceMsgA("%s %s.\n", __FUNCTION__, pPlayer->m_szLogFileName);
	DxTraceMsg("%s DvoPlayer Object:%d.\n", __FUNCTION__, pPlayer->m_nObjIndex);
	
#endif
	if (!pPlayer->StopPlay(25))
	{
		DxTraceMsg("%s Async close IPCPlay Object:%8X.\n",__FUNCTION__,pPlayer);
		g_csListPlayertoFree.Lock();
		g_listPlayerAsyncClose.push_back(hPlayHandle);
		g_csListPlayertoFree.Unlock();
	}
	else
	{
#ifdef _DEBUG
		g_csPlayerHandles.Lock();
		g_nPlayerHandles--;
		g_csPlayerHandles.Unlock();
#endif
		delete pPlayer;
	}

	return 0;
}

/// @brief			����������־
/// @param			szLogFile		��־�ļ���,�˲���ΪNULLʱ����ر���־
/// @retval			0	�����ɹ�
/// @retval			-1	���������Ч
/// @remark			Ĭ������£����Ὺ����־,���ô˺�����Ὺ����־���ٴε���ʱ���ر���־
IPCPLAYSDK_API int	EnableLog(IN IPC_PLAYHANDLE hPlayHandle, char *szLogFile)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	pPlayer->EnableRunlog(szLogFile);
	return 0;
}

IPCPLAYSDK_API int ipcplay_SetBorderRect(IN IPC_PLAYHANDLE hPlayHandle,HWND hWnd, LPRECT pRectBorder,bool bPercent)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	if (!pRectBorder || (pRectBorder->left < 0 || pRectBorder->right < 0 || pRectBorder->top < 0 || pRectBorder->bottom < 0))
		return IPC_Error_InvalidParameters;
	return pPlayer->SetBorderRect(hWnd, pRectBorder, bPercent);
}

IPCPLAYSDK_API int ipcplay_RemoveBorderRect(IN IPC_PLAYHANDLE hPlayHandle,HWND hWnd)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	pPlayer->RemoveBorderRect(hWnd);
	return IPC_Succeed;
}

IPCPLAYSDK_API int ipcplay_AddWindow(IN IPC_PLAYHANDLE hPlayHandle, HWND hRenderWnd, LPRECT pRectRender,bool bPercent)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	return pPlayer->AddRenderWindow(hRenderWnd, pRectRender, bPercent);
}

IPCPLAYSDK_API int ipcplay_RemoveWindow(IN IPC_PLAYHANDLE hPlayHandle, HWND hRenderWnd)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	return pPlayer->RemoveRenderWindow(hRenderWnd);
}

IPCPLAYSDK_API int  ipcplay_GetRenderWindows(IN IPC_PLAYHANDLE hPlayHandle, INOUT HWND* hWndArray, INOUT int& nCount)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	return pPlayer->GetRenderWindows(hWndArray, nCount);
}

/// @brief			����������
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @retval			0	�����ɹ�
/// @retval			1	������������
/// @retval			-1	���������Ч
/// @remark			����������ʱ����Ӧ��֡������ʵ��δ�������ţ����Ǳ����˲��Ŷ����У�Ӧ�ø���ipcplay_     
///					�ķ���ֵ���жϣ��Ƿ�������ţ���˵��������������Ӧ����ͣ����
IPCPLAYSDK_API int ipcplay_InputStream(IN IPC_PLAYHANDLE hPlayHandle, unsigned char *szFrameData, int nFrameSize)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	return pPlayer->InputStream(szFrameData, nFrameSize,0);
}

/// @brief			���������ʵʱ����
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @retval			0	�����ɹ�
/// @retval			1	������������
/// @retval			-1	���������Ч
/// @remark			����������ʱ����Ӧ��֡������ʵ��δ�������ţ����Ǳ����˲��Ŷ����У�Ӧ�ø���ipcplay_PlayStream
///					�ķ���ֵ���жϣ��Ƿ�������ţ���˵��������������Ӧ����ͣ����
IPCPLAYSDK_API int ipcplay_InputIPCStream(IN IPC_PLAYHANDLE hPlayHandle, IN byte *pFrameData, IN int nFrameType, IN int nFrameLength, int nFrameNum, time_t nFrameTime)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
	{
		assert(false);
		return IPC_Error_InvalidParameters;
	}
	
	return pPlayer->InputStream(pFrameData,nFrameType,nFrameLength,nFrameNum,nFrameTime);
}

/// @brief			��ʼ����
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
///	@param [in]		bEnableAudio	�Ƿ񲥷���Ƶ
///	-# true			��������
///	-# false		�ر�����
/// @param [in]		bEnableHaccel	�Ƿ���Ӳ����
/// #- true			����Ӳ���빦��
/// #- false		�ر�Ӳ���빦��
/// @retval			0	�����ɹ�
/// @retval			-1	���������Ч
/// @remark			������Ӳ���룬���Կ���֧�ֶ�Ӧ����Ƶ����Ľ���ʱ�����Զ��л�����������״̬,��ͨ��
///					ipcplay_GetHaccelStatus�ж��Ƿ��Ѿ�����Ӳ����
IPCPLAYSDK_API int ipcplay_Start(IN IPC_PLAYHANDLE hPlayHandle, 
									IN bool bEnableAudio/* = false*/, 
									IN bool bFitWindow /*= true*/, 
									IN bool bEnableHaccel/* = false*/)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	return pPlayer->StartPlay(bEnableAudio, bEnableHaccel,bFitWindow);
}

/// @brief			��ʼͬ������
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [in]		bFitWindow		��Ƶ�Ƿ���Ӧ����
/// @param [in]		pSyncSource		ͬ��Դ��Ϊ��һIPCPlaySDK �������ͬ��ԴΪnull,�򴴽�ͬ��ʱ�ӣ�����ͬ��
/// @param [in]		nVideoFPS		��Ƶ֡��
/// #- true			��Ƶ��������,�������ͼ������,���ܻ����ͼ�����
/// #- false		ֻ��ͼ��ԭʼ�����ڴ�������ʾ,������������,���Ժ�ɫ������ʾ
/// @retval			0	�����ɹ�
/// @retval			1	������������
/// @retval			-1	���������Ч
/// @remark			��pSyncSourceΪnull,��ǰ�Ĳ�������Ϊͬ��Դ��nVideoFPS����Ϊ0�����򷵻�IPC_Error_InvalidParameters����
///					��pSyncSource��Ϊnull����ǰ��������pSyncSourceΪͬ��Դ��nVideoFPSֵ������
int ipcplay_StartAsyncPlay(IN IPC_PLAYHANDLE hPlayHandle, bool bFitWindow , void *pSyncSource, int nVideoFPS)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;

	CIPCPlayer *pSyncPlayer = (CIPCPlayer *)pSyncSource;
	if (pSyncPlayer)
	{
		if (pSyncPlayer->nSize != sizeof(CIPCPlayer))
			return IPC_Error_InvalidParameters;
	}

	return pPlayer->StartAsyncPlay(bFitWindow, pSyncPlayer, nVideoFPS);
}

/// @brief			���ý�����ʱ
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
///	@param [in]		nDecodeDelay	������ʱ����λΪms
///	-# -1			ʹ��Ĭ����ʱ
///	-# 0			����ʱ
/// -# n			������ʱ
IPCPLAYSDK_API void ipcplay_SetDecodeDelay(IPC_PLAYHANDLE hPlayHandle, int nDecodeDelay)
{
	if (!hPlayHandle)
		return ;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return ;
	pPlayer->SetDecodeDelay(nDecodeDelay);
}
/// @brief			�жϲ������Ƿ����ڲ�����
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @retval			true	���������ڲ�����
/// @retval			false	�������ֹͣ����
IPCPLAYSDK_API bool ipcplay_IsPlaying(IN IPC_PLAYHANDLE hPlayHandle)
{
	if (!hPlayHandle)
		return false;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return false;
	return pPlayer->IsPlaying();
}
/// @brief ��λ������,�ڴ��ڴ�С�仯�ϴ��Ҫ�л����Ŵ���ʱ�����鸴λ���������������������ܻ������½�
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [in]		hWnd			��ʾ��Ƶ�Ĵ���
/// @param [in]		nWidth			���ڿ��,�ò�����δʹ��,����Ϊ0
/// @param [in]		nHeight			���ڸ߶�,�ò�����δʹ��,����Ϊ0
IPCPLAYSDK_API int  ipcplay_Reset(IN IPC_PLAYHANDLE hPlayHandle, HWND hWnd, int nWidth , int nHeight)
{
// 	if (!hPlayHandle)
// 		return IPC_Error_InvalidParameters;
// 	CDvoPlayer *pPlayer = (CDvoPlayer *)hPlayHandle;
// 	if (pPlayer->nSize != sizeof(CDvoPlayer))
// 		return IPC_Error_InvalidParameters;
// 	if (pPlayer->Reset(hWnd, nWidth, nHeight))
		return IPC_Succeed;
//	else
//		return IPC_Error_DxError;
}

/// @brief			ʹ��Ƶ��Ӧ����
/// @param [in]		bFitWindow		��Ƶ�Ƿ���Ӧ����
/// #- true			��Ƶ��������,�������ͼ������,���ܻ����ͼ�����
/// #- false		ֻ��ͼ��ԭʼ�����ڴ�������ʾ,������������,����ԭʼ������ʾ
IPCPLAYSDK_API int ipcplay_FitWindow(IN IPC_PLAYHANDLE hPlayHandle, bool bFitWindow )
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	pPlayer->FitWindow(bFitWindow);
	return IPC_Succeed;
}

/// @brief			ֹͣ����
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [in]		bStopAsync		�Ƿ��첽�ر�
/// @retval			0	�����ɹ�
/// @retval			-1	���������Ч
IPCPLAYSDK_API int ipcplay_Stop(IN IPC_PLAYHANDLE hPlayHandle,bool bStopAsync )
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	pPlayer->StopPlay(bStopAsync);
	return IPC_Succeed;
}

/// @brief			��ͣ�ļ�����
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @retval			0	�����ɹ�
/// @retval			-1	���������Ч
/// @remark			����һ�����غ���������ǰ����Ѿ����ڲ���״̬���״ε���ipcplay_Pauseʱ���Ქ�Ž��������ͣ
///					�ٴε���ʱ������ٶȲ���
IPCPLAYSDK_API int ipcplay_Pause(IN IPC_PLAYHANDLE hPlayHandle)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	pPlayer->Pause();
	return IPC_Succeed;
}

/// @brief			��ղ��Ż���
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @retval			0	�����ɹ�
/// @retval			-1	���������Ч
IPCPLAYSDK_API int ipcplay_ClearCache(IN IPC_PLAYHANDLE hPlayHandle)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	pPlayer->ClearFrameCache();
	return IPC_Succeed;
}


/// @brief			����Ӳ���빦��
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [in]		bEnableHaccel	�Ƿ���Ӳ���빦��
/// #- true			����Ӳ���빦��
/// #- false		�ر�Ӳ���빦��
/// @retval			0	�����ɹ�
/// @retval			-1	���������Ч
/// @remark			�����͹ر�Ӳ���빦�ܱ���Ҫipcplay_Start֮ǰ���ò�����Ч
IPCPLAYSDK_API int  ipcplay_EnableHaccel(IN IPC_PLAYHANDLE hPlayHandle, IN bool bEnableHaccel/* = false*/)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	return pPlayer->EnableHaccel(bEnableHaccel);
}

/// @brief			��ȡӲ����״̬
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [out]	bEnableHaccel	���ص�ǰhPlayHandle�Ƿ��ѿ���Ӳ���빦��
/// #- true			�ѿ���Ӳ���빦��
/// #- false		δ����Ӳ���빦��
/// @retval			0	�����ɹ�
/// @retval			-1	���������Ч
IPCPLAYSDK_API int  ipcplay_GetHaccelStatus(IN IPC_PLAYHANDLE hPlayHandle, OUT bool &bEnableHaccel)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	bEnableHaccel = pPlayer->GetHaccelStatus();
	return IPC_Succeed;
}

/// @brief			����Ƿ�֧���ض���Ƶ�����Ӳ����
/// @param [in]		nCodec		��Ƶ�����ʽ,@see IPC_CODEC
/// @retval			true		֧��ָ����Ƶ�����Ӳ����
/// @retval			false		��֧��ָ����Ƶ�����Ӳ����
IPCPLAYSDK_API bool  ipcplay_IsSupportHaccel(IN IPC_CODEC nCodec)
{
	return CIPCPlayer::IsSupportHaccel(nCodec);
}

/// @brief			ȡ�õ�ǰ������Ƶ��֡��Ϣ
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [out]	pFilePlayInfo	�ļ����ŵ������Ϣ���μ�@see FilePlayInfo
/// @retval			0	�����ɹ�
/// @retval			-1	���������Ч
IPCPLAYSDK_API int  ipcplay_GetPlayerInfo(IN IPC_PLAYHANDLE hPlayHandle, OUT PlayerInfo *pPlayerInfo)
{
	if (!hPlayHandle || !pPlayerInfo)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	return pPlayer->GetPlayerInfo(pPlayerInfo);
}

/// @brief			��ȡ���Ų��ŵ���Ƶͼ��
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [in]		szFileName		Ҫ������ļ���
/// @param [in]		nFileFormat		�����ļ��ı����ʽ,@see SNAPSHOT_FORMAT����
/// @retval			0	�����ɹ�
/// @retval			-1	���������Ч
IPCPLAYSDK_API int  ipcplay_SnapShotW(IN IPC_PLAYHANDLE hPlayHandle, IN WCHAR *szFileName, IN SNAPSHOT_FORMAT nFileFormat/* = XIFF_JPG*/)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	CHAR szFileNameA[MAX_PATH] = { 0 };
	WideCharToMultiByte(CP_ACP, 0, szFileName, -1, szFileNameA, MAX_PATH, NULL, NULL);
	return ipcplay_SnapShotA(hPlayHandle, szFileNameA, nFileFormat);
}

/// @brief			��ȡ���Ų��ŵ���Ƶͼ��
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [in]		szFileName		Ҫ������ļ���
/// @param [in]		nFileFormat		�����ļ��ı����ʽ,@see SNAPSHOT_FORMAT����
/// @retval			0	�����ɹ�
/// @retval			-1	���������Ч
IPCPLAYSDK_API int  ipcplay_SnapShotA(IN IPC_PLAYHANDLE hPlayHandle, IN CHAR *szFileName, IN SNAPSHOT_FORMAT nFileFormat/* = XIFF_JPG*/)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	return pPlayer->SnapShot(szFileName, nFileFormat);
}

/// @brief			���ò��ŵ�����
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [in]		nVolume			Ҫ���õ�����ֵ��ȡֵ��Χ0~100��Ϊ0ʱ����Ϊ����
/// @retval			0	�����ɹ�
/// @retval			-1	���������Ч
IPCPLAYSDK_API int  ipcplay_SetVolume(IN IPC_PLAYHANDLE hPlayHandle, IN int nVolume)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	pPlayer->SetVolume(nVolume);
	return IPC_Succeed;
}

/// @brief			ȡ�õ�ǰ���ŵ�����
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [out]	nVolume			��ǰ������ֵ��ȡֵ��Χ0~100��Ϊ0ʱ����Ϊ����
/// @retval			0	�����ɹ�
/// @retval			-1	���������Ч
IPCPLAYSDK_API int  ipcplay_GetVolume(IN IPC_PLAYHANDLE hPlayHandle, OUT int &nVolume)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	pPlayer->SetVolume(nVolume);
	return IPC_Succeed;
}

/// @brief			���õ�ǰ���ŵ��ٶȵı���
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [in]		nPlayRate		��ǰ�Ĳ��ŵı���,����1ʱΪ���ٲ���,С��1ʱΪ���ٲ��ţ�����Ϊ0��С��0
/// @retval			0	�����ɹ�
/// @retval			-1	���������Ч
IPCPLAYSDK_API int  ipcplay_SetRate(IN IPC_PLAYHANDLE hPlayHandle, IN float fPlayRate)
{
	if (!hPlayHandle ||fPlayRate <= 0.0f )
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	return pPlayer->SetRate(fPlayRate);
}

/// @brief			������һ֡
/// @retval			0	�����ɹ�
/// @retval			-1	���������Ч
/// @retval			-24	������δ��ͣ
/// @remark			�ú����������ڵ�֡����
IPCPLAYSDK_API int  ipcplay_SeekNextFrame(IN IPC_PLAYHANDLE hPlayHandle)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	if (pPlayer->IsFilePlayer())
	{
		return pPlayer->SeekNextFrame();
	}
	else
		return IPC_Error_NotFilePlayer;
}

/// @brief			��Ծ��ָ��Ƶ֡���в���
/// @param [in]		hPlayHandle		��ipcplay_OpenFile���صĲ��ž��
/// @param [in]		nFrameID		Ҫ����֡����ʼID
/// @param [in]		bUpdate		�Ƿ���»���,bUpdateΪtrue�����Ը��»���,�����򲻸���
/// @retval			0	�����ɹ�
/// @retval			-1	���������Ч
/// @remark			1.����ָ��ʱ����Ӧ֡Ϊ�ǹؼ�֡��֡�Զ��ƶ����ͽ��Ĺؼ�֡���в���
///					2.����ָ��֡Ϊ�ǹؼ�֡��֡�Զ��ƶ����ͽ��Ĺؼ�֡���в���
///					3.ֻ���ڲ�����ʱ,bUpdate��������Ч
IPCPLAYSDK_API int  ipcplay_SeekFrame(IN IPC_PLAYHANDLE hPlayHandle, IN int nFrameID,bool bUpdate)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	if (pPlayer->IsFilePlayer())
	{
		return pPlayer->SeekFrame(nFrameID,bUpdate);
	}
	else
		return IPC_Error_NotFilePlayer;
}

/// @brief			��Ծ��ָ��ʱ��ƫ�ƽ��в���
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [in]		tTimeOffset		Ҫ���ŵ���ʼʱ��,��λΪ��
/// @param [in]		bUpdate		�Ƿ���»���,bUpdateΪtrue�����Ը��»���,�����򲻸���
/// @retval			0	�����ɹ�
/// @retval			-1	���������Ч
/// @remark			1.����ָ��ʱ����Ӧ֡Ϊ�ǹؼ�֡��֡�Զ��ƶ����ͽ��Ĺؼ�֡���в���
///					2.����ָ��֡Ϊ�ǹؼ�֡��֡�Զ��ƶ����ͽ��Ĺؼ�֡���в���
///					3.ֻ���ڲ�����ʱ,bUpdate��������Ч
IPCPLAYSDK_API int  ipcplay_SeekTime(IN IPC_PLAYHANDLE hPlayHandle, IN time_t tTimeOffset,bool bUpdate)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	if (pPlayer->IsFilePlayer())	
		return pPlayer->SeekTime(tTimeOffset, bUpdate);
	else
		return IPC_Error_NotFilePlayer;
}

/// @brief ���ļ��ж�ȡһ֡����ȡ�����Ĭ��ֵΪ0,SeekFrame��SeekTime���趨�����λ��
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [out]	pFrameBuffer	֡���ݻ�����
/// @param [out]	nBufferSize		֡�������Ĵ�С
IPCPLAYSDK_API int  ipcplay_GetFrame(IN IPC_PLAYHANDLE hPlayHandle, OUT byte **pFrameBuffer, OUT UINT &nBufferSize)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	if (pPlayer->IsFilePlayer())
		return pPlayer->GetFrame(pFrameBuffer, nBufferSize);
	else
		return IPC_Error_NotFilePlayer;
}

/// @brief			���������Ƶ֡�ĳߴ�,Ĭ��������Ƶ�ĳߴ�Ϊ256K,����Ƶ֡����256Kʱ,
/// ���ܻ����ļ���ȡ�ļ�����,�����Ҫ������Ƶ֡�Ĵ�С,��ipcplay_Startǰ���ò���Ч
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [in]		nMaxFrameSize	�����Ƶ֡�ĳߴ�
/// @retval			0	�����ɹ�
/// @retval			-1	���������Ч
/// @remark			����ָ��ʱ����Ӧ֡Ϊ�ǹؼ�֡��֡�Զ��ƶ����ͽ��Ĺؼ�֡���в���
IPCPLAYSDK_API int  ipcplay_SetMaxFrameSize(IN IPC_PLAYHANDLE hPlayHandle, IN UINT nMaxFrameSize)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	return pPlayer->SetMaxFrameSize(nMaxFrameSize);
}


/// @brief			ȡ���ļ�����ʱ,֧�ֵ������Ƶ֡�ĳߴ�,Ĭ��������Ƶ�ĳߴ�Ϊ256K,����Ƶ֡
/// ����256Kʱ,���ܻ����ļ���ȡ�ļ�����,�����Ҫ������Ƶ֡�Ĵ�С,��ipcplay_Startǰ���ò���Ч
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [in]		nMaxFrameSize	�����Ƶ֡�ĳߴ�
/// @retval			0	�����ɹ�
/// @retval			-1	���������Ч
/// @remark			����ָ��ʱ����Ӧ֡Ϊ�ǹؼ�֡��֡�Զ��ƶ����ͽ��Ĺؼ�֡���в���
IPCPLAYSDK_API int  ipcplay_GetMaxFrameSize(IN IPC_PLAYHANDLE hPlayHandle, INOUT UINT &nMaxFrameSize)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	if (pPlayer->IsFilePlayer())
	{
		nMaxFrameSize = pPlayer->GetMaxFrameSize();
		return IPC_Succeed;
	}
	else
		return IPC_Error_NotFilePlayer;
}

/// @brief			������һ֡
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @retval			0	�����ɹ�
/// @retval			-1	���������Ч
/// @remark			�ú����������ڵ�֡����,�ú���������δʵ��
// IPCIPCPLAYSDK_API int  ipcplay_SeekNextFrame(IN IPC_PLAYHANDLE hPlayHandle)
// {
// 	return IPC_Succeed;
// }


/// @brief ������Ƶ���Ų���
/// @param nPlayFPS		��Ƶ������֡��
/// @param nSampleFreq	����Ƶ��
/// @param nSampleBit	����λ��
/// @remark �ڲ�����Ƶ֮ǰ��Ӧ��������Ƶ���Ų���,SDK�ڲ�Ĭ�ϲ���nPlayFPS = 50��nSampleFreq = 8000��nSampleBit = 16
///         ����Ƶ���Ų�����SDK�ڲ�Ĭ�ϲ���һ�£����Բ���������Щ����
IPCPLAYSDK_API int  ipcplay_SetAudioPlayParameters(IN IPC_PLAYHANDLE hPlayHandle, DWORD nPlayFPS /*= 50*/, DWORD nSampleFreq/* = 8000*/, WORD nSampleBit/* = 16*/)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	pPlayer->SetAudioPlayParameters(nPlayFPS, nSampleFreq, nSampleBit);
	return IPC_Succeed;
}

/// @brief			��/����Ƶ����
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [in]		bEnable			�Ƿ񲥷���Ƶ
/// -#	true		������Ƶ����
/// -#	false		������Ƶ����
/// @retval			0	�����ɹ�
/// @retval			-1	���������Ч
IPCPLAYSDK_API int  ipcplay_EnableAudio(IN IPC_PLAYHANDLE hPlayHandle, bool bEnable/* = true*/)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	return pPlayer->EnableAudio(bEnable);
}

/// @brief			ˢ�²��Ŵ���
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @retval			0	�����ɹ�
/// @retval			-1	���������Ч
/// @remark			�ù���һ�����ڲ��Ž�����ˢ�´��ڣ��ѻ�����Ϊ��ɫ
IPCPLAYSDK_API int  ipcplay_Refresh(IN IPC_PLAYHANDLE hPlayHandle)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	pPlayer->Refresh();
	return IPC_Succeed;
}

/// @brief			���û�ȡ�û��ص��ӿ�,ͨ���˻ص����û���ͨ���ص��õ�һЩ����,��Եõ���������һЩ����
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [in]		�ص����������� @see IPC_CALLBACK
/// @param [in]		pUserCallBack	�ص�����ָ��
/// @param [in]		pUserPtr		�û��Զ���ָ�룬�ڵ��ûص�ʱ�����ᴫ�ش�ָ��
IPCPLAYSDK_API int ipcplay_SetCallBack(IN IPC_PLAYHANDLE hPlayHandle, IPC_CALLBACK nCallBackType, IN void *pUserCallBack, IN void *pUserPtr)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	pPlayer->SetCallBack(nCallBackType, pUserCallBack, pUserPtr);
	return IPC_Succeed;
}

/// @brief			�����ⲿ���ƻص��ӿ�
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [in]		pUserPtr		�û��Զ���ָ�룬�ڵ��ûص�ʱ�����ᴫ�ش�ָ��
IPCPLAYSDK_API int ipcplay_SetExternDrawCallBack(IN IPC_PLAYHANDLE hPlayHandle, IN void *pExternCallBack,IN void *pUserPtr)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	return pPlayer->SetCallBack(ExternDcDraw,pExternCallBack, pUserPtr);
}

/// @brief			���û�ȡYUV���ݻص��ӿ�,ͨ���˻ص����û���ֱ�ӻ�ȡ������YUV����
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [in]		pUserPtr		�û��Զ���ָ�룬�ڵ��ûص�ʱ�����ᴫ�ش�ָ��
IPCPLAYSDK_API int ipcplay_SetYUVCapture(IN IPC_PLAYHANDLE hPlayHandle, IN void *pCaptureYUV, IN void *pUserPtr)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	return pPlayer->SetCallBack(YUVCapture, pCaptureYUV, pUserPtr);
}

/// @brief			���û�ȡYUV���ݻص��ӿ�,ͨ���˻ص����û���ֱ�ӻ�ȡ������YUV����
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [in]		pUserPtr		�û��Զ���ָ�룬�ڵ��ûص�ʱ�����ᴫ�ش�ָ��
IPCPLAYSDK_API int ipcplay_SetYUVCaptureEx(IN IPC_PLAYHANDLE hPlayHandle, IN void *pCaptureYUVEx, IN void *pUserPtr)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	return pPlayer->SetCallBack(YUVCaptureEx,pCaptureYUVEx, pUserPtr);
}

/// @brief			����IPC˽�ø�ʽ¼��֡�����ص�,ͨ���˻أ��û���ֱ�ӻ�ȡԭʼ��֡����
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [in]		pUserPtr		�û��Զ���ָ�룬�ڵ��ûص�ʱ�����ᴫ�ش�ָ��
IPCPLAYSDK_API int ipcplay_SetFrameParserCallback(IN IPC_PLAYHANDLE hPlayHandle, IN void *pCaptureFrame, IN void *pUserPtr)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;

	return	pPlayer->SetCallBack(FramePaser,pCaptureFrame, pUserPtr);
}

/// @brief			����һ��IPC¼���ļ�ͷ
/// @param [in,out]	pMediaHeader	���û��ṩ�����Խ���IPC¼���ļ�ͷ�Ļ�����
/// @param [in,out]	pHeaderSize		ָ���û��ṩ���û������ĳ��ȣ��������ɹ����򷵻������ɵ�IPC¼���ļ�ͷ����
/// @param [in]		nAudioCodec		��Ƶ�ı�������
/// @param [in]		nVideoCodec		��Ƶ�ı�������
/// @param [in]		nFPS			��Ƶ��֡��
/// @remark		    ��pMediaHeaderΪNULL,��pHeaderSizeֻ�������軺�����ĳ���
IPCPLAYSDK_API int ipcplay_BuildMediaHeader(INOUT byte *pMediaHeader, INOUT int  *pHeaderSize,IN IPC_CODEC nAudioCodec,IN IPC_CODEC nVideoCodec,USHORT nFPS)
{
	if (!pHeaderSize)
		return IPC_Error_InvalidParameters;
	if (!pMediaHeader)
	{
		*pHeaderSize = sizeof(IPC_MEDIAINFO);
		return IPC_Succeed;
	}
	else if (*pHeaderSize < sizeof(IPC_MEDIAINFO))
		return IPC_Error_BufferSizeNotEnough;
	
	// ��Ƶ����Ƶ��������ͱ����޶������·�Χ��
	if (nAudioCodec < CODEC_G711A ||
		nAudioCodec > CODEC_AAC||
		nVideoCodec < CODEC_H264||
		nVideoCodec > CODEC_H265)
		return IPC_Error_InvalidParameters;

	IPC_MEDIAINFO *pMedia = new IPC_MEDIAINFO();
	pMedia->nCameraType	 = 0;	// 0Ϊ��άŷ����������ļ�
	pMedia->nFps			 = (UCHAR)nFPS;	
	pMedia->nVideoCodec  = nVideoCodec;
	pMedia->nAudioCodec  = nAudioCodec;	
	memcpy(pMediaHeader, pMedia, sizeof(IPC_MEDIAINFO));
	*pHeaderSize = sizeof(IPC_MEDIAINFO);
	delete pMedia;
	return IPC_Succeed;
}

/// @brief			����һ��IPC¼��֡
/// @param [in,out]	pMediaFrame		���û��ṩ�����Խ���IPC¼��֡�Ļ�����
/// @param [in,out]	pFrameSize		ָ���û��ṩ���û������ĳ��ȣ��������ɹ����򷵻������ɵ�IPC¼��֡����
/// @param [in,out]	nFrameID		IPC¼��֡��ID����һ֡����Ϊ0������֡���ε�������Ƶ֡����Ƶ֡����ֿ�����
/// @param [in]		pIPCIpcStream	��IPC IPC�õ�����������
/// @param [in,out]	nStreamLength	����ʱΪ��IPC IPC�õ����������ݳ��ȣ����ʱΪ��������ȥͷ��ĳ���,���������ĳ���
/// @remark		    ��pMediaFrameΪNULL,��pFrameSizeֻ����IPC¼��֡����
IPCPLAYSDK_API int ipcplay_BuildFrameHeader(OUT byte *pFrameHeader, INOUT int *HeaderSize, IN int nFrameID, IN byte *pIPCIpcStream, IN int &nStreamLength)
{
	if (!pIPCIpcStream || !nStreamLength)
		return sizeof(IPCFrameHeaderEx);
	if (!pFrameHeader)
	{
		*HeaderSize = sizeof(IPCFrameHeaderEx);
		return IPC_Succeed;
	}
	else if (*HeaderSize < sizeof(IPCFrameHeaderEx))
		return IPC_Error_BufferSizeNotEnough;

	IPC_StreamHeader *pStreamHeader = (IPC_StreamHeader *)pIPCIpcStream;
	int nSizeofHeader = sizeof(IPC_StreamHeader);
	byte *pFrameData = (byte *)(pStreamHeader)+nSizeofHeader;
	int nFrameLength = nStreamLength - sizeof(IPC_StreamHeader);
	*HeaderSize = sizeof(IPCFrameHeaderEx);
	
	IPCFrameHeaderEx FrameHeader;	
	switch (pStreamHeader->frame_type)
	{
	case 0:									// ��˼I֡��Ϊ0�����ǹ̼���һ��BUG���д�����
	case IPC_IDR_FRAME: 	// IDR֡
	case IPC_I_FRAME:		// I֡
		FrameHeader.nType = FRAME_I;
		break;
	case IPC_P_FRAME:       // P֡
	case IPC_B_FRAME:       // B֡
	case IPC_711_ALAW:      // 711 A�ɱ���֡
	case IPC_711_ULAW:      // 711 U�ɱ���֡
	case IPC_726:           // 726����֡
	case IPC_AAC:           // AAC����֡
		FrameHeader.nType = pStreamHeader->frame_type;
		break;
	default:
	{
		assert(false);
		break;
	}
	}
	FrameHeader.nTimestamp	 = pStreamHeader->sec * 1000 * 1000 + pStreamHeader->usec;
	FrameHeader.nLength		 = nFrameLength;
	FrameHeader.nFrameTag	 = IPC_TAG;		///< IPC_TAG
	FrameHeader.nFrameUTCTime= time(NULL);
	FrameHeader.nFrameID	 = nFrameID;
	memcpy(pFrameHeader, &FrameHeader, sizeof(IPCFrameHeaderEx));
	nStreamLength			 = nFrameLength;
	
	return IPC_Succeed;
}

/// @brief			����̽����������ʱ���ȴ������ĳ�ʱֵ
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [in]		dwTimeout		�ȴ������ĸ�ʱֵ����λ����
/// @remark			�ú�������Ҫ��ipcplay_Start֮ǰ���ã�������Ч
IPCPLAYSDK_API int ipcplay_SetProbeStreamTimeout(IN IPC_PLAYHANDLE hPlayHandle, IN DWORD dwTimeout /*= 3000*/)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	pPlayer->SetProbeStreamTimeout(dwTimeout);
	return IPC_Succeed;
}

/// @brief			����ͼ������ظ�ʽ
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [in]		nPixelFMT		Ҫ���õ����ظ�ʽ�������@see PIXELFMORMAT
/// @param [in]		pUserPtr		�û��Զ���ָ�룬�ڵ��ûص�ʱ�����ᴫ�ش�ָ��
/// @remark			��Ҫ�����ⲿ��ʾ�ص����������ʾ��ʽ����ΪR8G8B8��ʽ
IPCPLAYSDK_API int ipcplay_SetPixFormat(IN IPC_PLAYHANDLE hPlayHandle, IN PIXELFMORMAT nPixelFMT)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	return pPlayer->SetPixelFormat(nPixelFMT);
}

IPCPLAYSDK_API int ipcplay_SetD3dShared(IN IPC_PLAYHANDLE hPlayHandle, IN bool bD3dShared )
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	pPlayer->SetD3dShared(bD3dShared);
	return IPC_Succeed;
}
IPCPLAYSDK_API void ipcplay_ClearD3DCache()
{
	
}

IPCPLAYSDK_API void *AllocAvFrame()
{
	return CIPCPlayer::_AllocAvFrame();
}

IPCPLAYSDK_API void AvFree(void*p)
{
	CIPCPlayer::_AvFree(p);
}

// IPCPLAYSDK_API int ipcplay_SuspendDecode(IN IPC_PLAYHANDLE hPlayHandle)
// {
// 	if (!hPlayHandle)
// 		return IPC_Error_InvalidParameters;
// 	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
// 	if (pPlayer->nSize != sizeof(CIPCPlayer))
// 		return IPC_Error_InvalidParameters;
// 	pPlayer->SuspendDecode();
// 	return IPC_Succeed;
// }
// 
// IPCPLAYSDK_API int ipcplay_ResumeDecode(IN IPC_PLAYHANDLE hPlayHandle)
// {
// 	if (!hPlayHandle)
// 		return IPC_Error_InvalidParameters;
// 	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
// 	if (pPlayer->nSize != sizeof(CIPCPlayer))
// 		return IPC_Error_InvalidParameters;
// 	pPlayer->ResumeDecode();
// 	return IPC_Succeed;
// }

// IPCPLAYSDK_API int AddRenderWnd(IN IPC_PLAYHANDLE hPlayHandle, IN HWND hRenderWnd)
// {
// 	if (!hPlayHandle)
// 		return IPC_Error_InvalidParameters;
// 	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
// 	if (pPlayer->nSize != sizeof(CIPCPlayer))
// 		return IPC_Error_InvalidParameters;
// 	return pPlayer->AddRenderWnd(hRenderWnd,nullptr);
// }

// IPCPLAYSDK_API int RemoveRenderWnd(IN IPC_PLAYHANDLE hPlayHandle, IN HWND hRenderWnd)
// {
// 	if (!hPlayHandle)
// 		return IPC_Error_InvalidParameters;
// 	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
// 	if (pPlayer->nSize != sizeof(CIPCPlayer))
// 		return IPC_Error_InvalidParameters;
// 	return pPlayer->RemoveRenderWnd(hRenderWnd);
// }


/// @brief			����ͼ����ת�Ƕ�
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [in]		nAngle			Ҫ��ת�ĽǶ�ֵ���������@see RocateAngle
/// @remark	ע��    Ŀǰͼ����ת���ܽ�֧�����
IPCPLAYSDK_API int ipcplay_SetRocateAngle(IN IPC_PLAYHANDLE hPlayHandle, RocateAngle nAngle )
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	return pPlayer->SetRocate(nAngle);
}

/// @brief		��YUVͼ��ת��ΪRGB24ͼ��
/// @param [in]		hPlayHandle	��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [in]		pY			YUV����Y����ָ��
/// @param [in]		pU			YUV����U����ָ��
/// @param [in]		pV			YUV����V����ָ��
/// @param [in]		nStrideY	YUV����Y�����ĸ�������һ�����ݵĳ���
/// @param [in]		nStrideUV	YUV����UV�����ĸ�������һ�����ݵĳ���
/// @param [in]		nWidth		YUVͼ��Ŀ��
/// @param [in]		nHeight		YUVͼ��ĸ߶�
/// @param [out]    pRGBBuffer	RGB24ͼ��Ļ���
/// @param [out]	nBufferSize	RGB24ͼ��Ļ���ĳ���
// IPCPLAYSDK_API int ipcplay_YUV2RGB24(IN IPC_PLAYHANDLE hPlayHandle,
// 	const unsigned char* pY,
//	const unsigned char* pU,
// 	const unsigned char* pV,
/*	int nStrideY,
	int nStrideUV,
	int nWidth,
	int nHeight,
	byte **ppRGBBuffer,
	long &nBufferSize)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	return pPlayer->YUV2RGB24(pY, pU, pV, nStrideY, nStrideUV, nWidth, nHeight, ppRGBBuffer, nBufferSize);
}
*/

/// @brief			����һ����������
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [in]		pPointArray		������������
/// @param [in]		nCount			pPointArray�а�����������������
/// @param [in]		fWidth			�������
/// @param [in]		nColor			��������ɫ
/// @return �����ɹ�ʱ������������ľ�������򷵻�0
/// @remark	ע��    ���ú����������,SDK�ڲ������������Ϣ����������һ���������������ɫ����ͬ�ģ������������ģ���Ҫ���ƶ����������������������ε���ipcplay_AddLineArray
IPCPLAYSDK_API long ipcplay_AddLineArray(IN IPC_PLAYHANDLE hPlayHandle, POINT *pPointArray, int nCount, float fWidth, DWORD nColor)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	return pPlayer->AddLineArray(pPointArray, nCount, fWidth, nColor);
}

/// @brief			�Ƴ�һ������
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [in]		nLineIndex		��ipcplay_AddLineArray���ص��������
/// @return �����ɹ�ʱ����SDK�ڴ����ڻ�������������������򷵻�-1
IPCPLAYSDK_API int  ipcplay_RemoveLineArray(IN IPC_PLAYHANDLE hPlayHandle, long nLineIndex)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	pPlayer->RemoveLineArray(nLineIndex);
	return IPC_Succeed;
}


/// @brief			���ñ���ͼƬ·��
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [in]		szImageFile		����ͼƬ·��������ͼƬ����jpg,png��bmp�ļ�,Ϊnullʱ����ɾ������ͼƬ
IPCPLAYSDK_API int ipcplay_SetBackgroundImageA(IN IPC_PLAYHANDLE hPlayHandle, LPCSTR szImageFile)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	WCHAR szImageFileW[1024] = { 0 };
	A2WHelper(szImageFile, szImageFileW, 1024);
	pPlayer->SetBackgroundImage(szImageFileW);
	return IPC_Succeed;
}

/// @brief			���ñ���ͼƬ·��
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [in]		szImageFile		����ͼƬ·��������ͼƬ����jpg,png��bmp�ļ���Ϊnullʱ����ɾ������ͼƬ
IPCPLAYSDK_API int ipcplay_SetBackgroundImageW(IN IPC_PLAYHANDLE hPlayHandle, LPCWSTR szImageFile)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	pPlayer->SetBackgroundImage(szImageFile);
	return IPC_Succeed;
}

/// @brief			����DirectDraw��Ϊ��Ⱦ��,�⽫����D3D��Ⱦ,Ӳ����ʱ�޷�����D3D����ģʽ���⽻�󸱽���Ӳ�����Ч��
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [in]		szImageFile		����ͼƬ·��������ͼƬ����jpg,png��bmp�ļ�,Ϊnullʱ����ɾ������ͼƬ
IPCPLAYSDK_API int ipcplay_EnableDDraw(IN IPC_PLAYHANDLE hPlayHandle, bool bEnable)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	return pPlayer->EnableDDraw(bEnable);
}

/// @brief			������������
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [in]		nCodec			�����ʽ
IPCPLAYSDK_API int ipcplay_EnableStreamParser(IN IPC_PLAYHANDLE hPlayHandle, IPC_CODEC nCodec )
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	return pPlayer->EnableStreamParser(nCodec);
}

/// @brief			����δ������δ��֡����
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [in]		pData			�����������
/// @param [in]		nLength			�������ߴ�
IPCPLAYSDK_API int ipcplay_InputStream2(IN IPC_PLAYHANDLE hPlayHandle, byte *pData, int nLength)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	return pPlayer->InputStream(pData, nLength);
}

/// @brief			�������Ƶ֡
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [in]		pData			����Ƶ֡
IPCPLAYSDK_API int ipcplay_InputDHStream(IN IPC_PLAYHANDLE hPlayHandle, byte *pBuffer,int nLength)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	return pPlayer->InputDHStream(pBuffer,nLength);
}
/// @brief			����һ��ʵ�Ķ����
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [in]		pPointArray		����ζ�����������
/// @param [in]		nCount			pPointArray�а�����������������
/// @param [in]		pVertexIndex	pPointArray�����������˳�򣬼����ƶ���λ����ƶ���˳��
/// @param [in]		nColor			����ε���ɫ
/// @return �����ɹ�ʱ������������ľ�������򷵻�0
/// @remark	ע��    ���ú����������,SDK�ڲ������������Ϣ����������һ���������������ɫ����ͬ�ģ������������ģ���Ҫ���ƶ����������������������ε���ipcplay_AddLineArray
IPCPLAYSDK_API long ipcplay_AddPolygon(IN IPC_PLAYHANDLE hPlayHandle, POINT *pPointArray, int nCount, WORD *pVertexIndex, DWORD nColor)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	return pPlayer->AddPolygon(pPointArray, nCount, pVertexIndex, nColor);
}

/// @brief			�Ƴ�һ�������
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [in]		nLineIndex		��ipcplay_AddPolygon���صĶ���ξ��
IPCPLAYSDK_API int ipcplay_RemovePolygon(IN IPC_PLAYHANDLE hPlayHandle, long nLineIndex)
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	pPlayer->RemovePolygon(nLineIndex);
	return IPC_Succeed;
}

IPCPLAYSDK_API int ipcplay_SetCoordinateMode(IN IPC_PLAYHANDLE hPlayHandle, int nCoordinateMode )
{
	if (!hPlayHandle)
		return IPC_Error_InvalidParameters;
	CIPCPlayer *pPlayer = (CIPCPlayer *)hPlayHandle;
	if (pPlayer->nSize != sizeof(CIPCPlayer))
		return IPC_Error_InvalidParameters;
	pPlayer->SetCoordinateMode(nCoordinateMode);
	return IPC_Succeed;
}