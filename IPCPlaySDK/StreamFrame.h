#pragma once
#include "Common.h"

struct  CTraceAlignMalloc
{
	int nCount;
	CRITICAL_SECTION cs;
	CTraceAlignMalloc()
	{
		ZeroMemory(this, sizeof(CTraceAlignMalloc));
		InitializeCriticalSection(&cs);
	}
	~CTraceAlignMalloc()
	{
		EnterCriticalSection(&cs);
		//TraceMsg("%s leak memory count = %d.\n",__FUNCTION__, nCount);
		LeaveCriticalSection(&cs);
		DeleteCriticalSection(&cs);
	}

	void * AlignMalloc(int nSize,int nAlignment)
	{
		Autolock(&cs);
		nCount++;
		return _aligned_malloc(nSize, nAlignment);
	}

	void AlignFree(void *p)
	{
		Autolock(&cs);
		nCount--;
		_aligned_free(p);
	}
};

/// @struct StreamFrame
/// StreamFrame��������ý�岥�ŵ�����֡

struct StreamFrame;
typedef shared_ptr<StreamFrame> StreamFramePtr;

struct StreamFrame
{
	StreamFrame()
	{
		ZeroMemory(this, sizeof(StreamFrame));
	}
	/// @brief	�����ѷ�װ�õĴ�IPCFrameHeaderͷ��IPCFrameHeaderExͷ������
	StreamFrame(byte *pBuffer, int nLenth, INT nFrameInterval, int nSDKVersion = IPC_IPC_SDK_VERSION_2015_12_16)
	{
		assert(pBuffer != nullptr);
		assert(nLenth >= sizeof(IPCFrameHeader));
		nSize = nLenth;
		pInputData = (byte *)_aligned_malloc(nSize, 16);
		if (!pInputData)
		{
			assert(false);
			return;
		}
		//pInputData = _New byte[nSize];
		if (pInputData)
			memcpy(pInputData, pBuffer, nLenth);
		IPCFrameHeader* pHeader = (IPCFrameHeader *)pInputData;
		dfInputTime = GetExactTime();
		if (nSDKVersion >= IPC_IPC_SDK_VERSION_2015_12_16 && nSDKVersion != IPC_IPC_SDK_GSJ_HEADER)
			pHeader->nTimestamp = ((IPCFrameHeaderEx*)pHeader)->nFrameID*nFrameInterval * 1000;
	}
	/// @brief	�����������������ʵʱ����������
	StreamFrame(IN byte *pFrame, IN int nFrameType, IN int nFrameLength, int nFrameNum, time_t nFrameTime)
	{
		nSize = sizeof(IPCFrameHeaderEx) + nFrameLength;
		pInputData = (byte *)_aligned_malloc(nSize, 16);
		if (!pInputData)
		{
			assert(false);
			return;
		}
		dfInputTime = GetExactTime();
		IPCFrameHeaderEx *pHeader = (IPCFrameHeaderEx *)pInputData;
#ifdef _DEBUG
		ZeroMemory(pHeader, nSize);
#endif
		switch (nFrameType)
		{
		case 0:									// ��˼I֡��Ϊ0�����ǹ̼���һ��BUG���д�����
		case IPC_IDR_FRAME: 	// IDR֡��
		case IPC_I_FRAME:		// I֡��
			// 			pHeader->nType = FRAME_I;
			// 			break;
		case IPC_P_FRAME:       // P֡
		case IPC_B_FRAME:       // B֡
		case IPC_711_ALAW:      // 711 A�ɱ���֡
		case IPC_711_ULAW:      // 711 U�ɱ���֡
		case IPC_726:           // 726����֡
		case IPC_AAC:           // AAC����֡��
		case IPC_GOV_FRAME:
			pHeader->nType = nFrameType;
			break;
		default:
		{
			assert(false);
			break;
		}
		}
		pHeader->nLength = nFrameLength;
		pHeader->nType = nFrameType;
		pHeader->nTimestamp = nFrameTime;
		pHeader->nFrameTag = IPC_TAG;		///< IPC_TAG
		if (!nFrameTime)
			pHeader->nFrameUTCTime = time(NULL);
		else
			pHeader->nFrameUTCTime = nFrameTime;

		pHeader->nFrameID = nFrameNum;
#ifdef _DEBUG
		pHeader->dfRecvTime = GetExactTime();
#endif
		memcpy(pInputData + sizeof(IPCFrameHeaderEx), pFrame, nFrameLength);
#ifdef _DEBUG
		//OutputMsg("%s pInputData = %08X size = %d.\n", __FUNCTION__, (long)pInputData,nSize);
#endif
	}

	inline const IPCFrameHeaderEx* FrameHeader()
	{
		return (IPCFrameHeaderEx*)pInputData;
	}
	inline const byte *Framedata(int nSDKVersion = IPC_IPC_SDK_VERSION_2015_12_16)
	{
		if (nSDKVersion >= IPC_IPC_SDK_VERSION_2015_12_16 && nSDKVersion != IPC_IPC_SDK_GSJ_HEADER)
			return (pInputData + sizeof(IPCFrameHeaderEx));
		else
			return (pInputData + sizeof(IPCFrameHeader));
	}
	~StreamFrame()
	{
		if (pInputData)
		{
			_aligned_free(pInputData);
			pInputData = nullptr;
		}
#ifdef _DEBUG
		//OutputMsg("%s pInputData = %08X.\n", __FUNCTION__, (long)pInputData);
		//ZeroMemory(this, sizeof(IPCFrameHeaderEx));
#endif
	}
	static bool IsIFrame(StreamFramePtr FramePtr)
	{
		//if (FramePtr->FrameHeader()->nType == FRAME_I)
		const IPCFrameHeaderEx* pHeader = FramePtr->FrameHeader();
		return (pHeader->nType == 0
			|| pHeader->nType == FRAME_IDR
			|| pHeader->nType == FRAME_I
			|| pHeader->nType == FRAME_GOV);
	}
	double  dfInputTime;
	int		nSize;
	byte	*pInputData;
};

#define _Frame_PERIOD			5.0f		///< һ��֡�ʼ�������

/// @brief ����IPC˽��֡�ṹ��
struct FrameParser
{
	union
	{
		IPCFrameHeader		*pHeader;		///< IPC�ɰ�˽��¼���֡����
		IPCFrameHeaderEx	*pHeaderEx;		///< IPC�°�˽��¼���֡����
	};

	UINT				nFrameSize;		///< pFrame�����ݳ���
	byte				*pRawFrame;		///< ԭʼ��������
	UINT				nRawFrameSize;	///< ԭʼ�������ݳ���
};


// ����ת���°��֡ͷ
#define FrameSize(p)	(((IPCFrameHeaderEx*)p)->nLength + sizeof(IPCFrameHeaderEx))	
#define Frame(p)		((IPCFrameHeaderEx *)p)

// ����ת���ɰ��֡ͷ
#define FrameSize2(p)	(((IPCFrameHeader*)p)->nLength + sizeof(IPCFrameHeader))	
#define Frame2(p)		((IPCFrameHeader *)p)

