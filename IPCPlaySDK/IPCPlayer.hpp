#pragma once
/////////////////////////////////////////////////////////////////////////
/// @file  IPCPlayer.hpp
/// Copyright (c) 2016, xionggao.lee
/// All rights reserved.  
/// @brief IPCIPC�������SDKʵ��
/// @version 1.0  
/// @author  xionggao.lee
/// @date  2015.12.17
///   
/// �޶�˵��������汾 
/////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
//#include <vld.h>
#include "Common.h"
#include "Media.h"
#include "IPCPlaySDK.h"
#include "./DxSurface/DxSurface.h"
#include "AutoLock.h"
#include "./VideoDecoder/VideoDecoder.h"
#include "StreamParser.hpp"
#include "TimeUtility.h"
#include "Utility.h"
#include "DSoundPlayer.hpp"
#include "AudioDecoder.h"
#include "DirectDraw.h"
#include "TimeTrace.h"
#include "StreamFrame.h"
#include "Snapshot.h"
#include "YUVFrame.h"
#include "RenderUnit.h"
#include "SimpleWnd.h"
#include "MMEvent.h"
#include "Stat.h"
#include "./DxSurface/gpu_memcpy_sse4.h"
//#include "DHStream.hpp"
#include "DhStreamParser.h"
#define  Win7MajorVersion	6
class CIPCPlayer;
/// @brief �ļ�֡��Ϣ�����ڱ�ʶÿһ֡���ļ���λ��
struct FileFrameInfo
{
	UINT	nOffset;	///< ֡�������ڵ��ļ�ƫ��
	UINT	nFrameSize;	///< ���� IPCFrameHeaderEx���ڵ�֡���ݵĳߴ�
	bool	bIFrame;	///< �Ƿ�I֡
	time_t	tTimeStamp;
};

 
// �����첽��Ⱦ��֡����
struct CAvFrame
{
private:
	CAvFrame()
	{
	}
public:
	time_t	tFrame;
	AVFrame *pFrame;
	int		nImageSize;
	byte	*pImageBuffer;
	CAvFrame(AVFrame *pSrcFrame,time_t tFrame)
	{
		ZeroMemory(this, sizeof(CAvFrame));
		pFrame = av_frame_alloc();
		char szAvError[256] = { 0 };
		int nImageSize = av_image_get_buffer_size((AVPixelFormat)pSrcFrame->format, pSrcFrame->width, pSrcFrame->height, 16);
		if (nImageSize < 0)
		{
			av_strerror(nImageSize, szAvError, 1024);
			DxTraceMsg("%s av_image_get_buffer_size failed:%s.\n", __FUNCTION__, szAvError);
			assert(false);
		}
		pImageBuffer = (byte *)_aligned_malloc(nImageSize, 16);
		if (!pImageBuffer)
		{
			DxTraceMsg("%s Alloc memory failed @%s %d.\n", __FUNCTION__, __FUNCTION__, __LINE__);
			assert(false);
			return;
		}
		// ����ʾͼ����YUV֡����
		av_image_fill_arrays(pFrame->data, pFrame->linesize, pImageBuffer, (AVPixelFormat)pSrcFrame->format, pSrcFrame->width, pSrcFrame->height, 16);
		pFrame->width = pSrcFrame->width;
		pFrame->height = pSrcFrame->height;
		pFrame->format = pSrcFrame->format;
		int nAvError = av_frame_copy(pFrame, pSrcFrame);
		if (nAvError < 0)
		{
			av_strerror(nAvError, szAvError, 1024);
			DxTraceMsg("%s av_image_get_buffer_size failed:%s.\n", __FUNCTION__, szAvError);
			assert(false);
		}
		this->tFrame = tFrame;
	}
	~CAvFrame()
	{
		if (pFrame)
		{
			av_frame_unref(pFrame);
			av_free(pFrame);
			pFrame = nullptr;
		}
		if (pImageBuffer)
		{
			_aligned_free(pImageBuffer);
			pImageBuffer = nullptr;
		}
	}
};
typedef shared_ptr<CAvFrame> CAVFramePtr;


// //���Կ���
// struct _DebugPin
// {
// 	bool bSwitch;
// 	double dfLastTime;
// 	_DebugPin()
// 	{
// 		bSwitch = true;
// 		dfLastTime = GetExactTime();
// 	}
// 
// };
// 
// struct _DebugSwitch
// {
// 	_DebugPin InputIpcStream;	
// 	_DebugPin DecodeSucceed;
// 	_DebugPin DecodeFailure;
// 	_DebugPin RenderSucceed;
// 	_DebugPin RenderFailure;
// };
// #ifdef _DEBUG
// #define SwitchTrace(pDebugSwitch,member) { if (pDebugSwitch && pDebugSwitch->##member##.bSwitch) if (TimeSpanEx(pDebugSwitch->##member##.dfLastTime) >= 5.0f) {	DxTraceMsg("%s %d %s.\n", __FUNCTION__, __LINE__,#member);	pDebugSwitch->##member##.dfLastTime = GetExactTime();}}
// #endif 
struct _OutputTime
{
	DWORD nDecode;
	DWORD nInputStream;
	DWORD nRender;
	DWORD nQueueSize;
	_OutputTime()
	{
		// �Զ���ʼ������DWORD�ͱ�������ʹ�������ӱ���Ҳ���������Ķ�
		DWORD *pThis = (DWORD *)this;
		for (int i = 0; i < sizeof(_OutputTime) / 4; i++)
			pThis[i] = timeGetTime();
		nInputStream = 0;
	}
};

/// @brief ��������̽�����
struct StreamProbe
{
	StreamProbe(int nBuffSize = 256 * 1024)
	{
		ZeroMemory(this, sizeof(StreamProbe));
		pProbeBuff = (byte *)_aligned_malloc(nBuffSize, 16);
		nProbeBuffSize = nBuffSize;
	}
	~StreamProbe()
	{
		_aligned_free(pProbeBuff);
	}
	bool GetProbeStream(byte *pStream, int nStreamLength)
	{
		if (!pStream || !nStreamLength)
			return false;
		if ((nProbeDataLength + nStreamLength) > nProbeBuffSize)
			return false;
		memcpy_s(&pProbeBuff[nProbeDataLength], nProbeBuffSize - nProbeDataLength, pStream, nStreamLength);
		nProbeDataLength += nStreamLength;
		nProbeDataRemained = nProbeDataLength;
		nProbeOffset = 0;
		return true;
	}

	byte		*pProbeBuff;			///< �����ؼ�̽�⻺����
	int			nProbeBuffSize;			///< ̽�⻺��ĳߴ�
	int			nProbeDataLength;		///< ̽�⻺�������ӵĳ���
	int			nProbeDataRemained;
	AVCodecID	nProbeAvCodecID;		///< ̽�⵽������ID
	int			nProbeWidth;
	int			nProbeHeight;
	int			nProbeCount;			///< ̽�����
	int			nProbeOffset;			///< ����̽��ʱ��Ƶ֡����ʱ��֡��ƫ��
};

extern volatile bool g_bThread_ClosePlayer/* = false*/;
extern list<IPC_PLAYHANDLE > g_listPlayerAsyncClose;
extern CCriticalSectionProxy  g_csListPlayertoFree;
extern double	g_dfProcessLoadTime ;
extern HMODULE  g_hDllModule;
//extern HWND g_hSnapShotWnd;

/// IPCIPCPlay SDK��Ҫ����ʵ����
struct HAccelRec
{
	HAccelRec()
	{
		ZeroMemory(this, sizeof(HAccelRec));
	}
	int		nMaxCount;		// �������·��
	int		nOpenedCount;	// �Ѿ�����·��
};

typedef shared_ptr<HAccelRec> HAccelRecPtr;
class CIPCPlayer
{
public:
	int		nSize;
private:
	list<FrameNV12Ptr>		m_listNV12;		// YUV���棬Ӳ����
	list<FrameYV12Ptr>		m_listYV12;		// YUV���棬�����
	list<StreamFramePtr>	m_listAudioCache;///< ��Ƶ������֡����
	list<StreamFramePtr>	m_listVideoCache;///< ��Ƶ������֡����
	//map<HWND, CDirectDrawPtr> m_MapDDraw;
	list <RenderUnitPtr>	m_listRenderUnit;
	list <RenderWndPtr>		m_listRenderWnd;	///< �ര����ʾͬһ��Ƶͼ��
	list<CAVFramePtr>		m_listAVFrame;		///<��Ƶ֡���棬�����첽��ʾͼ��
public:
	static map<string, HAccelRecPtr>m_MapHacceConfig;	///��ͬ�Կ��Ͽ���Ӳ���·��
	static CCriticalSectionProxy m_csMapHacceConfig;
private:
		
	CCriticalSectionProxy	m_cslistRenderWnd;
	CCriticalSectionProxy	m_csAudioCache;		
	CCriticalSectionProxy	m_csBorderRect;
	CCriticalSectionProxy	m_csListRenderUnit;
	CCriticalSectionProxy	m_csParser;
	CCriticalSectionProxy	m_csSeekOffset;
	CCriticalSectionProxy	m_csCaptureYUV;
	CCriticalSectionProxy	m_csFilePlayCallBack;
	CCriticalSectionProxy	m_csCaptureYUVEx;
	CCriticalSectionProxy	m_csYUVFilter;
	CCriticalSectionProxy	m_csVideoCache;
	CCriticalSectionProxy	m_csCaptureRGB;
	CCriticalSectionProxy	m_cslistAVFrame;	// �첽��Ⱦ֡������
	CCriticalSectionProxy	m_csTimebase;
	//////////////////////////////////////////////////////////////////////////
	/// ע�⣺����CCriticalSectionProxy��Ķ����ģ����Ķ��󶼱��붨����m_nZeroOffset
	/// ��Ա����֮ǰ��������ܻ�����ʴ���
	//////////////////////////////////////////////////////////////////////////
	int			m_nZeroOffset;
	bool		m_bEnableDDraw = false;			// �Ƿ�����DDRAW������DDRAW������D3D�������޷�����Ӳ������ģʽ�����½���Ч���½�
	UINT					m_nListAvFrameMaxSize;	///<��Ƶ֡�����������
	/************************************************************************/
	//   ********ע�� ********                                                               
	//����ƶ�m_nZeroOffset���������λ�ã���Ϊ�ڹ��캯����,Ϊʵ�ֿ��ٳ�ʼ������Ҫʹ��      
	//ZeroMemory��������Ա��ʼ������Ҫ�Դ�Ϊ�߽�ܿ�����STL���������,�����������������
	//�붨���ڴ˱���֮ǰ 
	//***********************************************************************/								
	
	//CCriticalSectionProxy	m_csListYUV;
	int					m_nMaxYUVCache;	// ��������YUV��������
	int					m_nMaxFrameCache;///< �����Ƶ��������,Ĭ��ֵ100
		///< ��m_FrameCache�е���Ƶ֡��������m_nMaxFrameCacheʱ�����޷��ټ�����������
public:
	static CCriticalSectionProxyPtr m_pCSGlobalCount;
	shared_ptr<CMMEvent> m_pRenderTimer;
	CHAR		m_szLogFileName[512];
//#ifdef _DEBUG
	
	int			m_nObjIndex;			///< ��ǰ����ļ���
	static int	m_nGloabalCount;		///< ��ǰ������CIPCPlayer��������
private:
	DWORD		m_nLifeTime;			///< ��ǰ�������ʱ��
	_OutputTime	m_OuputTime;
//#endif
private:
	// ��Ƶ������ر���
	int			m_nTotalFrames;			///< ��ǰ�ļ�����Ч��Ƶ֡������,���������ļ�ʱ��Ч
	int			m_nTotalTime;			///< ��ǰ�ļ��в�����ʱ��,���������ļ�ʱ��Ч
	shared_ptr<StreamProbe>m_pStreamProbe;
	/// ������Ҫ�����ڣ����е���Ⱦ���ڶ�����Ⱦ���ڱ���
	HWND		m_hRenderWnd;			///< ������Ƶ�Ĵ��ھ��
	
	volatile bool m_bIpcStream;			///< ������ΪIPC��
	volatile DWORD m_nProbeStreamTimeout;///< ̽��������ʱ�䣬��λ����
	D3DFORMAT	m_nPixelFormat;
	IPC_CODEC	m_nVideoCodec;			///< ��Ƶ�����ʽ @see IPC_CODEC	
	static		CAvRegister avRegister;	
	static		CTimerQueue m_TimeQueue;///< ���ж�ʱ��
	//static shared_ptr<CDSound> m_pDsPlayer;///< DirectSound���Ŷ���ָ��
	shared_ptr<CDSound> m_pDsPlayer;///< DirectSound���Ŷ���ָ��
	//shared_ptr<CDSound> m_pDsPlayer;	///< DirectSound���Ŷ���ָ��
	CDSoundBuffer* m_pDsBuffer;
	Coordinte		m_nCoordinate = Coordinte_Wnd;
	DxSurfaceInitInfo	m_DxInitInfo;
	CDxSurfaceEx* m_pDxSurface;			///< Direct3d Surface��װ��,������ʾ��Ƶ
	
	void *m_pDCCallBack = nullptr;
	void *m_pDCCallBackParam = nullptr;
  	CDirectDraw *m_pDDraw;				///< DirectDraw��װ�����������xp����ʾ��Ƶ
	WCHAR		*m_pszBackImagePath;
	bool		m_bEnableBackImage = false;
  	shared_ptr<ImageSpace> m_pYUVImage = NULL;
// 	bool		m_bDxReset;				///< �Ƿ�����DxSurface
// 	HWND		m_hDxReset;
	shared_ptr<CVideoDecoder>	m_pDecoder;
	
	bool		m_bRefreshWnd;			///< ֹͣ����ʱ�Ƿ�ˢ�»���
	int			m_nVideoWidth;			///< ��Ƶ���
	int			m_nVideoHeight;			///< ��Ƶ�߶�	
	int			m_nDecodeDelay;
	AVPixelFormat	m_nDecodePixelFmt;	///< ���������ظ�ʽ
	int			m_nFrameEplased;		///< �Ѿ�����֡��
	int			m_nCurVideoFrame;		///< ��ǰ�����ŵ���Ƶ֡ID
	time_t		m_nFirstFrameTime;		///< �ļ����Ż����طŵĵ�1֡��ʱ��
	time_t		m_tCurFrameTimeStamp;
	time_t		m_tLastFrameTime;
	USHORT		m_nPlayFPS;				///< ʵ�ʲ���ʱ֡��
	USHORT		m_nPlayFrameInterval;	///< ����ʱ֡���	
	HANDLE		m_hEventDecodeStart;	///< ��Ƶ�����Ѿ���ʼ�¼�
	int			m_nSkipFrames;			///< ��֡���е�Ԫ������
	bool		m_bAsnycClose;		///< �Ƿ�����D3D����
	double		m_dfLastTimeVideoPlay;	///< ǰһ����Ƶ���ŵ�ʱ��
	double		m_dfLastTimer;
	double		m_dfTimesStart;			///< ��ʼ���ŵ�ʱ��
	bool		m_bEnableHaccel;		///< �Ƿ�����Ӳ����
	bool		m_bFitWindow;			///< ��Ƶ��ʾ�Ƿ���������
										///< Ϊtrueʱ�������Ƶ��������,�������ͼ������,���ܻ����ͼ�����
										///< Ϊfalseʱ����ֻ��ͼ��ԭʼ�����ڴ�������ʾ,������������,���Ժ�ɫ������ʾ

	shared_ptr<RECT>m_pBorderRect;		///< ��Ƶ��ʾ�ı߽磬�߽����ͼ������ʾ
	bool		m_bBorderInPencent;	///< m_pBorderRect�еĸ������Ƿ�Ϊ�ٷֱ�,����left=20,��ʹ��ͼ��20%�Ŀ�ȣ���top=10,��ʹ��ͼ��10%�ĸ߶�
	bool		m_bPlayerInialized;		///< �������Ƿ��Ѿ���ɳ�ʼ��
	shared_ptr<PixelConvert> m_pConvert;
public:		// ʵʱ�����Ų���
	static shared_ptr<CSimpleWnd>m_pWndDxInit;///< ��Ƶ��ʾʱ�����Գ�ʼ��DirectX�����ش��ڶ���
	bool		m_bProbeStream ;
	int			m_nProbeOffset ;
	volatile bool m_bAsyncRender ;
	HANDLE		m_hRenderAsyncEvent;	///< �첽��Ⱦ�¼�
	
	time_t		m_tSyncTimeBase;		///< ͬ��ʱ����
	CIPCPlayer *m_pSyncPlayer;			///< ͬ�����Ŷ���
	int			m_nVideoFPS;			///< ��Ƶ֡��ԭʼ֡��
	int			m_nDisplayAdapter = 0;
	
private:	// ��Ƶ������ر���

	IPC_CODEC	m_nAudioCodec;			///< ��Ƶ�����ʽ @see IPC_CODEC
	bool		m_bEnableAudio;			///< �Ƿ�������Ƶ����
	DWORD		m_dwAudioOffset;		///< ��Ƶ�������е�ƫ�Ƶ�ַ
	HANDLE		m_hAudioFrameEvent[2];	///< ��Ƶ֡�Ѳ����¼�
	DWORD		m_dwAudioBuffLength;	///< 
	int			m_nNotifyNum;			///< ��Ƶ����������
	double		m_dfLastTimeAudioPlay;	///< ǰһ�β�����Ƶ��ʱ��
	double		m_dfLastTimeAudioSample;///< ǰһ����Ƶ������ʱ��
	int			m_nAudioFrames;			///< ��ǰ��������Ƶ֡����
	int			m_nCurAudioFrame;		///< ��ǰ�����ŵ���Ƶ֡ID
	DWORD		m_nSampleFreq;			///< ��Ƶ����Ƶ��
	WORD		m_nSampleBit;			///< ����λ��
	WORD		m_nAudioPlayFPS;		///< ��Ƶ����֡��

	static shared_ptr<CDSoundEnum> m_pDsoundEnum;	///< ��Ƶ�豸ö����
	static CCriticalSectionProxyPtr m_csDsoundEnum;
private:
	HANDLE		m_hThreadFileParser;	///< ����IPC˽�и�ʽ¼����߳�
	HANDLE		m_hThreadDecode;		///< ��Ƶ����Ͳ����߳�
	HANDLE		m_hThreadPlayAudio;		///< ��Ƶ����Ͳ����߳�
	HANDLE		m_hThreadAsyncReander;	///< �첽��ʾ�߳�
	HANDLE		m_hThreadStreamParser;	///< �������߳�;
	HANDLE		m_hThreadReversePlay;	///< ���򲥷��߳�
	
	//HANDLE		m_hThreadGetFileSummary;///< �ļ���ϢժҪ�߳�
	UINT		m_nVideoCache;
	UINT		m_nAudioCache;
	//HANDLE		m_hCacheFulled;			///< �����������̱߳��������¼�
	UINT		m_nHeaderFrameID;		///< �����е�1֡��ID
	UINT		m_nTailFrameID;			///< ���������һ֡��ID
	//bool		m_bThreadSummaryRun;
	bool		m_bSummaryIsReady;		///< �ļ�ժҪ��Ϣ׼�����
	bool		m_bStopFlag;			///< ������ֹͣ��־�����ٽ�������
	volatile bool m_bThreadParserRun;
	volatile bool m_bThreadDecodeRun;
	volatile bool m_bThreadReversePlayRun; ///<  �������򲥷�
	volatile bool m_bThreadPlayAudioRun;
	volatile bool m_bStreamParserRun = false;
	
	shared_ptr<CStreamParser> m_pStreamParser = nullptr;
	//shared_ptr<CDHStreamParser> m_pDHStreamParser = nullptr;
	shared_ptr<DhStreamParser> m_pDHStreamParser = nullptr;
#ifdef _DEBUG
	double		m_dfFirstFrameTime;
#endif
	byte*		m_pParserBuffer;		///< ���ݽ���������
	UINT		m_nParserBufferSize;	///< ���ݽ����������ߴ�
	DWORD		m_nParserDataLength;	///< ���ݽ����������е���Ч���ݳ���
	UINT		m_nParserOffset;		///< ���ݽ����������ߴ統ǰ�Ѿ�������ƫ��
			
	//volatile bool m_bThreadFileAbstractRun;
	bool		m_bPause;				///< �Ƿ�����ͣ״̬
	byte		*m_pYUV;				///< YVU��׽ר���ڴ�
	int			m_nYUVSize ;			///<
	shared_ptr<byte>m_pYUVPtr;
	// ��ͼ������ؾ����
	HANDLE		m_hEvnetYUVReady;		///< YUV���ݾ����¼�
	HANDLE		m_hEventYUVRequire;		///< YUV���������¼�,�����ѵ�ǰ����֡���и���m_pAvFrameSnapshot
	HANDLE		m_hEventFrameCopied;	///< ��ͼ��������ɸ����¼�
	shared_ptr<CSnapshot>m_pSnapshot;
	
private:	// �ļ�������ر���
	
	HANDLE		m_hVideoFile;				///< ���ڲ��ŵ��ļ����
	INT64		m_nSummaryOffset;		///< �ڶ�ȡ����ʱ��õ��ļ�����ƫ��
#ifdef _DEBUG
	bool		m_bSeekSetDetected = false;///< �Ƿ��������֡����
#endif
	shared_ptr<IPC_MEDIAINFO>m_pMediaHeader;/// ý���ļ�ͷ
	long		m_nSDKVersion;
	IPCFrameHeader m_FirstFrame, m_LastFrame;
	UINT		m_nFrametoRead;			///< ��ǰ��Ҫ��ȡ����Ƶ֡ID
	char		*m_pszFileName;			///< ���ڲ��ŵ��ļ���,���ֶν����ļ�����ʱ����Ч
	FileFrameInfo	*m_pFrameOffsetTable;///< ��Ƶ֡ID��Ӧ�ļ�ƫ�Ʊ�
	volatile LONGLONG m_nSeekOffset;	///< ���ļ���ƫ��
	
	float		m_fPlayRate;			///< ��ǰ�Ĳ��ŵı���,����1ʱΪ���ٲ���,С��1ʱΪ���ٲ��ţ�����Ϊ0��С��0
	int			m_nMaxFrameSize;		///< ���I֡�Ĵ�С�����ֽ�Ϊ��λ,Ĭ��ֵ256K
	
	USHORT		m_nFileFrameInterval;	///< �ļ���,��Ƶ֡��ԭʼ֡���
	float		m_fPlayInterval;		///< ֡���ż��,��λ����
	bool		m_bFilePlayFinished;	///< �ļ�������ɱ�־, Ϊtrueʱ�����Ž�����Ϊfalseʱ����δ����
	bool		m_bPlayOneFrame;		///< ���õ�֡����
	DWORD		m_dwStartTime;
private:
// 	CaptureFrame	m_pfnCaptureFrame;
// 	void*			m_pUserCaptureFrame;

	CaptureYUV		m_pfnCaptureYUV;
	void*			m_pUserCaptureYUV;
	CaptureYUVEx	m_pfnCaptureYUVEx;
	void*			m_pUserCaptureYUVEx;
	FilePlayProc	m_pFilePlayCallBack;
	void*			m_pUserFilePlayer;
	CaptureYUVEx	m_pfnYUVFilter;
	void*			m_pUserYUVFilter;
	CaptureRGB		m_pCaptureRGB;
	void*			m_pUserCaptureRGB;
private:
	shared_ptr<CRunlog> m_pRunlog;	///< ������־
#define __countof(array) (sizeof(array)/sizeof(array[0]))
#pragma warning (disable:4996)
	void OutputMsg(char *pFormat, ...)
	{
		int nBuff;
		CHAR szBuffer[4096];
		va_list args;
		va_start(args, pFormat);		
		nBuff = _vsnprintf(szBuffer, __countof(szBuffer), pFormat, args);
		//::wvsprintf(szBuffer, pFormat, args);
		//assert(nBuff >=0);
//#ifdef _DEBUG
		OutputDebugStringA(szBuffer);
//#endif
		if (m_pRunlog)
			m_pRunlog->Runlog(szBuffer);
		va_end(args);
	}
public:
	
private:
	CIPCPlayer();

	LONGLONG GetSeekOffset()
	{
		CAutoLock lock(&m_csSeekOffset, false,__FILE__, __FUNCTION__, __LINE__);
		return m_nSeekOffset;
	}
	void SetSeekOffset(LONGLONG nSeekOffset)
	{
		CAutoLock lock(&m_csSeekOffset, false, __FILE__, __FUNCTION__, __LINE__);
		m_nSeekOffset = nSeekOffset;
	}
	// �����ļ�Ѱַ
	__int64 LargerFileSeek(HANDLE hFile, __int64 nOffset64, DWORD MoveMethod);

	/// @brief �ж�����֡�Ƿ�ΪIPC˽�е���Ƶ֡
	/// @param[in]	pFrameHeader	IPC˽��֡�ṹָ�����@see IPCFrameHeaderEx
	/// @param[out] bIFrame			�ж�����֡�Ƿ�ΪI֡
	/// -# true			����֡ΪI֡
	///	-# false		����֡Ϊ����֡
	static bool IsIPCVideoFrame(IPCFrameHeader *pFrameHeader, bool &bIFrame, int nSDKVersion);

	/// @brief ȡ����Ƶ�ļ���һ��I֡�����һ����Ƶ֡ʱ��
	/// @param[out]	֡ʱ��
	/// @param[in]	�Ƿ�ȡ��һ��I֡ʱ�䣬��Ϊtrue��ȡ��һ��I֡��ʱ�����ȡ��һ����Ƶ֡ʱ��
	/// @remark �ú���ֻ���ڴӾɰ��IPC¼���ļ���ȡ�õ�һ֡�����һ֡
	int GetFrame(IPCFrameHeader *pFrame, bool bFirstFrame = true);
	
	/// @brief ȡ����Ƶ�ļ�֡��ID��ʱ��,�൱����Ƶ�ļ��а�������Ƶ��֡��
	int GetLastFrameID(int &nLastFrameID);

	//shared_ptr<CSimpleWnd> m_pSimpleWnd /*= make_shared<CSimpleWnd>()*/;

	// ��ʼ��DirectX��ʾ���
	bool InitizlizeDx(AVFrame *pAvFrame = nullptr);
	
	void UnInitlizeDx()
	{
		Safe_Delete(m_pDxSurface);
		Safe_Delete(m_pDDraw);
	}
#ifdef _DEBUG
	int m_nRenderFPS = 0;
	int m_nRenderFrames = 0;
	double dfTRender = 0.0f;
#endif
	/// @brief ��Ⱦһ֡
	void RenderFrame(AVFrame *pAvFrame)
	{
		if (!m_hRenderWnd)
			return;
		Autolock(m_cslistRenderWnd.Get());
		if (m_bEnableDDraw)
		{
			if (m_listRenderUnit.size() <= 0 || m_bStopFlag)
				return;
		}
		else  if (m_listRenderWnd.size() <= 0 || m_bStopFlag)
			return;
		
		lock.Unlock();
		if (pAvFrame->width != m_nVideoWidth ||
			pAvFrame->height != m_nVideoHeight)
		{
			Safe_Delete(m_pDxSurface);
			Safe_Delete(m_pDDraw);
			m_nVideoWidth = pAvFrame->width;
			m_nVideoHeight = pAvFrame->height;
			InitizlizeDx();
		}
		
		if (m_bFitWindow)
		{
			if (m_pDxSurface)
			{
				m_pDxSurface->Render(pAvFrame, m_nRocateAngle);
				Autolock(&m_cslistRenderWnd);
				if (m_listRenderWnd.size() <= 0)
					return;
				
				for (auto it = m_listRenderWnd.begin(); it != m_listRenderWnd.end(); it++)
				{
					if ((*it)->pRtBorder)
					{
						RECT rtBorder = { 0 };
						if (!(*it)->bPercent)
						{
							rtBorder.left = (*it)->pRtBorder->left;
							rtBorder.right = m_nVideoWidth - (*it)->pRtBorder->right;
							rtBorder.top = (*it)->pRtBorder->top;
							rtBorder.bottom = m_nVideoHeight - (*it)->pRtBorder->bottom;
						}
						else
						{
							rtBorder.left = (m_nVideoWidth*(*it)->pRtBorder->left) / 100;
							rtBorder.right = m_nVideoWidth - (m_nVideoWidth*(*it)->pRtBorder->right / 100);
							rtBorder.top = m_nVideoHeight*(*it)->pRtBorder->top / 100;
							rtBorder.bottom = m_nVideoHeight - (m_nVideoHeight*(*it)->pRtBorder->bottom / 100);
						}
						rtBorder.left = FFALIGN(rtBorder.left, 4);
						rtBorder.right = FFALIGN(rtBorder.right, 4);
						rtBorder.top = FFALIGN(rtBorder.top, 4);
						rtBorder.bottom = FFALIGN(rtBorder.bottom, 4);
						m_pDxSurface->Present((*it)->hRenderWnd, &rtBorder);
					}
					else
						m_pDxSurface->Present((*it)->hRenderWnd);
				}
			}
			else if (m_pDDraw)
			{
				if (pAvFrame->format == AV_PIX_FMT_DXVA2_VLD)
				{
					byte *pY = NULL , *pUV = NULL;
					int nPitch;
					LockDxvaFrame(pAvFrame, &pY, &pUV, nPitch);
					m_pYUVImage->dwLineSize[0] = nPitch;
					m_pYUVImage->pBuffer[0] = (PBYTE)pY;
					m_pYUVImage->pBuffer[1] = (PBYTE)pUV;
					UnlockDxvaFrame(pAvFrame);
				}
				else
				{
					m_pYUVImage->pBuffer[0] = (PBYTE)pAvFrame->data[0];
					m_pYUVImage->pBuffer[1] = (PBYTE)pAvFrame->data[1];
					m_pYUVImage->pBuffer[2] = (PBYTE)pAvFrame->data[2];
					m_pYUVImage->dwLineSize[0] = pAvFrame->linesize[0];
					m_pYUVImage->dwLineSize[1] = pAvFrame->linesize[1];
					m_pYUVImage->dwLineSize[2] = pAvFrame->linesize[2];
// 					RECT rtBorder;
// 					Autolock(&m_csBorderRect);
// 					if (m_pBorderRect)
// 					{
// 						CopyRect(&rtBorder, m_pBorderRect.get());
// 						lock.Unlock();
// 						m_pDDraw->Draw(*m_pYUVImage, &rtBorder, nullptr, true);
// 						Autolock1(&m_csListRenderUnit);
// 						for (auto it = m_listRenderUnit.begin(); it != m_listRenderUnit.end() && !m_bStopFlag; it++)
// 							(*it)->RenderImage(*m_pYUVImage, &rtBorder, nullptr, true);
// 					}
// 					else
// 					{
// 						lock.Unlock();
// 						m_pDDraw->Draw(*m_pYUVImage, nullptr, nullptr, true);
// 						Autolock1(&m_csListRenderUnit);
// 						for (auto it = m_listRenderUnit.begin(); it != m_listRenderUnit.end() && !m_bStopFlag; it++)
// 							(*it)->RenderImage(*m_pYUVImage, nullptr, nullptr, true);
// 					}
				}
				// RECT rtBorder;
				//m_pDDraw->Draw(*m_pYUVImage, &rtBorder, nullptr, true);
				Autolock(&m_csListRenderUnit);
				for (auto it = m_listRenderUnit.begin(); it != m_listRenderUnit.end() && !m_bStopFlag; it++)
				{
					if ((*it)->pRtBorder)
					{
						RECT rtBorder = { 0 };
						if (!(*it)->bPercent)
						{
							rtBorder.left = (*it)->pRtBorder->left;
							rtBorder.right = m_nVideoWidth - (*it)->pRtBorder->right;
							rtBorder.top = (*it)->pRtBorder->top;
							rtBorder.bottom = m_nVideoHeight - (*it)->pRtBorder->bottom;
						}
						else
						{
							rtBorder.left = (m_nVideoWidth*(*it)->pRtBorder->left) / 100;
							rtBorder.right = m_nVideoWidth - (m_nVideoWidth*(*it)->pRtBorder->right / 100);
							rtBorder.top = m_nVideoHeight*(*it)->pRtBorder->top / 100;
							rtBorder.bottom = m_nVideoHeight - (m_nVideoHeight*(*it)->pRtBorder->bottom / 100);
						}
						rtBorder.left = FFALIGN(rtBorder.left, 4);
						rtBorder.right = FFALIGN(rtBorder.right, 4);
						rtBorder.top = FFALIGN(rtBorder.top, 4);
						rtBorder.bottom = FFALIGN(rtBorder.bottom, 4);
						(*it)->RenderImage(*m_pYUVImage, &rtBorder, nullptr, true);
					}
					else
						(*it)->RenderImage(*m_pYUVImage, nullptr, nullptr, true);
				}
			}
		}
		else
		{
			RECT rtRender;
			GetWindowRect(m_hRenderWnd, &rtRender);
			int nWndWidth = rtRender.right - rtRender.left;
			int nWndHeight = rtRender.bottom - rtRender.top;
			float fScaleWnd = (float)nWndHeight / nWndWidth;
			float fScaleVideo = (float)pAvFrame->height / pAvFrame->width;
			if (fScaleVideo < fScaleWnd)			// ���ڸ߶ȳ�������,��Ҫȥ��һ���ָ߶�,��Ƶ��Ҫ���¾���
			{
				int nNewHeight = (int)nWndWidth * fScaleVideo;
				int nOverHeight = nWndHeight - nNewHeight;
				if ((float)nOverHeight / nWndHeight > 0.01f)	// ���ڸ߶ȳ���1%,������߶ȣ�������Ըò���
				{
					rtRender.top += nOverHeight / 2;
					rtRender.bottom -= nOverHeight / 2;
				}
			}
			else if (fScaleVideo > fScaleWnd)		// ���ڿ�ȳ�������,��Ҫȥ��һ���ֿ�ȣ���Ƶ��Ҫ���Ҿ���
			{
				int nNewWidth = nWndHeight/fScaleVideo;
				int nOverWidth = nWndWidth - nNewWidth;
				if ((float)nOverWidth / nWndWidth > 0.01f)
				{
					rtRender.left += nOverWidth / 2;
					rtRender.right -= nOverWidth / 2;
				}
			}
			if (m_pDxSurface)
			{
				m_cslistRenderWnd.Lock();
				ScreenToClient(m_hRenderWnd, (LPPOINT)&rtRender);
				ScreenToClient(m_hRenderWnd, ((LPPOINT)&rtRender) + 1);
				m_cslistRenderWnd.Unlock();
				RECT rtBorder;

				if (m_pBorderRect)
				{
					CopyRect(&rtBorder, m_pBorderRect.get());
					m_pDxSurface->Render(pAvFrame,m_nRocateAngle);
					Autolock(&m_cslistRenderWnd);
					m_pDxSurface->Present(m_hRenderWnd, &rtBorder, &rtRender);
					if (m_listRenderWnd.size() > 0)
					{
						for (auto it = m_listRenderWnd.begin(); it != m_listRenderWnd.end(); it++)
							m_pDxSurface->Present((*it)->hRenderWnd, &rtBorder, &rtRender);
					}
				}
				else
				{
					m_pDxSurface->Render(pAvFrame, m_nRocateAngle);
					Autolock(&m_cslistRenderWnd);
					m_pDxSurface->Present(m_hRenderWnd, nullptr, &rtRender);
					if (m_listRenderWnd.size() > 0)
					{
						for (auto it = m_listRenderWnd.begin(); it != m_listRenderWnd.end(); it++)
							m_pDxSurface->Present((*it)->hRenderWnd, nullptr, &rtRender);
					}
				}
			}
			else if (m_pDDraw)
			{
				m_pYUVImage->pBuffer[0] = (PBYTE)pAvFrame->data[0];
				m_pYUVImage->pBuffer[1] = (PBYTE)pAvFrame->data[1];
				m_pYUVImage->pBuffer[2] = (PBYTE)pAvFrame->data[2];
				m_pYUVImage->dwLineSize[0] = pAvFrame->linesize[0];
				m_pYUVImage->dwLineSize[1] = pAvFrame->linesize[1];
				m_pYUVImage->dwLineSize[2] = pAvFrame->linesize[2];
				RECT rtBorder;
				Autolock(&m_csBorderRect);
				if (m_pBorderRect)
				{
					CopyRect(&rtBorder, m_pBorderRect.get());
					lock.Unlock();
					m_pDDraw->Draw(*m_pYUVImage, &rtBorder, &rtRender, true);
				}
				else
				{
					lock.Unlock();
					m_pDDraw->Draw(*m_pYUVImage,nullptr, &rtRender, true);
				}	
			}
		}
	}
	// ���ֲ���
	int BinarySearch(time_t tTime)
	{
		int low = 0;
		int high = m_nTotalFrames - 1;
		int middle = 0;
		while (low <= high)
		{
			middle = (low + high) / 2;
			if (tTime >= m_pFrameOffsetTable[middle].tTimeStamp &&
				tTime <= m_pFrameOffsetTable[middle + 1].tTimeStamp)
				return middle;
			//������
			else if (tTime < m_pFrameOffsetTable[middle].tTimeStamp)
				high = middle - 1;
			//���Ұ��
			else if (tTime > m_pFrameOffsetTable[middle + 1].tTimeStamp)
				low = middle + 1;
		}
		//û�ҵ�
		return -1;
	}
public:
	/// @brief  ������־
	/// @param	szLogFile		��־�ļ���,���ò���Ϊnull���������־
	void EnableRunlog(const char *szLogFile)
	{
		char szFileLog[MAX_PATH] = { 0 };
		if (szLogFile && strlen(szLogFile) != 0)
		{
			strcpy(szFileLog, szLogFile);
			m_pRunlog = make_shared<CRunlog>(szFileLog);
		}
		else
			m_pRunlog = nullptr;
	}

	int EnableDDraw(bool bEnableDDraw = true)
	{
		if (m_pDDraw || m_pDxSurface)
			return IPC_Error_DXRenderInitialized;
		m_bEnableDDraw = bEnableDDraw;
		return IPC_Succeed;
	}

	/// @brief ������Ƶ���Ų���
	/// @param nPlayFPS		��Ƶ������֡��
	/// @param nSampleFreq	����Ƶ��
	/// @param nSampleBit	����λ��
	/// @remark �ڲ�����Ƶ֮ǰ��Ӧ��������Ƶ���Ų���,SDK�ڲ�Ĭ�ϲ���nPlayFPS = 50��nSampleFreq = 8000��nSampleBit = 16
	///         ����Ƶ���Ų�����SDK�ڲ�Ĭ�ϲ���һ�£����Բ���������Щ����
	void SetAudioPlayParameters(DWORD nPlayFPS = 50, DWORD nSampleFreq = 8000, WORD nSampleBit = 16)
	{
		m_nAudioPlayFPS	 = nPlayFPS;
		m_nSampleFreq	 = nSampleFreq;
		m_nSampleBit	 = nSampleBit;
	}
	void ClearOnException()
	{
		if (m_pszFileName)
		{
			delete[]m_pszFileName;
			m_pszFileName = nullptr;
		}
		if (m_pRunlog)
			m_pRunlog = nullptr;
		/*
		ʹ��CCriticalSectionProxy���������ֱ�ӵ���DeleteCriticalSection����
		DeleteCriticalSection(&m_csVideoCache); 
		*/
		if (m_hVideoFile)
			CloseHandle(m_hVideoFile);
	}
	CIPCPlayer(HWND hWnd, CHAR *szFileName = nullptr, char *szLogFile = nullptr);
	
	CIPCPlayer(HWND hWnd, int nBufferSize = 2048 * 1024, char *szLogFile = nullptr);

	~CIPCPlayer();
	
	/// @brief  �Ƿ�Ϊ�ļ�����
	/// @retval			true	�ļ�����
	/// @retval			false	������
	bool IsFilePlayer()
	{
		if (m_hVideoFile || m_pszFileName)
			return true;
		else
			return false;
	}
	// 
	class WndFinder {
	public:
		WndFinder(const HWND hWnd)
		{
			this->hWnd = hWnd; 
		}
	
		bool operator()(RenderWndPtr& Value)
		{
			if (Value->hRenderWnd == hWnd)
				return true;
			else
				return false;
		}
	public:
		HWND hWnd;
	};

	class UnitFinder {
	public:
		UnitFinder(const HWND hWnd)
		{
			this->hWnd = hWnd;
		}

		bool operator()(RenderUnitPtr& Value)
		{
			if (Value->hRenderWnd == hWnd)
				return true;
			else
				return false;
		}
	public:
		HWND hWnd;
	};

	class FrameFinder {
	public:
		FrameFinder(const time_t tFrame)
		{
			this->tFrame = tFrame;
		}

		bool operator()(CAVFramePtr& Value)
		{
			if (abs(Value->tFrame - tFrame)< 40)
				return true;
			else
				return false;
		}
	public:
		time_t tFrame;
	};
	int AddRenderWindow(HWND hRenderWnd, LPRECT pRtRender, bool bPercent = false);

	int RemoveRenderWindow(HWND hRenderWnd);

	// ��ȡ��ʾͼ�񴰿ڵ�����
	int GetRenderWindows(HWND* hWndArray, int &nSize);

	void SetRefresh(bool bRefresh = true)
	{
		m_bRefreshWnd = bRefresh;
	}

	// ����������ͷ
	// ��������ʱ������֮ǰ��������������ͷ
	int SetStreamHeader(CHAR *szStreamHeader, int nHeaderSize);

	int SetMaxFrameSize(int nMaxFrameSize = 256*1024)
	{
		if (m_hThreadFileParser || m_hThreadDecode)
			return IPC_Error_PlayerHasStart;
		m_nMaxFrameSize = nMaxFrameSize;
		return IPC_Succeed;
	}

	inline int GetMaxFrameSize()
	{
		return m_nMaxFrameSize;
	}

	inline void SetMaxFrameCache(int nMaxFrameCache = 100)
	{
		m_nMaxFrameCache = nMaxFrameCache;
	}

	void ClearFrameCache()
	{
		m_csVideoCache.Lock();
		m_listVideoCache.clear();
		m_csVideoCache.Lock();

		m_csAudioCache.Lock();
		m_listAudioCache.clear();
		m_csAudioCache.Lock();
	}
	
	/// @brief ������ʾ�߽�,�߽����ͼ�񽫲�������ʾ
	/// @param rtBorder	�߽���� �������ͼ��
	/// @param bPercent �Ƿ�ʹ�ðٷֱȿ��Ʊ߽�
	/// left	��߽����
	/// top		�ϱ߽����
	/// right	�ұ߽����
	/// bottom  �±߽���� 
	///������������������������������������������������������������������������������
	///��                  ��                  ��
	///��                 top                 ��
	///��                  ��                  ���������������� the source rect
	///��       ����������������������������������������������       ��
	///��       ��                     ��       ��
	///��       ��                     ��       ��
	///����left�� ��  the clipped rect   ����right����
	///��       ��                     ��       ��
	///��       ��                     ��       ��
	///��       ����������������������������������������������       ��
	///��                  ��                  ��
	///��                bottom               ��
	///��                  ��                  ��
	///������������������������������������������������������������������������������
	/// @remark �߽����������λ�ò��ɵߵ�
	int SetBorderRect(HWND hWnd, LPRECT pRectBorder, bool bPercent = false);

	void RemoveBorderRect(HWND hWnd)
	{
		Autolock(&m_cslistRenderWnd);
		auto itFind = find_if(m_listRenderWnd.begin(), m_listRenderWnd.end(), WndFinder(hWnd));
		if (itFind != m_listRenderWnd.end())
			(*itFind)->SetBorder(nullptr);
	}
	
	void SetDecodeDelay(int nDecodeDelay = -1)
	{
		if (nDecodeDelay == 0)
			EnableAudio(false);
		else
			EnableAudio(true);
		m_nDecodeDelay = nDecodeDelay;
		if (m_nDecodeDelay <= 2)
		{
			m_fPlayInterval = nDecodeDelay;
		}
	}
	/// @brief			��ʼ����
	/// @param [in]		bEnaleAudio		�Ƿ�����Ƶ����
	/// #- true			������Ƶ����
	/// #- false		�ر���Ƶ����
	/// @param [in]		bEnableHaccel	�Ƿ���Ӳ���빦��
	/// #- true			����Ӳ����
	/// #- false		����Ӳ����
	/// @param [in]		bFitWindow		��Ƶ�Ƿ���Ӧ����
	/// #- true			��Ƶ��������,�������ͼ������,���ܻ����ͼ�����
	/// #- false		ֻ��ͼ��ԭʼ�����ڴ�������ʾ,������������,���Ժ�ɫ������ʾ
	/// @retval			0	�����ɹ�
	/// @retval			1	������������
	/// @retval			-1	���������Ч
	/// @remark			����������ʱ����Ӧ��֡������ʵ��δ�������ţ����Ǳ����˲��Ŷ����У�Ӧ�ø���ipcplay_PlayStream
	///					�ķ���ֵ���жϣ��Ƿ�������ţ���˵��������������Ӧ����ͣ����
	int StartPlay(bool bEnaleAudio = false, bool bEnableHaccel = false, bool bFitWindow = true);

	/// @brief			��ʼͬ������
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
	int StartSyncPlay(bool bFitWindow = true, CIPCPlayer *pSyncSource = nullptr, int nVideoFPS = 25);

	/// �ƶ�����ʱ��㵽ָλλ��
	/// tTime			��Ҫ�����ʱ��㣬��λ����
	int SeekTime(time_t tTime)
	{
		if (!m_bAsyncRender)
			return IPC_Error_NotAsyncPlayer;
		if (m_hThreadAsyncReander)
			return IPC_Error_PlayerNotStart;
		m_tSyncTimeBase = tTime;
	}

	/// @brief			�жϲ������Ƿ����ڲ�����	
	/// @retval			true	���������ڲ�����
	/// @retval			false	�������ֹͣ����
	bool IsPlaying()
	{
		if (m_hThreadDecode)
			return true;
		else
			return false;
	}

	/// @brief ��λ���Ŵ���
	bool Reset(HWND hWnd = nullptr, int nWidth = 0, int nHeight = 0)
	{
// 		CAutoLock lock(&m_csDxSurface, false, __FILE__, __FUNCTION__, __LINE__);
// 		if (m_pDxSurface)
// 		{
// 			m_bDxReset = true;
// 			m_hDxReset = hWnd;
// 			return true;
// 		}
// 		else
// 			return false;
		return true;
	}

	int SetPixelFormat(PIXELFMORMAT nPixelFmt = YV12)
	{
		switch (nPixelFmt)
		{
		case YV12:
			m_nPixelFormat = (D3DFORMAT)MAKEFOURCC('Y', 'V', '1', '2');
			break;
		case NV12:
			m_nPixelFormat = (D3DFORMAT)MAKEFOURCC('N', 'V', '1', '2');
			break;
		case R8G8B8:
			m_nPixelFormat = D3DFMT_A8R8G8B8;
			break;
		default:
			assert(false);
			return IPC_Error_InvalidParameters;
			break;
		}
		return IPC_Succeed;
	}

	int GetFileHeader();
	
	void TryEnableHAccel(CHAR* szAdapterID,int nBuffer)
	{
		HMONITOR hMonitor = MonitorFromWindow(m_hRenderWnd, MONITOR_DEFAULTTONEAREST);
		if (hMonitor)
		{
			MONITORINFOEX mi;
			mi.cbSize = sizeof(MONITORINFOEX);
			if (GetMonitorInfo(hMonitor, &mi))
			{
				for (int i = 0; i < g_pD3D9Helper.m_nAdapterCount; i++)
				{
					if (strcmp(g_pD3D9Helper.m_AdapterArray[i].DeviceName, mi.szDevice) == 0)
					{
						m_nDisplayAdapter = i;
						OutputMsg("%s Wnd[%08X] is on Monitor:[%s],it's connected on Adapter[%i]:%s.\n", __FUNCTION__, m_hRenderWnd, mi.szDevice, i, g_pD3D9Helper.m_AdapterArray[i].Description);
						WCHAR szGuidW[64] = { 0 };
						CHAR szGuidA[64] = { 0 };
						StringFromGUID2(g_pD3D9Helper.m_AdapterArray[i].DeviceIdentifier, szGuidW,64);
						
						W2AHelper(szGuidW, szGuidA, 64);
						if (szAdapterID)
							strcpy_s(szAdapterID, nBuffer, szGuidA);
						CAutoLock lock(m_csMapHacceConfig.Get());
						auto itFind = m_MapHacceConfig.find(szGuidA);
						if (itFind != m_MapHacceConfig.end())
						{
							if (itFind->second->nOpenedCount < itFind->second->nMaxCount)
							{
								m_bD3dShared = true;
								m_bEnableHaccel = true;
								itFind->second->nOpenedCount++;
								OutputMsg("%s HAccels On:Monitor:%s,Adapter:%s is %d.\n", __FUNCTION__, mi.szDevice, g_pD3D9Helper.m_AdapterArray[i].Description, itFind->second->nOpenedCount);
							}
							else
							{
								OutputMsg("%s HAccels On:Monitor:%s,Adapter:%s has reached up limit��%d.\n", __FUNCTION__, mi.szDevice, g_pD3D9Helper.m_AdapterArray[i].Description, itFind->second->nMaxCount);
							}
						}
						break;
					}
				}
			}
		}

	}
	void FitWindow(bool bFitWindow)
	{
		m_bFitWindow = bFitWindow;
	}

	/// @brief			����IPC˽��ʽ����������
	/// @retval			0	�����ɹ�
	/// @retval			1	������������
	/// @retval			-1	���������Ч
	/// @remark			����������ʱ����Ӧ��֡������ʵ��δ�������ţ����Ǳ����˲��Ŷ����У�Ӧ�ø���ipcplay_PlayStream
	///					�ķ���ֵ���жϣ��Ƿ�������ţ���˵��������������Ӧ����ͣ����
	int InputStream(unsigned char *szFrameData, int nFrameSize, UINT nCacheSize, bool bThreadInside = false/*�Ƿ��ڲ��̵߳��ñ�־*/);
	

	/// @brief			����IPC��������
	/// @retval			0	�����ɹ�
	/// @retval			1	������������
	/// @retval			-1	���������Ч
	/// @remark			����������ʱ����Ӧ��֡������ʵ��δ�������ţ����Ǳ����˲��Ŷ����У�Ӧ�ø���ipcplay_PlayStream
	///					�ķ���ֵ���жϣ��Ƿ�������ţ���˵��������������Ӧ����ͣ����
// 	UINT m_nAudioFrames1 = 0;
// 	UINT m_nVideoFraems = 0;
// 	DWORD m_dwInputStream = timeGetTime();
	int InputStream(IN byte *pFrameData, IN int nFrameType, IN int nFrameLength, int nFrameNum, time_t nFrameTime);

	// ����δ��������
	int InputStream(IN byte *pData, IN int nLength);

	bool IsH264KeyFrame(byte *pFrame)
	{
		int RTPHeaderBytes = 0;
		if (pFrame[0] == 0 &&
			pFrame[1] == 0 &&
			pFrame[2] == 0 &&
			pFrame[3] == 1)
		{
			int nal_type = pFrame[4] & 0x1F;
			if (nal_type == 5 || nal_type == 7 || nal_type == 8 || nal_type == 2)
			{
				return true;
			}
		}

		return false;
	}

	int InputDHStream(byte *pBuffer, int nLength);
	
	bool StopPlay(DWORD nTimeout = 50);

	void Pause()
	{
		m_bPause = !m_bPause;
	}

	/// @brief			����̽����������ʱ���ȴ������ĳ�ʱֵ
	/// @param [in]		dwTimeout �ȴ������ĸ�ʱֵ����λ����
	/// @remark			�ú�������Ҫ��Start֮ǰ���ã�������Ч
	void SetProbeStreamTimeout(DWORD dwTimeout = 3000)
	{
		m_nProbeStreamTimeout = dwTimeout;
	}

	/// @brief			����Ӳ���빦��
	/// @param [in]		bEnableHaccel	�Ƿ���Ӳ���빦��
	/// #- true			����Ӳ���빦��
	/// #- false		�ر�Ӳ���빦��
	/// @remark			�����͹ر�Ӳ���빦�ܱ���ҪStart֮ǰ���ò�����Ч
	int  EnableHaccel(IN bool bEnableHaccel = false);

	/// @brief			��ȡӲ����״̬
	/// @retval			true	�ѿ���Ӳ���빦��
	/// @retval			flase	δ����Ӳ���빦��
	inline bool  GetHaccelStatus()
	{
		return m_bEnableHaccel;
	}


/// @brief			����/�����첽��Ⱦ
/// @param [in]		hPlayHandle		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
/// @param [in]		bEnable			�Ƿ������첽��Ⱦ
/// -#	true		�����첽��Ⱦ
/// -#	false		�����첽��Ⱦ
/// @retval			0	�����ɹ�
/// @retval			-1	���������Ч
/// �����첽��Ⱦ�󣬽���õ���YUV���ݻᱻ����YUV���У���Ⱦʱ��YUV����ȡ������˱���������YUV���е����ֵ����Ȼ���ܻᵼ���ڴ治��
	int EnableAsyncRender(bool bEnable = true,int nFrameCache = 50)
	{
		if (m_hThreadDecode)
			return IPC_Error_PlayerHasStart;
		m_bAsyncRender = bEnable;
		m_nListAvFrameMaxSize = nFrameCache;
		return IPC_Succeed;
	}
	/// @brief			����Ƿ�֧���ض���Ƶ�����Ӳ����
	/// @param [in]		nCodec		��Ƶ�����ʽ,@see IPC_CODEC
	/// @retval			true		֧��ָ����Ƶ�����Ӳ����
	/// @retval			false		��֧��ָ����Ƶ�����Ӳ����
	static bool  IsSupportHaccel(IN IPC_CODEC nCodec);

	/// @brief			ȡ�õ�ǰ������Ƶ�ļ�ʱ��Ϣ
	int GetPlayerInfo(PlayerInfo *pPlayInfo);
	
// 	static void CloseShareMemory(HANDLE &hMemHandle, void *pMemory)
// 	{
// 		if (!pMemory || !hMemHandle)
// 			return;
// 		if (pMemory)
// 		{
// 			UnmapViewOfFile(pMemory);
// 			pMemory = NULL;
// 		}
// 		if (hMemHandle)
// 		{
// 			CloseHandle(hMemHandle);
// 			hMemHandle = NULL;
// 		}
// 	}
// 
// 	static void *OpenShareMemory(HANDLE &hMemHandle, TCHAR *szUniqueName)
// 	{
// 		void *pShareSection = NULL;
// 		bool bResult = false;
// 		__try
// 		{
// 			__try
// 			{
// 				if (!szUniqueName || _tcslen(szUniqueName) < 1)
// 					__leave;
// 				hMemHandle = OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, szUniqueName);
// 				if (hMemHandle == INVALID_HANDLE_VALUE ||
// 					hMemHandle == NULL)
// 				{
// 					_TraceMsg(_T("%s %d MapViewOfFile Failed,ErrorCode = %d.\r\n"), __FILEW__, __LINE__, GetLastError());
// 					__leave;
// 				}
// 				pShareSection = MapViewOfFile(hMemHandle, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 0);
// 				if (pShareSection == NULL)
// 				{
// 					_TraceMsg(_T("%s %d MapViewOfFile Failed,ErrorCode = %d.\r\n"), __FILEW__, __LINE__, GetLastError());
// 					__leave;
// 				}
// 				bResult = true;
// 				_TraceMsg(_T("Open Share memory map Succeed."));
// 			}
// 			__finally
// 			{
// 				if (!bResult)
// 				{
// 					CloseShareMemory(hMemHandle, pShareSection);
// 					pShareSection = NULL;
// 				}
// 			}
// 		}
// 		__except (1)
// 		{
// 			TraceMsg(_T("Exception in OpenShareMemory.\r\n"));
// 		}
// 		return pShareSection;
// 	}
	/// @brief			��ȡ���Ų��ŵ���Ƶͼ��
	/// @param [in]		szFileName		Ҫ������ļ���
	/// @param [in]		nFileFormat		�����ļ��ı����ʽ,@see SNAPSHOT_FORMAT����
	/// @retval			0	�����ɹ�
	/// @retval			-1	���������Ч
	int  SnapShot(IN CHAR *szFileName, IN SNAPSHOT_FORMAT nFileFormat = XIFF_JPG);
	
	/// @brief			�����ͼ����
	/// remark			������ɺ󣬽�����m_hEventSnapShot�¼�
	void ProcessSnapshotRequire(AVFrame *pAvFrame);

	/// @brief			���ò��ŵ�����
	/// @param [in]		nVolume			Ҫ���õ�����ֵ��ȡֵ��Χ0~100��Ϊ0ʱ����Ϊ����
	/// @retval			0	�����ɹ�
	/// @retval			-1	���������Ч
	void  SetVolume(IN int nVolume)
	{
		if (m_bEnableAudio && m_pDsBuffer)
		{
			//int nDsVolume = nVolume * 100 - 10000;
			m_pDsBuffer->SetVolume(nVolume);
		}
	}

	/// @brief			ȡ�õ�ǰ���ŵ�����
	int  GetVolume()
	{
		if (m_bEnableAudio && m_pDsBuffer)
			return (m_pDsBuffer->GetVolume() + 10000) / 100;
		else
			return 0;
	}

	/// @brief			���õ�ǰ���ŵ��ٶȵı���
	/// @param [in]		fPlayRate		��ǰ�Ĳ��ŵı���,����1ʱΪ���ٲ���,С��1ʱΪ���ٲ��ţ�����Ϊ0��С��0
	/// @retval			0	�����ɹ�
	/// @retval			-1	���������Ч
	int  SetRate(IN float fPlayRate);

	/// @brief			��Ծ��ָ��Ƶ֡���в���
	/// @param [in]		nFrameID	Ҫ����֡����ʼID
	/// @param [in]		bUpdate		�Ƿ���»���,bUpdateΪtrue�����Ը��»���,�����򲻸���
	/// @retval			0	�����ɹ�
	/// @retval			-1	���������Ч
	/// @remark			1.����ָ��ʱ����Ӧ֡Ϊ�ǹؼ�֡��֡�Զ��ƶ����ͽ��Ĺؼ�֡���в���
	///					2.����ָ��֡Ϊ�ǹؼ�֡��֡�Զ��ƶ����ͽ��Ĺؼ�֡���в���
	///					3.ֻ���ڲ�����ʱ,bUpdate��������Ч
	///					4.���ڵ�֡����ʱֻ����ǰ�ƶ�
	int  SeekFrame(IN int nFrameID, bool bUpdate = false);
	
	/// @brief			��Ծ��ָ��ʱ��ƫ�ƽ��в���
	/// @param [in]		tTimeOffset		Ҫ���ŵ���ʼʱ��,��λ��,��FPS=25,��0.04��Ϊ��2֡��1.00�룬Ϊ��25֡
	/// @param [in]		bUpdate		�Ƿ���»���,bUpdateΪtrue�����Ը��»���,�����򲻸���
	/// @retval			0	�����ɹ�
	/// @retval			-1	���������Ч
	/// @remark			1.����ָ��ʱ����Ӧ֡Ϊ�ǹؼ�֡��֡�Զ��ƶ����ͽ��Ĺؼ�֡���в���
	///					2.����ָ��֡Ϊ�ǹؼ�֡��֡�Զ��ƶ����ͽ��Ĺؼ�֡���в���
	///					3.ֻ���ڲ�����ʱ,bUpdate��������Ч
	int  SeekTime(IN time_t tTimeOffset, bool bUpdate);

	/// ����ָ����֡����ֻ��Ⱦһ֡
	/// @param [in]		tTimeOffset		Ҫ���ŵ���ʼʱ��,��λ��,��FPS=25,��0.04��Ϊ��2֡��1.00�룬Ϊ��25֡
	/// @param [in]		bUpdate		�Ƿ���»���,bUpdateΪtrue�����Ը��»���,�����򲻸���
	/// @retval			0	�����ɹ�
	/// @retval			-1	���������Ч
	/// @remark			1.ֻ���첽��Ⱦʱ���ú���������Ч�����򷵻�IPC_Error_NotAsyncPlayer����
	///					2.���������������󣬸ú����������������򷵻�IPC_Error_PlayerNotStart����
	int  AsyncSeekTime(IN time_t tTimeOffset, bool bUpdate);

	/// @brief ���ļ��ж�ȡһ֡����ȡ�����Ĭ��ֵΪ0,SeekFrame��SeekTime���趨�����λ��
	/// @param [in,out]		pBuffer		֡���ݻ�����,������Ϊnull
	/// @param [in,out]		nBufferSize ֡�������Ĵ�С
	int GetFrame(INOUT byte **pBuffer, OUT UINT &nBufferSize);
	
	/// @brief			������һ֡
	/// @retval			0	�����ɹ�
	/// @retval			-24	������δ��ͣ
	/// @remark			�ú����������ڵ�֡����
	int  SeekNextFrame();

	/// @brief			��/����Ƶ����
	/// @param [in]		bEnable			�Ƿ񲥷���Ƶ
	/// -#	true		������Ƶ����
	/// -#	false		������Ƶ����
	/// @retval			0	�����ɹ�
	/// @retval			-1	���������Ч
	/// @retval			IPC_Error_AudioFailed	��Ƶ�����豸δ����
	int  EnableAudio(bool bEnable = true);

	/// @brief			ˢ�²��Ŵ���
	/// @retval			0	�����ɹ�
	/// @retval			-1	���������Ч
	/// @remark			�ù���һ�����ڲ��Ž�����ˢ�´��ڣ��ѻ�����Ϊ��ɫ
	void  Refresh();

	void SetBackgroundImage(LPCWSTR szImageFilePath = nullptr);

	// �������ʧ��ʱ������0�����򷵻�������ľ��
	long AddLineArray(POINT *pPointArray, int nCount, float fWidth, D3DCOLOR nColor);

	int	RemoveLineArray(long nIndex);

	long AddPolygon(POINT *pPointArray, int nCount, WORD *pVertexIndex, DWORD nColor);

	int RemovePolygon(long nIndex);

	int SetCoordinateMode(int nMode = 1)
	{
		m_nCoordinate = (Coordinte)nMode;
		return IPC_Succeed;
	}
	int SetCallBack(IPC_CALLBACK nCallBackType, IN void *pUserCallBack, IN void *pUserPtr);
	

	/// @brief			ȡ���ļ�����������Ϣ,���ļ���֡��,�ļ�ƫ�Ʊ�
	/// �Զ�ȡ����ļ����ݵ���ʽ����ȡ֡��Ϣ��ִ��Ч���ϱ�GetFileSummary0Ҫ��
	/// Ϊ���Ч�ʣ���ֹ�ļ������̺߳������߳̾������̶�ȡȨ���ڵ�ȡ������Ϣ��ͬʱ��
	/// �򻺳���Ͷ��16��(400֡)����Ƶ���ݣ��൱�ڽ����߳̿����ӳ�8-10��ſ�ʼ������
	/// �����˾������������ٶ�;���ͬʱ���û����������������棬�������û����飻
	int GetFileSummary(volatile bool &bWorking);

	/// @brief			����֡����
	/// @param [in,out]	pBuffer			����IPC˽��¼���ļ��е�������
	/// @param [in,out]	nDataSize		pBuffer����Ч���ݵĳ���
	/// @param [out]	pFrameParser	IPC˽��¼���֡����	
	/// @retval			true	�����ɹ�
	/// @retval			false	ʧ�ܣ�pBuffer�Ѿ�û����Ч��֡����
	bool ParserFrame(INOUT byte **ppBuffer,	INOUT DWORD &nDataSize,	FrameParser* pFrameParser);

	///< @brief ��Ƶ�ļ������߳�
	static UINT __stdcall ThreadFileParser(void *p);

	int EnableStreamParser(IPC_CODEC nCodec = CODEC_H264);

	///< @brief ��Ƶ�������߳�
	static UINT __stdcall ThreadStreamParser(void *p);

	/// @brief			��ȡ��������	
	/// @param [in]		pVideoCodec		��ipcplay_OpenFile��ipcplay_OpenStream���صĲ��ž��
	/// @param [out]	pAudioCodec	���ص�ǰhPlayHandle�Ƿ��ѿ���Ӳ���빦��
	/// @remark �������Ͷ�����ο�:@see IPC_CODEC
	/// @retval			0	�����ɹ�
	/// @retval			-1	���������Ч
	/// @retval			IPC_Error_PlayerNotStart	��������δ����,�޷�ȡ�ò��Ź��̵���Ϣ������
	int GetCodec(IPC_CODEC *pVideoCodec, IPC_CODEC *pAudioCodec)
	{
		if (!m_hThreadDecode)
			return IPC_Error_PlayerNotStart;
		if (pVideoCodec)
			*pVideoCodec = m_nVideoCodec;
		if (pAudioCodec)
			*pAudioCodec = m_nAudioCodec;
		return IPC_Succeed;
	}

	// ̽����Ƶ��������,Ҫ���������I֡
	bool ProbeStream(byte *szFrameBuffer, int nBufferLength);
			
	/// @brief ��NV12ͼ��ת��ΪYUV420Pͼ��
	void CopyNV12ToYUV420P(byte *pYV12, byte *pNV12[2], int src_pitch[2], unsigned width, unsigned height);

	/// @brief ��DxvaӲ����NV12֡ת����YV12ͼ��
	void CopyDxvaFrame(byte *pYuv420, AVFrame *pAvFrameDXVA);
	
	void CopyDxvaFrameYV12(byte **ppYV12, int &nStrideY, int &nWidth, int &nHeight, AVFrame *pAvFrameDXVA);
	
	void CopyDxvaFrameNV12(byte **ppNV12, int &nStrideY, int &nWidth, int &nHeight, AVFrame *pAvFrameDXVA);
	
	bool LockDxvaFrame(AVFrame *pAvFrameDXVA, byte **ppSrcY, byte **ppSrcUV, int &nPitch);

	void UnlockDxvaFrame(AVFrame *pAvFrameDXVA);

	// ��YUVC420P֡���Ƶ�YV12������
	void CopyFrameYUV420(byte *pYUV420, int nYUV420Size, AVFrame *pFrame420P);
	
	void ProcessYUVFilter(AVFrame *pAvFrame, LONGLONG nTimestamp);
	
	void ProcessYUVCapture(AVFrame *pAvFrame, LONGLONG nTimestamp);

// 	int YUV2RGB24(const unsigned char* pY,
// 		const unsigned char* pU,
// 		const unsigned char* pV,
// 		int nStrideY,
// 		int nStrideUV,
// 		int nWidth,
// 		int nHeight,
// 		byte **pRGBBuffer,
// 		long &nLineSize)
// 	{
// 		AVFrame *pFrameYUV = av_frame_alloc();
// 		pFrameYUV->data[0] = (uint8_t *)pY;
// 		pFrameYUV->data[1] = (uint8_t *)pU;
// 		pFrameYUV->data[2] = (uint8_t *)pV;
// 		pFrameYUV->linesize[0] = nStrideY;
// 		pFrameYUV->linesize[1] = nStrideUV;
// 		pFrameYUV->linesize[2] = nStrideUV;
// 		pFrameYUV->width = nWidth;
// 		pFrameYUV->height = nHeight;
// 		pFrameYUV->format = AV_PIX_FMT_YUV420P;
// 		//pFrameYUV->pict_type = AV_PICTURE_TYPE_P;
// 		if (!m_pConvert)
// 			m_pConvert = make_shared<PixelConvert>(pFrameYUV, D3DFMT_R8G8B8);
// 		int nStatus = m_pConvert->ConvertPixel();
// 		if (nStatus> 0 )
// 		{
// 			*pRGBBuffer = m_pConvert->pImage;
// 			nLineSize = m_pConvert->pFrameNew->linesize[0];
// 		}
// 		return nStatus;
// 	}
	
	/// @brief			����̽���ȡ���ݰ��ص�����
	/// @param [in]		opaque		�û�����Ļص���������ָ��
	/// @param [in]		buf			��ȡ���ݵĻ���
	/// @param [in]		buf_size	����ĳ���
	/// @return			ʵ�ʶ�ȡ���ݵĳ���
	static int ReadAvData(void *opaque, uint8_t *buf, int buf_size);

	/// @brief			ʵ��ָʱʱ������ʱ
	/// @param [in]		dwDelay		��ʱʱ������λΪms
	/// @param [in]		bStopFlag	����ֹͣ�ı�־,Ϊfalseʱ����ֹͣ��ʱ
	/// @param [in]		nSleepGranularity	��ʱʱSleep������,����ԽС����ֹ��ʱԽѸ�٣�ͬʱҲ����CPU����֮��Ȼ
	static void Delay(DWORD dwDelay, volatile bool &bStopFlag, WORD nSleepGranularity = 20)
	{
		DWORD dwTotal = 0;
		while (bStopFlag && dwTotal < dwDelay)
		{
			Sleep(nSleepGranularity);
			dwTotal += nSleepGranularity;
		}
	}
	bool m_bD3dShared = false;
	void SetD3dShared(bool bD3dShared = true)
	{
		m_bD3dShared = bD3dShared;
	}

	// ָ���������
	// �����������ָ������ڴ�
	
	static UINT __stdcall ThreadDecode(void *p);
	
	static UINT __stdcall ThreadPlayAudioGSJ(void *p);

	static UINT __stdcall ThreadPlayAudioIPC(void *p);

	RocateAngle m_nRocateAngle;
	AVFrame		*m_pRacoateFrame = nullptr;
	byte		*m_pRocateImage = nullptr;
	int SetRocate(RocateAngle nRocate = Rocate90)
	{
		if (m_bEnableHaccel)
			return IPC_Error_AudioDeviceNotReady;
		m_nRocateAngle = nRocate;
		return IPC_Succeed;
	}

	bool InitialziePlayer();
	
	
	// �����첽��Ⱦ�ĵ���֡����
	bool PopFrame(CAVFramePtr &pAvFrame,int &nSize)
	{
		CAutoLock lock(m_cslistAVFrame.Get(), false, __FILE__, __FUNCTION__, __LINE__);
		if (m_listAVFrame.size())
		{
			pAvFrame = m_listAVFrame.front();
			m_listAVFrame.pop_front();
			nSize = m_listAVFrame.size();
			return true;
		}
		else
			return false;
	}
	
	/// ���õ�֡����
	/// ����Start֮ǰ����
	int EnabelPlayOneFrame(bool bEanble = true)
	{
		if (m_hThreadDecode || m_hThreadAsyncReander)
			return IPC_Error_PlayerHasStart;
		m_bPlayOneFrame = true;
		return IPC_Succeed;
	}
	
	// �����첽��Ⱦ��ѹ��֡����
	int PushFrame(AVFrame *pSrcFrame,time_t tFrame)
	{
		CAVFramePtr pFrame = make_shared<CAvFrame>(pSrcFrame,tFrame);
		CAutoLock lock(m_cslistAVFrame.Get(),false,__FILE__,__FUNCTION__,__LINE__);
		if (m_listAVFrame.size() < m_nListAvFrameMaxSize)
			m_listAVFrame.push_back(pFrame);
		else
		{
			while (m_listAVFrame.size() >= m_nListAvFrameMaxSize)
				m_listAVFrame.pop_front();
		}
		return m_listAVFrame.size();
	}
	
	static void *_AllocAvFrame()
	{
		return av_frame_alloc();
	}
	static void _AvFree(void *p)
	{
		av_free(p);
	}
	
// 	int EnableAsyncRender(bool bAsync)
// 	{
// 		if (m_hThreadDecode)
// 			return IPC_Error_PlayerHasStart;
// 		m_bAsyncRender = bAsync;
// 		return IPC_Succeed;
// 	}
	static UINT __stdcall ThreadAsyncRender(void *p);
	static UINT __stdcall ThreadSyncDecode(void *p);
// 	static UINT __stdcall ThreadReversePlay(void *p)
// 	{
// 		CIPCPlayer *pThis = (CIPCPlayer *)p;
// 		return pThis->ReversePlayRun();
// 	}
//
// 	UINT ReversePlayRun();
	/// @brief			�������򲥷�
	/// @remark			���򲥷ŵ�ԭ�����ȸ��ٽ��룬��ͼ����������ȳ����еĻ�����в��ţ�����Ҫ���򲥷ŷţ���ӻ���β����ͷ�����ţ��γ�����Ч��
	/// @param [in]		bFlag			�Ƿ��������򲥷ţ�Ϊtrueʱ�����ã�Ϊfalseʱ����رգ��رպͿ�������������Ƶ֡����
	/// @param [in]		nCacheFrames	���򲥷���Ƶ֡��������
	/// void EnableReservePlay(bool bFlag = true);
	int SetDisplayAdapter(int nAdapterNo)
	{
		if (nAdapterNo >= 0)
		{ 
			m_nDisplayAdapter = nAdapterNo;
			return IPC_Succeed;
		}
		else
			return IPC_Error_InvalidParameters;
	}
};