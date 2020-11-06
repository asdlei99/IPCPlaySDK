﻿#pragma once
#include <d3d9.h>
#include <d3dx9tex.h>
//#include <ppl.h>
#include <assert.h>
using namespace std;
#include "../../include/Runlog.h"

#ifdef _STD_SMARTPTR
#include <memory>
//using namespace std::tr1;
#else
#include <boost/shared_ptr.hpp>
using namespace boost;
#endif


#include <map>
#include <list>
#include <algorithm>
#include <MMSystem.h>
#include <windows.h>
#include <tchar.h>
#include <wtypes.h>
#include <process.h>
#include <Shlwapi.h>
#include "gpu_memcpy_sse4.h"
#include "DxTrace.h"
#include "../AutoLock.h"
#include "../Runlog.h"
#include "../Utility.h"
#include "../TimeUtility.h"
#include "../CriticalSectionProxy.h"

#define Min(x,y)	(x<y?x:y)
#pragma warning(push)

#ifdef __cplusplus
extern "C" {
#endif
#define __STDC_CONSTANT_MACROS
#define FF_API_PIX_FMT 0
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libavutil/imgutils.h"
#include "libavutil/avstring.h"
//#include "libavutil/pixfmt.h"	

//#define _ENABLE_FFMPEG_STAITC_LIB

#ifdef _ENABLE_FFMPEG_STAITC_LIB
#pragma comment(lib,"libgcc.a")
//#pragma comment(lib,"libmingwex.a")
//#pragma comment(lib,"libcoldname.a")
#pragma comment(lib,"libavcodec.a")
#pragma comment(lib,"libavformat.a")
#pragma comment(lib,"libavutil.a")
#pragma comment(lib,"libswscale.a")
#pragma comment(lib,"libswresample.a")
#pragma comment(lib,"WS2_32.lib")	
#else
#pragma comment(lib,"avcodec.lib")
	//#pragma comment(lib,"avfilter.lib")
#pragma comment(lib,"avformat.lib")
#pragma comment(lib,"avutil.lib")
#pragma comment(lib,"swscale.lib")	
#endif	
#ifdef __cplusplus
}
#endif
#pragma warning(pop)
#pragma warning(push)
#pragma warning(disable:4244)
#ifdef __cplusplus
extern "C" {
#endif
#define __STDC_CONSTANT_MACROS
#define FF_API_PIX_FMT 0
#include "libavutil/frame.h"
#include "libavutil/cpu.h"
#ifdef __cplusplus
}
#endif
#pragma warning(pop)
#pragma warning(disable:4244)
#pragma warning(disable:4838)

//#pragma comment ( lib, "d3d9.lib" )
#pragma comment ( lib, "d3dx9.lib")
#pragma comment(lib,"winmm.lib")

using namespace  std;
//#include <boost/smart_ptr.hpp>
//using namespace boost;
//using namespace  std::tr1;
#ifndef SafeDelete
#define SafeDelete(p)       { if(p) { delete (p);     (p)=NULL; } }
#endif

#ifndef SafeDeleteArray
#define SafeDeleteArray(p)  { if(p) { delete[] (p);   (p)=NULL; } }
#endif

#ifndef SafeRelease
#define SafeRelease(p)      { if(p) { (p)->Release(); (p)=NULL; } }
#endif

typedef void(*CopyFrameProc)(const BYTE *pSourceData, BYTE *pY, BYTE *pUV, size_t surfaceHeight, size_t imageHeight, size_t pitch);
extern CopyFrameProc CopyFrameNV12;
//extern CopyFrameProc CopyFrameYUV420P;

struct  D3DLineArray
{
	D3DXVECTOR2* pLineArray;
	float		 fWidth;
	D3DCOLOR	 nColor;
	int			 nCount;
	int			 nIndex;
	bool		 bConvert;		// 是否已经执行坐标转换
	D3DLineArray()
	{
		ZeroMemory(this, sizeof(D3DLineArray));
	}
	~D3DLineArray()
	{
		SafeDeleteArray(pLineArray);
	}
};
typedef shared_ptr<D3DLineArray> D3DLineArrayPtr;

struct LineArrayFinder
{
	long		nIndex;
	LineArrayFinder(long nInputIndex)
		:nIndex(nInputIndex)
	{}
	bool operator()(D3DLineArrayPtr pLine)
	{
		return ((long)pLine.get() == nIndex);
	}
};

#define WM_RENDERFRAME		WM_USER + 1024		// 帧渲染消息	WPARAM为CDxSurface指针,LPARAM为一个指向DxSurfaceRenderInfo结构的指针,
#define	WM_INITDXSURFACE	WM_USER + 1025		// DXSurface初始化消息	WPARAM为CDxSurface指针，LPARAM为DxSurfaceInitInfo结构的指针
struct DxSurfaceInitInfo
{
	int		nSize;
	HWND	hPresentWnd;
	int		nFrameWidth;
	int		nFrameHeight;
	BOOL	bWindowed;
	D3DFORMAT	nD3DFormat;
};
struct DxSurfaceRenderInfo
{
	int			nSize;
	HWND		hPresentWnd;
	AVFrame		*pAvFrame;
};
class CDxSurface;
class CCriticalSectionProxy1;
typedef map<HWND,CDxSurface*> WndSurfaceMap;

typedef void (CALLBACK *ExternDrawProc)(HWND hWnd, HDC hDc, RECT rt, void *pUserPtr);

enum Coordinte
{
	Coordinte_Video,
	Coordinte_Wnd
};

enum GraphicQulityParameter
{
	GQ_SINC = -3,		//30		相对于上一算法，细节要清晰一些。
	GQ_SPLINE = -2,		//47		和上一个算法，我看不出区别。
	GQ_LANCZOS = -1,	//70		相对于上一算法，要平滑(也可以说是模糊)一点点，几乎无区别。
	GQ_BICUBIC = 0,		//80		感觉差不多，比上上算法边缘要平滑，比上一算法要锐利。	
	GQ_GAUSS,			//80		相对于上一算法，要平滑(也可以说是模糊)一些。
	GQ_BICUBLIN,		//87		同上。
	GQ_X,				//91		与上一图像，我看不出区别。
	GQ_BILINEAR,		//95		感觉也很不错，比上一个算法边缘平滑一些。
	GQ_AREA,			//116		与上上算法，我看不出区别。
	GQ_FAST_BILINEAR,	//228		图像无明显失真，感觉效果很不错
	GQ_POINT			//427		细节比较锐利，图像效果比上图略差一点点。
};

//#define _TraceMemory
#define TraceTimeout		2
#if defined(_DEBUG) && defined(_TraceFunction)

#define TraceFunction()			CTraceFunction Tx(__FUNCTION__);
#define TraceFunction1(szText)	CTraceFunction Tx(__FUNCTION__,true,szText);
#else 
#define TraceFunction()	
#endif

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <windows.h>
#include <psapi.h>
#pragma comment(lib,"psapi.lib")

#ifdef _DEBUG
/// @brief 跟踪函数执行过程的一些信息的类,主要用于调试
class CTraceFunction
{
	explicit CTraceFunction(){};
public:
	CTraceFunction(CHAR *szFunction, bool bDeconstructOut = true,CHAR *szTxt = nullptr)
	{
		ZeroMemory(this, sizeof(CTraceFunction));
		m_dfTimeIn = GetExactTime();
		m_bDeconstructOut = bDeconstructOut;
		HANDLE handle = GetCurrentProcess();
		PROCESS_MEMORY_COUNTERS pmc;
		GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));
		CloseHandle(handle);
		m_nMemoryCount = pmc.WorkingSetSize / 1024;
		strcpy_s(m_szFunction, 256, szFunction);
		if (szTxt)
			strcpy_s(m_szText, 1024, szTxt);
		CHAR szText[1024] = { 0 };
		sprintf_s(szText, 1024, "%s\t_IN_ %s \tMemory = %d KB.\n",__FUNCTION__, szFunction, m_nMemoryCount);
		OutputDebugStringA(szText);
	}
	~CTraceFunction()
	{
		if (m_bDeconstructOut)
		{
			CHAR szText[4096] = { 0 };
			HANDLE handle = GetCurrentProcess();
			PROCESS_MEMORY_COUNTERS pmc;
			GetProcessMemoryInfo(handle, &pmc, sizeof(pmc));
			CloseHandle(handle);
			m_nMemoryCount = pmc.WorkingSetSize / 1024;
			if (TimeSpanEx(m_dfTimeIn) > TraceTimeout/1000.0f)
			{
				if (strlen(m_szText) ==0)
					sprintf_s(szText, 4096, "%s\t_OUT_ %s \tMemory = %d KB\tTimeSpan = %.3f.\n",__FUNCTION__, m_szFunction, m_nMemoryCount,TimeSpanEx(m_dfTimeIn));
				else
					sprintf_s(szText, 4096, "%s\t_OUT_ %s %s\tMemory = %d KB\tTimeSpan = %.3f.\n", __FUNCTION__, m_szFunction,m_szText, m_nMemoryCount, TimeSpanEx(m_dfTimeIn));
				OutputDebugStringA(szText);
			}
		}
	}
private:
	INT		m_nMemoryCount;
	double	m_dfTimeIn;
	bool	m_bDeconstructOut;
	CHAR	m_szFile[MAX_PATH];
	CHAR	m_szText[1024];
	CHAR	m_szFunction[256];
};
#endif

struct LineTime
{
	CHAR szFile[256];
	int nLine;
	DWORD nTime;
	LineTime(char *pszFile,int nFileLine)
	{
		nTime = timeGetTime();
		strcpy(szFile, pszFile);
		nLine = nFileLine;
	}
};
#ifdef _LineTime
#define SaveRunTime()				LineSave.SaveLineTime(__FILE__,__LINE__);
#define DeclareRunTime(nTimeout)	CLineRunTime LineSave(nTimeout,__FUNCTION__);LineSave.SaveLineTime(__FILE__,__LINE__);
#else
#define SaveRunTime()
#define DeclareRunTime(nTimeout)			
#endif

#include <vector>
using namespace  std;
/// @brief 追踪每一行代码运行的时间，用于追查代码中哪一段代码执行时间超出预期
class CLineRunTime
{
	DWORD m_nTimeout;
	char *m_pszName;
	shared_ptr<char> m_pszNamePtr;
public:
	CLineRunTime(int nTimeout,char *szName = nullptr,CRunlogA *plog = nullptr)
	{
		if (szName)
		{
			m_pszName = new char[256];
			strcpy(m_pszName, szName);
		}
		else
			m_pszName = nullptr;
		m_pszNamePtr = shared_ptr<char>(m_pszName);
		m_nTimeout = nTimeout;
		pRunlog = plog;
	}
	
	~CLineRunTime()
	{
		DWORD dwTotalSpan = 0;
		int nSize = pTimeArray.size();
		if (nSize < 1)
			return;
		else 
			dwTotalSpan = timeGetTime() - pTimeArray[0]->nTime;
		if (dwTotalSpan >= m_nTimeout)
		{
			if (!m_pszName)
				OutputMsg("%s @File:%s line %d Total Runtime span = %d.\n", __FUNCTION__, pTimeArray[0]->szFile, pTimeArray[0]->nLine, dwTotalSpan);
			else
				OutputMsg("%s LineTime:%s line %d Total Runtime span = %d.\n", m_pszName, pTimeArray[0]->szFile, pTimeArray[0]->nLine, dwTotalSpan);
		}
		for (int i = 1; i < nSize;i ++)
		{
			DWORD dwSpan = pTimeArray[i]->nTime - pTimeArray[i - 1]->nTime;
			//if (dwSpan >= m_nTimeout)
			{
				if (!m_pszName)
					OutputMsg("%s @File:%s line %d Runtime span = %d.\n", __FUNCTION__, pTimeArray[i]->szFile, pTimeArray[i]->nLine, dwSpan);
				else
					OutputMsg("%s LineTime:%s line %d Runtime span = %d.\n", m_pszName, pTimeArray[i]->szFile, pTimeArray[i]->nLine, dwSpan);
			}
		}
		
	}
	void SaveLineTime(char *szFile, int nLine)
	{
		shared_ptr<LineTime> pLineTime = make_shared<LineTime>(szFile, nLine);
		pTimeArray.push_back(pLineTime);
	}
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
#ifdef _DEBUG
		OutputDebugStringA(szBuffer);
#endif
		if (pRunlog)
			pRunlog->Runlog(szBuffer);
		va_end(args);
	}
public:
	vector<shared_ptr<LineTime>> pTimeArray;
	CRunlogA *pRunlog /*= nullptr*/;
};

/// @biref 把FFMPEG像素转换为D3DFomrat像素
struct PixelConvert
{
private:
	explicit PixelConvert()
	{
	}
	SwsContext	*pConvertCtx;	
	GraphicQulityParameter	nGQP;
	int		nScaleFlag;
// 	int		nSrcImageWidth,nSrcImageHeight;
 	AVPixelFormat nDstAvFormat;	
public:
	int nImageSize;
	byte *pImage ;
	byte *pImageYUV = nullptr;
	AVFrame *pFrameNew;
	AVFrame *pSrcFrame;
	AVFrame *pFrameFromDXVA = nullptr;
	AVCodecID nCodecID = AV_CODEC_ID_H264;
	int nAlignedWidth = 0;
	int nAlignedHeight = 0;
	void CopyFrameNV12toYV12(AVFrame *pFrameYV12, AVFrame *pAvFrameDXVA)
	{
		if (pAvFrameDXVA->format != AV_PIX_FMT_DXVA2_VLD)
			return;	
		IDirect3DSurface9* pSurface = (IDirect3DSurface9 *)pAvFrameDXVA->data[3];
		D3DLOCKED_RECT lRect;
		D3DSURFACE_DESC SurfaceDesc;
		pSurface->GetDesc(&SurfaceDesc);
		HRESULT hr = pSurface->LockRect(&lRect, nullptr, D3DLOCK_READONLY);
		if (FAILED(hr))
		{
			DxTraceMsg("%s IDirect3DSurface9::LockRect failed:hr = %08.\n", __FUNCTION__, hr);
			return;
		}

		byte *pSrcY = (byte *)lRect.pBits;
		int nStrideY = lRect.Pitch;
		int nWidth = SurfaceDesc.Width;
		int nHeight = SurfaceDesc.Height;

		int nPictureSize = lRect.Pitch*SurfaceDesc.Height;
		byte *pNV12 = pFrameYV12->data[0];
		byte *pDst = pNV12;
		byte *pSrc = (byte *)lRect.pBits;
		for (int i = 0; i < nAlignedHeight; i++)
		{
			memcpy(pDst, pSrc, nAlignedWidth);
			pDst += nAlignedWidth;
			pSrc += lRect.Pitch;
		}
		
		UINT heithtUV = SurfaceDesc.Height >> 1;
		UINT widthUV = nAlignedWidth>>1;
		byte *pSrcUV = (byte *)lRect.pBits + nPictureSize;
		byte* dstV = pFrameYV12->data[1];
		byte* dstU = pFrameYV12->data[2];
		// ¸´ÖÆVU·ÖÁ¿
		int nOffsetSrc = 0;
		int nOffsetDst = 0;
		for (int i = 0; i < heithtUV; i++)
		{
			for (int j = 0; j < widthUV; j++)
			{
				dstV[nOffsetDst + j] = pSrcUV[nOffsetSrc + 2 * j];
				dstU[nOffsetDst + j] = pSrcUV[nOffsetSrc + 2 * j + 1];
			}
			nOffsetSrc += lRect.Pitch;
			nOffsetDst += (nAlignedWidth/2);
		}
		pSurface->UnlockRect();
		//pSrcFrame->format = AV_PIX_FMT_YUV420P;
	}
	//ConvertInfo(AVFrame *pSrcFrame,AVPixelFormat DestPixfmt = AV_PIX_FMT_YUV420P,GraphicQulityParameter nGQ = GQ_FAST_BILINEAR)
	PixelConvert(AVFrame *pSrcFrame,D3DFORMAT nDestD3dFmt = (D3DFORMAT)MAKEFOURCC('Y', 'V', '1', '2'),GraphicQulityParameter nGQ = GQ_BICUBIC)
	{
		ZeroMemory(this,sizeof(PixelConvert));
		switch(nDestD3dFmt)
		{//Direct3DSurface::GetDC() only supports D3DFMT_R5G6B5, D3DFMT_X1R5G5B5, D3DFMT_A1R5G5B5, D3DFMT_R8G8B8, D3DFMT_X8R8G8B8, and D3DFMT_A8R8G8B8.
		case (D3DFORMAT)MAKEFOURCC('Y', 'V', '1', '2'):
		default:
			nDstAvFormat = AV_PIX_FMT_YUV420P;
			break;
		case D3DFMT_R5G6B5:
			nDstAvFormat = AV_PIX_FMT_RGB565;	// 亦有可能是、AV_PIX_FMT_BGR565LE、AV_PIX_FMT_BGR565BE
			break;
		case D3DFMT_X1R5G5B5:
		case D3DFMT_A1R5G5B5:
			nDstAvFormat = AV_PIX_FMT_RGB555;	// 亦有可能是AV_PIX_FMT_BGR555BE
			break;
		case D3DFMT_R8G8B8:
			nDstAvFormat = AV_PIX_FMT_BGR24;
			break;
		case D3DFMT_X8R8G8B8:
			nDstAvFormat = AV_PIX_FMT_BGR0;
			break;
		case D3DFMT_A8R8G8B8:
			nDstAvFormat = AV_PIX_FMT_BGRA;
			break;
		}
		
		pFrameNew = av_frame_alloc();

		int align = 16;

		// MPEG-2 needs higher alignment on Intel cards, and it doesn't seem to harm anything to do it for all cards.
		if (nCodecID == AV_CODEC_ID_MPEG2VIDEO)
			align <<= 1;
		else if (nCodecID == AV_CODEC_ID_HEVC)
			align = 32;

		nAlignedWidth = pSrcFrame->width;
		nAlignedHeight = pSrcFrame->height;
		if (pSrcFrame->format == AV_PIX_FMT_DXVA2_VLD)
		{
			nAlignedWidth = FFALIGN(pSrcFrame->width, align);
			nAlignedHeight = FFALIGN(pSrcFrame->height, align);
		}
		
		nImageSize	 = av_image_get_buffer_size(nDstAvFormat, nAlignedWidth,nAlignedHeight,16); 
		if (nImageSize < 0)
		{
			char szAvError[256] = {0};
			av_strerror(nImageSize, szAvError, 1024);
			DxTraceMsg("%s av_image_get_buffer_size failed:%s.\n",__FUNCTION__,szAvError);
			assert(false);
		}
		pImage = (byte *)_aligned_malloc(nImageSize, 16);
		if (!pImage)
		{
			DxTraceMsg("%s Alloc memory failed @%d.\n", __FUNCTION__,  __LINE__);
			assert(false);
			return;
		}
		DxTraceMsg("%s Image size = %d.\n",__FUNCTION__,nImageSize);
		// 把显示图像与YUV帧关联
		av_image_fill_arrays(pFrameNew->data,pFrameNew->linesize, pImage, nDstAvFormat, nAlignedWidth,nAlignedHeight,16);
		pFrameNew->width	 = nAlignedWidth;
		pFrameNew->height	 = nAlignedHeight;
		pFrameNew->format	 = nDstAvFormat;
		
		if (pSrcFrame->format == AV_PIX_FMT_DXVA2_VLD)
		{
			if (!pFrameFromDXVA)
			{
				pFrameFromDXVA = av_frame_alloc();
				int nYUVSize = av_image_get_buffer_size(AV_PIX_FMT_YUV420P, nAlignedWidth , nAlignedHeight, 16);
				pImageYUV = (byte *)_aligned_malloc(nYUVSize, 16);
				if (!pImageYUV)
				{
					DxTraceMsg("%s Alloc memory failed @%d.\n", __FUNCTION__, __LINE__);
					assert(false);
					return;
				}
				av_image_fill_arrays(pFrameFromDXVA->data, pFrameFromDXVA->linesize, pImageYUV, AV_PIX_FMT_YUV420P, nAlignedWidth, nAlignedHeight, 16);
				pFrameFromDXVA->width = nAlignedWidth;
				pFrameFromDXVA->height = nAlignedHeight;
				pFrameFromDXVA->format = AV_PIX_FMT_YUV420P;
			}
			CopyFrameNV12toYV12(pFrameFromDXVA,pSrcFrame);
			this->pSrcFrame = pFrameFromDXVA;
		}
		else
		{
			this->pSrcFrame = pSrcFrame;
		}
		nGQP = nGQ;
		switch(nGQP)
		{
		default:
		case GQ_BICUBIC:
			nScaleFlag = SWS_BICUBIC;
			break;
		case GQ_SINC:
			nScaleFlag = SWS_SINC;
			break;
		case GQ_SPLINE:
			nScaleFlag = SWS_SPLINE;
			break;
		case GQ_LANCZOS:
			nScaleFlag = SWS_LANCZOS;
			break;			
		case GQ_GAUSS:
			nScaleFlag = SWS_GAUSS;
			break;
		case GQ_BICUBLIN:
			nScaleFlag = SWS_BICUBLIN;
			break;
		case GQ_X:
			nScaleFlag = SWS_X;
			break;
		case GQ_BILINEAR:
			nScaleFlag = SWS_BILINEAR;
			break;
		case GQ_AREA:
			nScaleFlag = SWS_AREA;
			break;
		case GQ_FAST_BILINEAR:
			nScaleFlag = SWS_FAST_BILINEAR;
			break;
		case GQ_POINT:
			nScaleFlag = SWS_POINT;
			break;
		}
		
// 		nDstAvFormat = nDstFormat;
// 		nSrcImageWidth = nImageWidth;
// 		nSrcImageHeight = nImageHeight;
		pConvertCtx = sws_getContext(nAlignedWidth,				// src Width
									nAlignedHeight,				// src Height
									(AVPixelFormat)this->pSrcFrame->format,	// src format
									nAlignedWidth,				// dst Width
									nAlignedHeight,				// dst Height
									nDstAvFormat,				// dst format
									nScaleFlag, 
									NULL, 
									NULL, 
									NULL);

		/*		
		sws_scale转换函数各种算法转换(缩小)效率和画质对比情况(详情请参考:http://blog.csdn.net/leixiaohua1020/article/details/12029505)
		算法				帧率	图像主观感受
		SWS_FAST_BILINEAR	228		图像无明显失真，感觉效果很不错。
		SWS_BILINEAR		95		感觉也很不错，比上一个算法边缘平滑一些。
		SWS_BICUBIC			80		感觉差不多，比上上算法边缘要平滑，比上一算法要锐利。
		SWS_X				91		与上一图像，我看不出区别。
		SWS_POINT			427		细节比较锐利，图像效果比上图略差一点点。
		SWS_AREA			116		与上上算法，我看不出区别。
		SWS_BICUBLIN		87		同上。
		SWS_GAUSS			80		相对于上一算法，要平滑(也可以说是模糊)一些。
		SWS_SINC			30		相对于上一算法，细节要清晰一些。
		SWS_LANCZOS			70		相对于上一算法，要平滑(也可以说是模糊)一点点，几乎无区别。
		SWS_SPLINE			47		和上一个算法，我看不出区别。

		sws_scale转换函数各种算法转换(放大)效率和画质对比情况
		算法				帧率	图像主观感受
		SWS_FAST_BILINEAR	103		图像无明显失真，感觉效果很不错。
		SWS_BILINEAR		100		和上图看不出区别。
		SWS_BICUBIC			78		相对上图，感觉细节清晰一点点。
		SWS_X				106		与上上图无区别。
		SWS_POINT			112		边缘有明显锯齿。
		SWS_AREA			114		边缘有不明显锯齿。
		SWS_BICUBLIN		95		与上上上图几乎无区别。
		SWS_GAUSS			86		比上图边缘略微清楚一点。
		SWS_SINC			20		与上上图无区别。
		SWS_LANCZOS			64		与上图无区别。
		SWS_SPLINE			40		与上图无区别。
		结论和建议:
			如果对图像的缩放，要追求高效，比如说是视频图像的处理，在不明确是放大还是缩小时，
		直接使用SWS_FAST_BILINEAR算法即可。如果明确是要缩小并显示，建议使用Point算法，如果
		是明确要放大并显示，其实使用CImage的Strech更高效。
			当然，如果不计速度追求画面质量。在上面的算法中，选择帧率最低的那个即可，画面效
			果一般是最好的。
		*/
	}
	~PixelConvert()
	{
		if (pImage)
		{
			//delete[]pImage;
			_aligned_free(pImage);
			pImage = nullptr;
		}
		if (pImageYUV)
		{
			_aligned_free(pImageYUV);
			pImageYUV = nullptr;
		}
		if (pFrameFromDXVA)
		{
			av_free(pFrameFromDXVA);
			pFrameFromDXVA = nullptr;
		}
			
		if (pFrameNew)
		{

			av_free(pFrameNew);
			pFrameNew = nullptr;
		}
		sws_freeContext(pConvertCtx);
		pConvertCtx = nullptr;
		DxTraceMsg("%s FreeImage size %d.\n",__FUNCTION__,nImageSize);
	}
	// 进行像素转换
	int inline ConvertPixel(AVFrame *pInputFrame = nullptr, GraphicQulityParameter nGQ = GQ_BICUBIC)
	{
		TraceFunction();
		if (pInputFrame)
		{
			if (pInputFrame->format == AV_PIX_FMT_DXVA2_VLD && pFrameFromDXVA)
			{
				CopyFrameNV12toYV12(pFrameFromDXVA, pInputFrame);
				this->pSrcFrame = pFrameFromDXVA;
			}
			else
				this->pSrcFrame = pInputFrame;
		}
		if (!pImage)
			return -1;
		//DxTrace("@File:%s %s pSrcFrame(%d,%d)->LineSize(%d,%d,%d).\n", __FILE__, __FUNCTION__, pSrcFrame->width, pSrcFrame->height, pSrcFrame->linesize[0], pSrcFrame->linesize[1], pSrcFrame->linesize[2]);
		if (nGQP != nGQ)
		{
			// 转换算法调整
			sws_freeContext(pConvertCtx);
			pConvertCtx = nullptr;
			nGQP = nGQ;
			switch(nGQP)
			{
			default:
			case GQ_BICUBIC:
				nScaleFlag = SWS_BICUBIC;
				break;
			case GQ_SINC:
				nScaleFlag = SWS_SINC;
				break;
			case GQ_SPLINE:
				nScaleFlag = SWS_SPLINE;
				break;
			case GQ_LANCZOS:
				nScaleFlag = SWS_LANCZOS;
				break;			
			case GQ_GAUSS:
				nScaleFlag = SWS_GAUSS;
				break;
			case GQ_BICUBLIN:
				nScaleFlag = SWS_BICUBLIN;
				break;
			case GQ_X:
				nScaleFlag = SWS_X;
				break;
			case GQ_BILINEAR:
				nScaleFlag = SWS_BILINEAR;
				break;
			case GQ_AREA:
				nScaleFlag = SWS_AREA;
				break;
			case GQ_FAST_BILINEAR:
				nScaleFlag = SWS_FAST_BILINEAR;
				break;
			case GQ_POINT:
				nScaleFlag = SWS_POINT;
				break;
			}
			pConvertCtx = sws_getContext(nAlignedWidth,					// src Width
										nAlignedHeight,					// src Height
										(AVPixelFormat)pSrcFrame->format,	// src format
										nAlignedWidth,					// dst Width
										nAlignedHeight,					// dst Height
										nDstAvFormat,						// dst format
										nScaleFlag, 
										nullptr,
										nullptr,
										nullptr);
		}
		return sws_scale(pConvertCtx,
						(const byte * const *)pSrcFrame->data,
						pSrcFrame->linesize,
						0,
						nAlignedHeight,
						pFrameNew->data,
						pFrameNew->linesize);

	}
	inline AVPixelFormat GetDestPixelFormat()
	{
		return nDstAvFormat;
	}
};

typedef IDirect3D9* WINAPI pDirect3DCreate9(UINT);
typedef HRESULT WINAPI pDirect3DCreate9Ex(UINT, IDirect3D9Ex**);

// 顶点结构
struct PolygonVertex
{
	float x, y, z, rhw;
	unsigned long color;
	PolygonVertex()
	{
		ZeroMemory(this, sizeof(PolygonVertex));
	}
};

// Our custom FVF, which describes our custom vertex structure.
#define D3DFVF_VERTEX (D3DFVF_XYZRHW | D3DFVF_DIFFUSE)

struct DxPolygon
{
	LPDIRECT3DVERTEXBUFFER9 VertexBuff;
	LPDIRECT3DINDEXBUFFER9 IndexBuff;
	int		nVertexCount;
	int		nTriangleCount;
	DxPolygon()
	{

	}
	~DxPolygon()
	{
		SafeRelease(VertexBuff);
		SafeRelease(IndexBuff);
	}
};

typedef shared_ptr<DxPolygon>  DxPolygonPtr;

struct D3DADAPTER_IDENTIFIER9Ex :public D3DADAPTER_IDENTIFIER9
{
	char szMonitorArray[10][32];
	int nMonitorCount;
};
class CD3D9Helper
{
public:
	HMODULE		 m_hD3D9 ;
	pDirect3DCreate9*	 m_pDirect3DCreate9;
	pDirect3DCreate9Ex*	 m_pDirect3DCreate9Ex;
	IDirect3D9 *  m_pDirect3D9;
	IDirect3D9Ex* m_pDirect3D9Ex;	/* = NULL*/;
	D3DADAPTER_IDENTIFIER9Ex m_AdapterArray[16];
	int			  m_nAdapterCount;
	CD3D9Helper()
	{
		ZeroMemory(this, sizeof(CD3D9Helper));		
		m_hD3D9 = ::LoadLibraryA("d3d9.dll");
		if (!m_hD3D9)
		{
			MessageBox(nullptr, _T("CD3D9Helper"), _T("Can't load d3d9.dll."), MB_OK | MB_ICONSTOP);
			return;
		}
		m_pDirect3DCreate9 = (pDirect3DCreate9*)GetProcAddress(m_hD3D9, "Direct3DCreate9");
		if (m_pDirect3DCreate9 == NULL)
		{
			DxTraceMsg("%s Can't locate the Procedure \"Direct3DCreate9\".\n", __FUNCTION__);
			assert(false);
			return;
		}
		m_pDirect3D9 = m_pDirect3DCreate9(D3D_SDK_VERSION);
		if (!m_pDirect3D9)
		{
			DxTraceMsg("%s Direct3DCreate9 failed.\n", __FUNCTION__);
			assert(false);
		}

		m_pDirect3DCreate9Ex = (pDirect3DCreate9Ex*)GetProcAddress(m_hD3D9, "Direct3DCreate9Ex");
		if (!m_pDirect3DCreate9Ex)
		{
			DxTraceMsg("%s Can't locate the Procedure \"Direct3DCreate9Ex\".\n", __FUNCTION__);
			assert(false);
			return;
		}
		HRESULT hr = m_pDirect3DCreate9Ex(D3D_SDK_VERSION, &m_pDirect3D9Ex);
		if (!m_pDirect3D9Ex)
		{
			DxTraceMsg("%s Direct3DCreate9Ex failed.\n", __FUNCTION__);
			assert(false);
		}
		//BOOL bResult = VirtualProtectEx(GetCurrentProcess(), m_pDirect3D9, sizeof(IDirect3D9), PAGE_EXECUTE_READ);
		//bResult = VirtualProtectEx(GetCurrentProcess(), m_pDirect3D9Ex, sizeof(IDirect3D9Ex), PAGE_EXECUTE_READ);
		UpdateAdapterInfo();
	}
	bool UpdateAdapterInfo()
	{
		if (m_pDirect3D9Ex)
		{
			ZeroMemory(m_AdapterArray, sizeof(m_AdapterArray));
			D3DADAPTER_IDENTIFIER9 D3DAdapter;
			DWORD dwTNow = timeGetTime();
			int nD3dAdapterCount = m_pDirect3D9Ex->GetAdapterCount();
			DWORD dwTimeSpan = MMTimeSpan(dwTNow);
			m_nAdapterCount = 0;
			TraceMsgA(_T("%s AdapterCount = %d.\r\n"), __FUNCTION__, nD3dAdapterCount);
			// LUID AdapterLUIDArray[10] = { 0 };
			// GUID AdapterGUIDArray[10] = { 0 };
			for (DWORD i = 0; i < nD3dAdapterCount; i++)
			{
				HMONITOR hMonitor = m_pDirect3D9Ex->GetAdapterMonitor(i);
				TraceMsg(_T("%s hMonitor[%d] = %p\n"), __FUNCTION__, i, hMonitor);
				ZeroMemory(&D3DAdapter, sizeof(D3DAdapter));
				if (m_pDirect3D9Ex->GetAdapterIdentifier(i/*D3DADAPTER_DEFAULT*/, 
														 0/*Reserved for future use*/, 
														 (D3DADAPTER_IDENTIFIER9 *)&D3DAdapter) != D3D_OK)
					return false;
				bool bFound = false;
				/*LUID luid;
				HRESULT hr = m_pDirect3D9Ex->GetAdapterLUID(i, &luid);				
				for (int k = 0; k < m_nAdapterCount; k++)
				{
					if (memcmp(&AdapterLUIDArray[k], &luid, sizeof(LUID)) == 0)
					{
						bFound = true;
						break;
					}
				}*/
				int nIndex = -1;
				for (int k = 0; k < m_nAdapterCount; k++)
				{
					if (memcmp(&m_AdapterArray[k].DeviceIdentifier, &D3DAdapter.DeviceIdentifier, sizeof(GUID)) == 0)
					{
						bFound = true;
						nIndex = k;
						break;
					}
				}
				if (bFound)
				{
					strcpy_s(m_AdapterArray[nIndex].szMonitorArray[m_AdapterArray[nIndex].nMonitorCount], 32, D3DAdapter.DeviceName);
					m_AdapterArray[nIndex].nMonitorCount++;
					continue;
				}
				//memcpy(&AdapterLUIDArray[m_nAdapterCount], &luid, sizeof(sizeof(LUID)));
				//memcpy(&AdapterGUIDArray[m_nAdapterCount], &D3DAdapter.DeviceIdentifier, sizeof(GUID));
				memcpy(&m_AdapterArray[m_nAdapterCount], &D3DAdapter, sizeof(D3DADAPTER_IDENTIFIER9));
				strcpy_s(m_AdapterArray[m_nAdapterCount].szMonitorArray[0], 32, D3DAdapter.DeviceName);
				m_AdapterArray[m_nAdapterCount].nMonitorCount++;
				TraceMsgA("[%d]Driver: %s.\n", m_nAdapterCount, D3DAdapter.Driver);
				TraceMsgA("[%d]Description: %s\n", m_nAdapterCount, D3DAdapter.Description);
				TraceMsgA("[%d]Device Name: %s\n", m_nAdapterCount, D3DAdapter.DeviceName);
				TraceMsgA("[%d]Vendor id:%4x\n", m_nAdapterCount, D3DAdapter.VendorId);
				TraceMsgA("[%d]Device id: %4x\n", m_nAdapterCount, D3DAdapter.DeviceId);
				TraceMsgA("[%d]Product: %x\n", m_nAdapterCount, HIWORD(D3DAdapter.DriverVersion.HighPart));
				TraceMsgA("[%d]Version:%x\n", m_nAdapterCount, LOWORD(D3DAdapter.DriverVersion.HighPart));
				TraceMsgA("[%d]SubVersion: %x\n", m_nAdapterCount, HIWORD(D3DAdapter.DriverVersion.LowPart));
				TraceMsgA("[%d]Build: %x %d.%d.%d.%d\n", m_nAdapterCount, LOWORD(D3DAdapter.DriverVersion.LowPart),
					HIWORD(D3DAdapter.DriverVersion.HighPart),
					LOWORD(D3DAdapter.DriverVersion.HighPart),
					HIWORD(D3DAdapter.DriverVersion.LowPart),
					LOWORD(D3DAdapter.DriverVersion.LowPart));
				
				TraceMsgA("[%d]SubSysId: %x\n, Revision: %x\n,GUID:%s\n, WHQLLevel:%d\n", m_nAdapterCount,
					D3DAdapter.SubSysId,
					D3DAdapter.Revision,
					StringFromGUIDA(&D3DAdapter.DeviceIdentifier),
					D3DAdapter.WHQLLevel);
				m_nAdapterCount++;
			}
			TraceMsgA("%s m_nAdapterCount = %d.\n",__FUNCTION__, m_nAdapterCount);
			return true;
		}
		else
			return false;
	}
	~CD3D9Helper()
	{
		//SafeRelease(m_pDirect3D9);
		//SafeRelease(m_pDirect3D9Ex);
		if (m_hD3D9)
		{
			FreeLibrary(m_hD3D9);
			m_hD3D9 = NULL;
		}
	}
};

struct OSDInfo
{
	OSDInfo(){};
	OSDInfo(WCHAR *szText, int nLength, RECT rtPostion, DWORD dwFormat, D3DCOLOR nColor)
	{
		assert(szText != nullptr);
		if (nLength <= 0)
			nLength = wcslen(szText);
		if ((nLength > 0) && szText)
		{
			pszText = new WCHAR[nLength + 1];
			wcsncpy_s(pszText, nLength + 1, szText, nLength);
		}
		nTextLength = nLength;
		this->rtPostion = rtPostion;
		this->dwFormat = dwFormat;
		this->nColor = nColor;
	}
	~OSDInfo()
	{
		delete[]pszText;
	}
	WCHAR *pszText;
	int nTextLength;
	RECT rtPostion;
	DWORD dwFormat;
	D3DCOLOR nColor;
};

typedef shared_ptr<OSDInfo> OSDInfoPtr;

// 注意:
/// @brief IDirect3DSurface9对象封类，用于创建和管理IDirect3DSurface9对象，可以显示多种象素格式的图像
/// @remark 使用CDxSurface对象显示图象时，必须在创建线程内显示图，否则当发生DirectX设备丢失时，无法重置DirectX的资源

extern CD3D9Helper    g_pD3D9Helper;
class CDxSurface
{
//protected:	
public:
	map<long,DxPolygonPtr>	m_mapPolygon;
	list<D3DLineArrayPtr>	m_listLine;
	map<long, map<long,OSDInfoPtr>> m_MapOSD;
	CCriticalSectionAgent   m_csMapOSD;
	long					m_nVtableAddr;		// 虚函数表地址，该变量地址位置虚函数表之后，仅用于类初始化，请匆移动该变量的位置
	D3DPRESENT_PARAMETERS	m_d3dpp;
	CRITICAL_SECTION		m_csRender;			// 渲染临界区
	CRITICAL_SECTION		m_csSnapShot;		// 截图临界区
	CCriticalSectionAgentPtr m_pCsListLine;
	CCriticalSectionAgentPtr m_pCsMapPolygon;
	bool					m_bD3DShared;		// IDirect3D9接口是否为共享 	
	/*HWND					m_hWnd;*/
	DWORD					m_dwExStyle;
	DWORD					m_dwStyle;
	bool					m_bFullScreen;		// 是否全屏,为true时，则进行全屏/窗口的切换,切换完成后,m_bSwitchScreen将被置为false
	WINDOWPLACEMENT			m_WndPlace;
	HMENU					m_hMenu;
	D3DPOOL					m_nCurD3DPool;
	IDirect3DSurface9		*m_pSurfaceRender/* = NULL*/;
	IDirect3DSurface9		*m_pRGBSurface/* = NULL*/;
	byte					*m_pRGBBuffer;
	UINT					m_nRGBBufferSize = 0;
	//				*m_pD3DXFont = nullptr;
	IDirect3DSurface9		*m_pSurfaceYUVCache/* = NULL*/;
	IDirect3DSurface9		*m_pSnapshotSurface;	/* = NULL*/;	//截图专用表面
	shared_ptr<PixelConvert> m_pVideoScale = nullptr;
	D3DFORMAT				m_nD3DFormat;
	UINT					m_nVideoWidth;
	UINT					m_nVideoHeight;
	UINT					m_nWndWidth;
	UINT					m_nWndHeight;
	bool					m_bInitialized;	
	Coordinte				m_nCordinateMode = Coordinte_Video;
	shared_ptr<PixelConvert>m_pPixelConvert;
	HANDLE					m_hYUVCacheReady;
	IDirect3D9				*m_pDirect3D9		/* = NULL*/;
	IDirect3DDevice9		*m_pDirect3DDevice	/*= NULL*/;	
	HANDLE					m_hEventSnapShot = nullptr;	// 截图请求事件
	WCHAR					*m_pszBackImageFileW;
	HWND					m_hBackImageWnd;
	bool					m_bSnapFlag;		// 截图事件标志,若该标志为TRUE，即使窗口被隐藏也会进行画面渲染
	// AVFrame*				m_pAVFrame;
	HANDLE					m_hEventFrameReady; // 解码数据已经就绪
	HANDLE					m_hEventFrameCopied;// 解码数据复制完毕
	CRITICAL_SECTION		m_csExternDraw;
	ExternDrawProc			m_pExternDraw;		// 外部绘制接口，提供外部接口，允许调用方自行绘制图像
	void*					m_pUserPtr;			// 外部调用者自定义指针
	void*					m_pUserPtrEx;			// 外部调用者自定义指针
	int						m_nFrameWidth = 0;
	int						m_nFrameHeight = 0;
	
	bool					m_bFullWnd;			// 是否填满窗口,为True时，则填充整个窗口客户区，否则只按视频比例显示	
	HWND					m_hFullScreenWindow;// 伪全屏窗口
	// 截图相关变量

	D3DXIMAGE_FILEFORMAT	m_D3DXIFF;			// 截图类型,默认为bmp格式
 	WCHAR					m_szSnapShotPath[MAX_PATH];
	bool					m_bEnableVsync;		// 启用垂直同步,默认为true,在需要显示的帧率大于显示器实际帧率时,需要禁用垂直同步

	bool					m_bVideoScaleFixed;	// 当m_bVideoScaleFixed为true,并且m_fWHScale = 0时,则使用图像原始比例,比例值为Width/Height
												// 当m_bVideoScaleFixed为true,并且m_fWHScale 大于0时,则使用dfWHScale提供的比例显示，图像可能会被拉伸变形
												// 当m_bVideoScaleFixed为false,m_fWHScale参数将被忽略,此时像将填满窗口客户区
	float					m_fWHScale;	
	WNDPROC					m_pOldWndProc;
	int						m_nDisplayAdapter;
	static WndSurfaceMap	m_WndSurfaceMap;
	static CCriticalSectionAgentPtr m_WndSurfaceMapcs;
	static	int				m_nObjectCount;
	static CCriticalSectionAgentPtr m_csObjectCount;
	bool					m_bWndSubclass;		// 是否子类化显示窗口,为ture时，则将显示窗口子类化,此时窗口消息会先被CDxSurface::WndProc优先处理,再由窗口函数处理
	pDirect3DCreate9*		m_pDirect3DCreate9;
	LPD3DXLINE              m_pD3DXLine = NULL; //Direct3D线对象  
	//D3DXVECTOR2*            m_pLineArray = NULL; //线段顶点 
	CHAR					m_szAdapterID[64];	///< 显卡的GUID
public:
		
// 	explicit CDxSurface(IDirect3D9 *pD3D9)
// 	{
// 		ZeroMemory(&m_nVtableAddr, sizeof(CDxSurface) - offsetof(CDxSurface,m_nVtableAddr));
// 		InitializeCriticalSection(&m_csRender);
// 		InitializeCriticalSection(&m_csSnapShot);
// 		InitializeCriticalSection(&m_csExternDraw);
// 		m_pCriListLine = make_shared<CCriticalSectionProxy>();
// 		m_nCordinateMode = Coordinte_Wnd;
// 		m_pCriMapPolygon = make_shared<CCriticalSectionProxy>();
// 		if (pD3D9)
// 		{
// 			m_bD3DShared = true;
// 			m_pDirect3D9 = pD3D9;
// 		}
// 	}
	CDxSurface()
	{
		// TraceFunction();
		// 借助于m_nVtableAddr变量，避开对虚函数表的初始化
		// 仅适用于微软的Visual C++编译器
		DeclareRunTime(5);
		ZeroMemory(&m_nVtableAddr, sizeof(CDxSurface) - offsetof(CDxSurface,m_nVtableAddr));
		m_csObjectCount->Lock();
		m_nObjectCount++;
		DxTraceMsg("%s CDxSurface Count = %d.\n", __FUNCTION__, m_nObjectCount);
		m_csObjectCount->Unlock();
		SaveRunTime();
		m_bEnableVsync = true;
		

		m_pDirect3DCreate9 = g_pD3D9Helper.m_pDirect3DCreate9;
		SaveRunTime();
		if (m_pDirect3DCreate9 == NULL)
		{
			DxTraceMsg("%s Can't locate the Procedure \"Direct3DCreate9\".\n",__FUNCTION__);
			assert(false);
			return;
		}
		m_pDirect3D9 = g_pD3D9Helper.m_pDirect3D9;
		if (!m_pDirect3D9)
		{
			DxTraceMsg("%s Direct3DCreate9 failed.\n",__FUNCTION__);
			assert(false);
		}

		SaveRunTime();
		m_nCordinateMode = Coordinte_Video;
		InitializeCriticalSection(&m_csRender);
		InitializeCriticalSection(&m_csSnapShot);
		InitializeCriticalSection(&m_csExternDraw);
		m_pCsListLine = make_shared<CCriticalSectionAgent>();
		m_pCsMapPolygon = make_shared<CCriticalSectionAgent>();
		m_hEventFrameReady	= CreateEvent(NULL,FALSE,FALSE,NULL);
		m_hEventFrameCopied = CreateEvent(NULL,FALSE,FALSE, NULL);
		m_hEventSnapShot	= CreateEvent(NULL,FALSE,FALSE,NULL);
		SaveRunTime();
	}

	virtual long CreateFontW(LOGFONTW *pLogFont)
	{
		assert(m_pDirect3DDevice != nullptr);
		D3DXFONT_DESCW FontDesc;
		FontDesc.Height = pLogFont->lfHeight;
		FontDesc.CharSet = pLogFont->lfCharSet;
		FontDesc.Width = pLogFont->lfWidth;
		FontDesc.Weight = pLogFont->lfWeight;
		FontDesc.Italic = pLogFont->lfItalic;
		FontDesc.MipLevels = 1;
		FontDesc.OutputPrecision = pLogFont->lfOutPrecision;
		FontDesc.PitchAndFamily = pLogFont->lfPitchAndFamily;
		FontDesc.Quality = pLogFont->lfQuality;
		wcscpy_s(FontDesc.FaceName, 32, pLogFont->lfFaceName);
		ID3DXFont *pFont = nullptr;
		if (FAILED(D3DXCreateFontIndirectW(m_pDirect3DDevice, &FontDesc, &pFont)))
		{
			assert(false);
			return 0;
		}
		map<long,OSDInfoPtr> vec;
		m_csMapOSD.Lock();
		m_MapOSD.insert(pair<long, map<long,OSDInfoPtr>>((long)pFont,vec));
		m_csMapOSD.Unlock();
		return (long)pFont;
	}
	void DestroyFont(long nFont)
	{
		if (nFont)
		{
			CAutoLock lock(m_csMapOSD.Get());
			auto itFind = m_MapOSD.find(nFont);
			if (itFind == m_MapOSD.end())
				return;
			ID3DXFont *pFont = (ID3DXFont *)nFont;
			SafeRelease(pFont);
			m_MapOSD.erase(itFind);
		}
	}

	long DrawTextA(long nFont, CHAR *szText, int nLength, RECT rtPostion, DWORD dwFormat, D3DCOLOR nColor)
	{
		assert(nFont != 0);
		if (!nFont)
			return 0;
		if (!szText || !nLength)
			return 0;
		int nNeedBuffSize = ::MultiByteToWideChar(CP_ACP, NULL, szText, nLength, NULL, 0);
		WCHAR *pTextW = new WCHAR[nNeedBuffSize + 1];
		shared_ptr<WCHAR> TextWPtr(pTextW);
		::MultiByteToWideChar(CP_ACP, 0, szText, nLength, pTextW, nNeedBuffSize);
		return DrawTextW(nFont, pTextW, nNeedBuffSize, rtPostion,dwFormat, nColor);
	}

	long DrawTextW(long nFont,WCHAR *szText, int nLength, RECT rtPostion,DWORD dwFormat, D3DCOLOR nColor)
	{
		assert(nFont != 0);
		if (!nFont)
			return 0;
		if (!szText || !nLength)
			return 0;
		CAutoLock lock(m_csMapOSD.Get());
		auto itFind = m_MapOSD.find(nFont);
		if (itFind == m_MapOSD.end())
			return 0;
		OSDInfoPtr OsdPtr = make_shared<OSDInfo>(szText, nLength, rtPostion, dwFormat, nColor);
		itFind->second.insert(pair<long, OSDInfoPtr>((long)OsdPtr.get(),OsdPtr));
		return (long)OsdPtr.get();
	}
	// nText is a handle ,not a real Text
	void RemoveText(long nFont,long nText)
	{
		assert(nFont != 0);
		if (!nFont)
			return ;
		if (!nText )
			return ;
		CAutoLock lock(m_csMapOSD.Get());
		auto itFind = m_MapOSD.find(nFont);
		if (itFind == m_MapOSD.end())
			return;
		auto itFind2 = itFind->second.find(nText);
		if (itFind2 == itFind->second.end())
			return;
		itFind->second.erase(itFind2);
	}
	virtual IDirect3DSurface9* LoadImage(WCHAR *szFileName)
	{
		// 以下代码加载背景图片
		IDirect3DSurface9 *pSurfaceBackImage = nullptr;
		D3DXIMAGE_INFO ImageInfo;
		if (szFileName)
		{
			HRESULT hr = D3DXGetImageInfoFromFileW(szFileName, &ImageInfo);
			if (SUCCEEDED(hr))
			{
				hr = m_pDirect3DDevice->CreateOffscreenPlainSurface(ImageInfo.Width,
					ImageInfo.Height,
					/*ImageInfo.Format*/
					D3DFMT_X8R8G8B8,
					m_nCurD3DPool,
					&pSurfaceBackImage,
					NULL);
				if (SUCCEEDED(hr))
				{
					hr = D3DXLoadSurfaceFromFileW(
						pSurfaceBackImage,					//目标表面
						NULL,								//目标调色板
						NULL,								//目标矩形,NULL为加载整个表面
						szFileName,							//文件
						NULL,								//源矩形,NULL为复制整个图片
						D3DX_DEFAULT,						//过滤
						D3DCOLOR_XRGB(0, 0, 0),				//透明色
						&ImageInfo							//源图像信息
						);
					if (SUCCEEDED(hr))
					{
						return pSurfaceBackImage;
					}
				}
			}
		}
		SafeRelease(pSurfaceBackImage);
		return nullptr;
	}
	int SetDisplayAdapter(int nAdapter = 0)
	{
		if (nAdapter >= 0)
		{
			m_nDisplayAdapter = nAdapter;
			return 0;
		}
		else
			return -1;
	}

	void ReleaseOffSurface()
	{
		if (m_pSurfaceRender)
			SafeRelease(m_pSurfaceRender);
	}
	
	virtual ~CDxSurface()
	{
		TraceFunction();
		//DetachWnd();
		m_csMapOSD.Lock();
		for (auto it = m_MapOSD.begin(); it != m_MapOSD.end();)
		{
			ID3DXFont *pFont = (ID3DXFont *)it->first;
			it->second.clear();
			SafeRelease(pFont);
			it = m_MapOSD.erase(it);
		}
		m_csMapOSD.Unlock();
		
		m_pCsListLine->Lock();
		m_listLine.clear();
		m_pCsListLine->Unlock();
		SafeRelease(m_pD3DXLine);
		DxCleanup();
//		SafeRelease(m_pDirect3D9);
// 		if (m_hD3D9)
// 		{
// 			FreeLibrary(m_hD3D9);
// 			m_hD3D9 = NULL;
// 		}
		DeleteCriticalSection(&m_csRender);
		DeleteCriticalSection(&m_csSnapShot);
		DeleteCriticalSection(&m_csExternDraw);
 		if (m_hEventFrameReady)
 		{
 			CloseHandle(m_hEventFrameReady);
 			m_hEventFrameReady = NULL;
 		}
		if (m_hEventSnapShot)
		{
			CloseHandle(m_hEventSnapShot);
			m_hEventSnapShot = NULL;
		}
		if (m_pszBackImageFileW)
		{
			delete[]m_pszBackImageFileW;
			m_pszBackImageFileW = nullptr;
		}
		if (m_pRGBBuffer)
			delete []m_pRGBBuffer;
	}

	// 禁用垂直同步,只有在初始化之前调用,才会有效
	// 默认情况下，垂直同步被启用, 在需要显示的帧率大于显示器实际帧率时,需要禁用垂直同步
	bool	DisableVsync()
	{
		if (!m_bInitialized)
		{
			m_bEnableVsync = false;
			return true;
		}
		else
			return false;
	}
	/// @设置外部DC回调函数
	/// @remark IDirect3DSurface9可以产生一个DC指针，通过回调函数把此DC指针传给用户，用户通过此DC可以在IDirect3DSurface9上作图
	/// 但这种方式效率极低，会明显拖慢视频显示的速度
	void SetExternDraw(void *pExternDrawProc,void *pUserPtr)
	{
		EnterCriticalSection(&m_csExternDraw);
		m_pExternDraw = (ExternDrawProc)pExternDrawProc;
		m_pUserPtr = pUserPtr;
		LeaveCriticalSection(&m_csExternDraw);
	}

	
	bool SetBackgroundPictureFile(LPCWSTR szPhotoFile, HWND hBackImageWnd)
	{
		if (!szPhotoFile /*|| !PathFileExistsW(szPhotoFile)*/)
			return false;
		int nLength = wcslen(szPhotoFile);
		m_pszBackImageFileW = new WCHAR[nLength + 1];
		wcscpy_s(m_pszBackImageFileW, nLength + 1, szPhotoFile);
		m_hBackImageWnd = hBackImageWnd;
		return true;
	}
	
	// 调用外部绘制函数
	virtual void ExternDrawCall(HWND hWnd, IDirect3DSurface9 * pBackSurface, RECT *pRect)
	{
		if (!m_pSurfaceRender)
			return;
		CAutoLock lock(&m_csExternDraw);
		if (m_pExternDraw)
		{
			D3DSURFACE_DESC Desc;
			if (FAILED(pBackSurface->GetDesc(&Desc)))
				return;
			switch (Desc.Format)
			{//Direct3DSurface::GetDC() only supports D3DFMT_R5G6B5, D3DFMT_X1R5G5B5, D3DFMT_A1R5G5B5, D3DFMT_R8G8B8, D3DFMT_X8R8G8B8, and D3DFMT_A8R8G8B8.
				// 外部图你绘制仅支持以下像素格式
			case D3DFMT_R5G6B5:
			case D3DFMT_X1R5G5B5:
			case D3DFMT_A1R5G5B5:
			case D3DFMT_R8G8B8:
			case D3DFMT_X8R8G8B8:
			case D3DFMT_A8R8G8B8:
				break;
			default:
			{
				DxTraceMsg("%s Unsupported Pixel format.\r\n", __FUNCTION__);
				assert(false);
				return;
			}
			}
			HDC hDc = NULL;
			HRESULT hr = pBackSurface->GetDC(&hDc);
			if (SUCCEEDED(hr))
			{
				RECT MotionRect;
				HWND hMotionWnd = m_d3dpp.hDeviceWindow;
				if (hWnd)
					hMotionWnd = hWnd;
				if (pRect)
					memcpy(&MotionRect, pRect, sizeof(RECT));
				else
				{
					MotionRect.left = 0;
					MotionRect.right = Desc.Width;
					MotionRect.top = 0;
					MotionRect.bottom = Desc.Height;
				}
				
				if (m_pExternDraw)
					m_pExternDraw(hMotionWnd, hDc, MotionRect, m_pUserPtr);
				pBackSurface->ReleaseDC(hDc);
			}
			else
			{
				DxTraceMsg("%s Get DC(Device Context) from DxSurface failed,Error=%08X.\r\n", __FUNCTION__,hr);
				assert(false);
			}
		}
	}

	// 添加一个多边形
	virtual long AddPolygon(POINT *pPtArray, int nCount, WORD *pInputIndexArray, D3DCOLOR nColor)
	{
		if (!m_pDirect3DDevice)
			return 0;
		PolygonVertex *pVertexArray = new PolygonVertex[nCount];
		for (int i = 0; i < nCount; i++)
		{
			pVertexArray[i].x = (float)pPtArray[i].x;
			pVertexArray[i].y = (float)pPtArray[i].y;
			pVertexArray[i].rhw = 1.0f;
			pVertexArray[i].color = nColor;
		}
		WORD *pIndexArray = new WORD[(nCount - 2) * 3];
		for (int i = 0; i < nCount - 2; i++)
		{
			pIndexArray[i * 3] = 0;
			pIndexArray[i * 3 + 1] = pInputIndexArray[i + 1];
			pIndexArray[i * 3 + 2] = pInputIndexArray[i + 2];
		}
		bool bSucceed = false;
		DxPolygonPtr pDxPolygon = make_shared<DxPolygon>();
		pDxPolygon->nVertexCount = nCount -1;
		pDxPolygon->nTriangleCount = nCount - 2;

		if (FAILED(m_pDirect3DDevice->CreateVertexBuffer(sizeof(PolygonVertex)*nCount, 0, D3DFVF_VERTEX, D3DPOOL_DEFAULT, &pDxPolygon->VertexBuff, NULL)))
			goto __Failure;

		// Fill the vertex buffer.
		void *ptr;
		if (FAILED(pDxPolygon->VertexBuff->Lock(0, sizeof(PolygonVertex)*nCount, (void**)&ptr, 0)))
			return false;

		memcpy(ptr, pVertexArray, sizeof(PolygonVertex)*nCount);
		pDxPolygon->VertexBuff->Unlock();

		INT nIndexSize = (nCount - 2) * 3 * sizeof(WORD);
		void *pIndex_ptr;
		if (FAILED(m_pDirect3DDevice->CreateIndexBuffer(nIndexSize, 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &pDxPolygon->IndexBuff, NULL)))
			goto __Failure;

		if (FAILED(pDxPolygon->IndexBuff->Lock(0, nIndexSize, (void**)&pIndex_ptr, 0)))
			goto __Failure;
		memcpy(pIndex_ptr, pIndexArray, nIndexSize);
		pDxPolygon->IndexBuff->Unlock();
		bSucceed = true;

	__Failure:
		SafeDeleteArray(pVertexArray);
		SafeDeleteArray(pIndexArray);
		if (bSucceed)
		{
			m_pCsMapPolygon->Lock();
			m_mapPolygon.insert(pair<long, DxPolygonPtr >((long)pDxPolygon.get(), pDxPolygon));
			m_pCsMapPolygon->Unlock();
			return (long)pDxPolygon.get();
		}
		else
			return 0;
	}
	// 添加一组线条坐标
	// 返回值为索条索引值，删除该线条时需要用到这个索引值
	// 添加失败时返回0
	virtual long AddD3DLineArray(POINT *pPointArray,int nCount,float fWidth,D3DCOLOR nColor)
	{
		if (!m_pD3DXLine)
		{
			// 创建Direct3D线对象  
			if (FAILED(D3DXCreateLine(m_pDirect3DDevice, &m_pD3DXLine)))
			{
				return 0;
			}
		}
		D3DLineArrayPtr pLineArray = make_shared<D3DLineArray>();
		pLineArray->fWidth = fWidth;
		pLineArray->nColor = nColor;
		pLineArray->nCount = nCount;
		pLineArray->pLineArray = new D3DXVECTOR2[nCount];
		for (int i = 0; i < nCount; i++)
		{
			pLineArray->pLineArray[i].x = (float)pPointArray[i].x;
			pLineArray->pLineArray[i].y = (float)pPointArray[i].y;
		}
		m_pCsListLine->Lock();
		m_listLine.push_back(pLineArray);
		m_pCsListLine->Unlock();
		return (long)pLineArray.get();
	}
	// 删除多边形
	void RemovePolygon(long nPolygonIndex)
	{
		m_pCsMapPolygon->Lock();
		auto itFind = m_mapPolygon.find(nPolygonIndex);
		if (itFind != m_mapPolygon.end())
		{
			m_mapPolygon.erase(itFind);
		}
		m_pCsMapPolygon->Unlock();
	}

	// 删除线段
	// nArrayIndex为AddD3DLineArray返回的索引值
	int  RemoveD3DLineArray(long nArrayIndex)
	{
		CAutoLock lock(m_pCsListLine->Get());
		auto it = find_if(m_listLine.begin(), m_listLine.end(),LineArrayFinder(nArrayIndex));
		if (it != m_listLine.end())
		{
			m_listLine.erase(it);
		}
		return m_listLine.size();
	}
		
	void SetCoordinateMode(Coordinte nCoordinate = Coordinte_Video)
	{
		m_nCordinateMode = nCoordinate;
	}
	virtual void ProcessD3DXDraw(HWND hWnd)
	{
		if (m_pD3DXLine && hWnd)
		{
			RECT rtWnd;
			GetWindowRect(hWnd, &rtWnd);
			UINT nWndWidth = RectWidth(rtWnd);
			UINT nWndHeight = RectHeight(rtWnd);
			if (nWndWidth > 0 &&
				nWndHeight > 0)
			{
				CAutoLock lock(m_pCsListLine->Get());
				if (m_listLine.size())
				{
					for (auto it = m_listLine.begin(); it != m_listLine.end(); it++)
					{
						m_pD3DXLine->SetWidth((*it)->fWidth);
						m_pD3DXLine->SetAntialias(TRUE);
						D3DXVECTOR2* pLineArray = new D3DXVECTOR2[(*it)->nCount];
						D3DXVECTOR2* pLineArraySrc = (*it)->pLineArray;
						if (m_nCordinateMode == Coordinte_Video || (*it)->bConvert)
						{
							for (int nIndex = 0; nIndex < (*it)->nCount; nIndex++)
							{
								pLineArray[nIndex].x = pLineArraySrc[nIndex].x;
								pLineArray[nIndex].y = pLineArraySrc[nIndex].y;
							}
						}
						else
						{
							for (int nIndex = 0; nIndex < (*it)->nCount; nIndex++)
							{
								pLineArraySrc[nIndex].x = pLineArraySrc[nIndex].x*m_nVideoWidth / nWndWidth;
								pLineArraySrc[nIndex].y = pLineArraySrc[nIndex].y*m_nVideoHeight / nWndHeight;
								pLineArray[nIndex].x = pLineArraySrc[nIndex].x;
								pLineArray[nIndex].y = pLineArraySrc[nIndex].y;
							}
							(*it)->bConvert = true;
						}
						m_pD3DXLine->Draw(pLineArray, (*it)->nCount, (*it)->nColor);
						delete[]pLineArray;
					}
				}
			}
		}
		m_csMapOSD.Lock();
		for (auto it = m_MapOSD.begin(); it != m_MapOSD.end(); it++)
		{
			ID3DXFont *pFont =(ID3DXFont*)it->first;
			map<long, OSDInfoPtr> &TextMap = it->second;
			for (auto it2 = TextMap.begin(); it2 != TextMap.end(); it2++)
			{
				OSDInfoPtr &TextPtr = it2->second;
				pFont->DrawTextW(nullptr, TextPtr->pszText, TextPtr->nTextLength, &TextPtr->rtPostion, TextPtr->dwFormat, TextPtr->nColor);
			}
		}
		m_csMapOSD.Unlock();
		
	}
	virtual bool ResetDevice()
	{
		TraceFunction();
		SafeRelease(m_pSurfaceRender);	
#ifdef _DEBUG
		HRESULT hr = m_pDirect3DDevice->Reset(&m_d3dpp);
		if (SUCCEEDED(hr))
		{
			DxTraceMsg("%s Direct3DDevice::Reset Succceed.\n",__FUNCTION__);
			return true;
		}
		else
		{
			DxTraceMsg("%s Direct3DDevice::Reset Failed,hr = %08X.\n",__FUNCTION__,hr);
			return false;
		}
#else
		return SUCCEEDED(m_pDirect3DDevice->Reset(&m_d3dpp));
#endif
	}
	/// @brief 取图像尺寸和象素格式 低32位的低16 bit为宽度，高16bit为高度，高32位为象素格式
	UINT64 GetVideoSizeAndFormat()
	{
		return MAKEUINT64(MAKELONG(m_nVideoWidth, m_nVideoHeight), m_nD3DFormat);
	}
	// @brief D3dDirect9Ex下，该成员不再有效
	virtual bool RestoreDevice()
	{// 恢复设备，即用原始参数重建资源
		TraceFunction();
		if (!m_pDirect3D9 || !m_pDirect3DDevice)
			return false;
#ifdef _DEBUG
		HRESULT hr = m_pDirect3DDevice->CreateOffscreenPlainSurface(m_nVideoWidth, m_nVideoHeight, m_nD3DFormat, D3DPOOL_DEFAULT, &m_pSurfaceRender, NULL);
		if (SUCCEEDED(hr))
		{
			DxTraceMsg("%s Direct3DDevice::CreateOffscreenPlainSurface Succeed.\n",__FUNCTION__);
			return true;
		}
		else
		{
			DxTraceMsg("%s Direct3DDevice::CreateOffscreenPlainSurface Failed,hr = %08X.\n",__FUNCTION__,hr);
			return false;
		}
#else
		return SUCCEEDED(m_pDirect3DDevice->CreateOffscreenPlainSurface(m_nVideoWidth, m_nVideoHeight, m_nD3DFormat, D3DPOOL_DEFAULT, &m_pSurfaceRender, NULL));
#endif
	}

#ifdef _DEBUG
	virtual void OutputDxPtr()
	{
		DxTraceMsg("%s m_pDirect3D9 = %p\tm_pDirect3DDevice = %p\tm_pDirect3DSurfaceRender = %p.\n", __FUNCTION__, m_pDirect3D9, m_pDirect3DDevice, m_pSurfaceRender);
	}
#endif
	virtual void DxCleanup()
	{
		if (!m_bD3DShared)
		{
			SafeRelease(m_pSurfaceRender);
		}
		
		SafeRelease(m_pRGBSurface);
		SafeRelease(m_pSnapshotSurface);
		SafeRelease(m_pDirect3DDevice);
	}
	// 发送截图请求，即置信截图事件
	/*void RequireSnapshot()
	{
		SetEvent(m_hEventSnapShot);
		m_bSnapFlag = true;
	}*/

	// 把解码帧pAvFrame中的图像传送到截图表面
	void TransferSnapShotSurface(AVFrame *pAvFrame)
	{
		if (m_pSnapshotSurface && WaitForSingleObject(m_hEventSnapShot, 0) == WAIT_OBJECT_0)						// 收到截图请求
		{
			//ResetEvent(m_hEventCreateSurface);
			// 不能使用StretchRect把显存表面复制到系统内存表面
			// hr = m_pDirect3DDevice->StretchRect(m_pDirect3DSurfaceRender, &srcrt, m_pSnapshotSurface, &srcrt, D3DTEXF_LINEAR);
			D3DLOCKED_RECT D3dRect;
			D3DSURFACE_DESC desc;
		
			// FFMPEG像素编码字节序与DirectX像素的字节序相反,因此
			// D3DFMT_A8R8G8B8(A8:R8:G8:B8)==>AV_PIX_FMT_BGRA(B8:G8:R8:A8)
//#if _MSC_VER >= 1600
			if (!m_pVideoScale)
				m_pVideoScale = make_shared<PixelConvert>(pAvFrame,D3DFMT_A8R8G8B8,GQ_BICUBIC);
//#else
//			shared_ptr<PixelConvert> pVideoScale (new PixelConvert(pAvFrame,D3DFMT_A8R8G8B8,GQ_BICUBIC));
//#endif
			m_nFrameWidth = pAvFrame->width;
			m_nFrameHeight = pAvFrame->height;
			m_pVideoScale->ConvertPixel(pAvFrame);
			m_pSnapshotSurface->GetDesc(&desc);
			m_pSnapshotSurface->LockRect(&D3dRect, NULL, D3DLOCK_DONOTWAIT);
			memcpy_s(D3dRect.pBits, D3dRect.Pitch*desc.Height, m_pVideoScale->pImage, m_pVideoScale->nImageSize);
			m_pSnapshotSurface->UnlockRect();
			SetEvent(m_hEventFrameReady);
			DxTraceMsg("%s Surface is Transfered.\n", __FUNCTION__);
		}
	}
	
	// 解码抓图，把Surface中的图像数据保存到文件中，此截图得到的图像是原始的图像
	virtual bool SaveSurfaceToFileA(CHAR *szFilePath,D3DXIMAGE_FILEFORMAT D3DImageFormat = D3DXIFF_JPG)
	{
		if (!szFilePath || strlen(szFilePath) <= 0)
			return false;
		
		WCHAR szFilePathW[1024] = { 0 };
		MultiByteToWideChar(CP_ACP, 0, szFilePath, -1, szFilePathW, 1024);
		return SaveSurfaceToFileW(szFilePathW, D3DImageFormat);
	}
	
	// 解码抓图，把Surface中的图像数据保存到文件中，此截图得到的图像是原始的图像
	virtual bool SaveSurfaceToFileW(WCHAR *szFilePath, D3DXIMAGE_FILEFORMAT D3DImageFormat = D3DXIFF_JPG)
	{
		if (!m_pDirect3DDevice ||
			!m_pSurfaceRender)
			return false;
		if (!szFilePath || wcslen(szFilePath) <= 0)
			return false;

		SetEvent(m_hEventSnapShot);
		m_bSnapFlag = true;

		CAutoLock lock(&m_csSnapShot);		
		m_D3DXIFF = D3DImageFormat;
		HRESULT hr = S_OK;
		if (!m_pSnapshotSurface)
		{
			HRESULT hr = m_pDirect3DDevice->CreateOffscreenPlainSurface(m_nVideoWidth,
				m_nVideoHeight,
				D3DFMT_A8R8G8B8,
				D3DPOOL_SYSTEMMEM,
				&m_pSnapshotSurface,
				NULL);
			if (FAILED(hr))
			{
				DxTraceMsg("%s IDirect3DSurface9::GetDesc failed,hr = %08X.\n", __FUNCTION__, hr);
				return false;
			}
		}

		wcscpy(m_szSnapShotPath, szFilePath);
		// 截图数据尚未就绪,则置信截图事件
		if (WaitForSingleObject(m_hEventFrameReady, 1000) == WAIT_TIMEOUT)
			return false;
		// 截图数据已就绪			
		
		hr = D3DXSaveSurfaceToFileW(szFilePath, m_D3DXIFF, m_pSnapshotSurface, NULL, NULL);
		if (FAILED(hr))
		{
			DxTraceMsg("%s D3DXSaveSurfaceToFile Failed,hr = %08X.\n", __FUNCTION__, hr);
			return false;
		}
		m_bSnapFlag = false;
		return true;
	}
	
	// 屏幕抓图，把显示到屏幕上的图象，保存到文件中,此截图得到的有可能不是原始的图像，可能是被拉伸或处理过的图像 
	virtual void CaptureScreen(TCHAR *szFilePath,D3DXIMAGE_FILEFORMAT D3DImageFormat = D3DXIFF_JPG)		
	{
		if (!m_pDirect3DDevice)
			return ;

		D3DDISPLAYMODE mode;
		HRESULT hr = m_pDirect3DDevice->GetDisplayMode(0,&mode);
		if (FAILED(hr))
		{
			DxTraceMsg("%s IDirect3DSurface9::GetDesc GetDisplayMode,hr = %08X.\n",__FUNCTION__,hr);
			return ;
		}

		IDirect3DSurface9 *pSnapshotSurface = NULL;	
		hr = m_pDirect3DDevice->CreateOffscreenPlainSurface(mode.Width,
															mode.Height, 
															D3DFMT_A8R8G8B8, 
															D3DPOOL_SYSTEMMEM, 
															&pSnapshotSurface, 
															NULL);
		if (FAILED(hr))
		{
			DxTraceMsg("%s IDirect3DSurface9::GetDesc failed,hr = %08X.\n",__FUNCTION__,hr);
			return ;
		}
		
		hr = m_pDirect3DDevice->GetFrontBufferData(0,pSnapshotSurface);
		if (FAILED(hr))
		{
			DxTraceMsg("%s IDirect3DDevice9::GetFrontBufferData failed,hr = %08X.\n",__FUNCTION__,hr);
			SafeRelease(pSnapshotSurface);
			return ;
		}

		RECT *rtSaved = NULL;
		WINDOWINFO windowInfo ;
		if (m_d3dpp.Windowed)
		{
			windowInfo.cbSize = sizeof(WINDOWINFO);
			GetWindowInfo(m_d3dpp.hDeviceWindow, &windowInfo);
			rtSaved = &windowInfo.rcWindow;
		}
		//_tcscpy_s(m_szSnapShotPath,MAX_PATH,szPath);
		// m_D3DXIFF = D3DXIFF_JPG;
		hr = D3DXSaveSurfaceToFile(szFilePath,D3DImageFormat,pSnapshotSurface,NULL,rtSaved);
		if (FAILED(hr))		
			DxTraceMsg("%s D3DXSaveSurfaceToFile Failed,hr = %08X.\n",__FUNCTION__,hr);
		SafeRelease(pSnapshotSurface);
	}
	/*
	D3DXIFF_BMP          = 0,
	D3DXIFF_JPG          = 1,
	D3DXIFF_TGA          = 2,
	D3DXIFF_PNG          = 3,
	D3DXIFF_DDS          = 4,
	D3DXIFF_PPM          = 5,
	D3DXIFF_DIB          = 6,
	D3DXIFF_HDR          = 7,
	D3DXIFF_PFM          = 8,
	*/
	bool CapturetoFile(TCHAR *szPath,D3DXIMAGE_FILEFORMAT nD3DXIFF)
	{
		//_tcscpy_s(m_szSnapShotPath,MAX_PATH,szPath);
#ifdef _UNICODE
		return SaveSurfaceToFileW(szPath,nD3DXIFF);
#else
		return SaveSurfaceToFileA(szPath, nD3DXIFF);
#endif
	}

	bool CapturetoBmp(TCHAR *szPath)
	{
		//_tcscpy_s(m_szSnapShotPath,MAX_PATH,szPath);
#ifdef _UNICODE
		SaveSurfaceToFileW(szPath,D3DXIFF_BMP);
#else
		SaveSurfaceToFileA(szPath, D3DXIFF_BMP);
#endif
	}
	
	// 调用InitD3D之前必须先调用AttachWnd函数关联视频显示窗口
	// nD3DFormat 必须为以下格式之一
	// MAKEFOURCC('Y', 'V', '1', '2')	默认格式,可以很方便地由YUV420P转换得到,而YUV420P是FFMPEG解码后得到的默认像素格式
	// MAKEFOURCC('N', 'V', '1', '2')	仅DXVA硬解码使用该格式
	// D3DFMT_R5G6B5
	// D3DFMT_X1R5G5B5
	// D3DFMT_A1R5G5B5
	// D3DFMT_R8G8B8
	// D3DFMT_X8R8G8B8
	// D3DFMT_A8R8G8B8
	virtual bool InitD3D(HWND hWnd,
				 int nVideoWidth,
				 int nVideoHeight,
				 BOOL bIsWindowed = TRUE,
				 D3DFORMAT nD3DFormat = (D3DFORMAT)MAKEFOURCC('Y', 'V', '1', '2'))
	{
		//TraceFunction();
		assert(hWnd != NULL);
		assert(IsWindow(hWnd));
		assert(nVideoWidth != 0 || nVideoHeight != 0);
		bool bSucceed = false;
		
		D3DCAPS9 caps;
		m_pDirect3D9->GetDeviceCaps(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,&caps);
		int vp = 0;
		if (caps.DevCaps& D3DDEVCAPS_HWTRANSFORMANDLIGHT)		
			vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
		else		
			vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;

		HRESULT hr = S_OK;		
		D3DDISPLAYMODE d3ddm;
		if (FAILED(hr = m_pDirect3D9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm)))
		{
			DxTraceMsg("%s GetAdapterDisplayMode Failed.\nhr=%x", __FUNCTION__, hr);
			goto _Failed;
		}

		ZeroMemory(&m_d3dpp, sizeof(D3DPRESENT_PARAMETERS));
		m_d3dpp.BackBufferFormat		= d3ddm.Format;
		m_d3dpp.BackBufferCount			= 1;
		m_d3dpp.Flags					= 0;
		m_d3dpp.Windowed				= bIsWindowed;
		m_d3dpp.PresentationInterval	= D3DPRESENT_INTERVAL_ONE;// D3DPRESENT_INTERVAL_IMMEDIATE;
		m_d3dpp.hDeviceWindow			= hWnd;
		m_d3dpp.MultiSampleQuality		= 0;
		m_d3dpp.MultiSampleType			= D3DMULTISAMPLE_NONE;					// 显示视频时，不宜使用多重采样，否则将导致画面错乱
		m_d3dpp.SwapEffect				= D3DSWAPEFFECT_DISCARD;				// 指定系统如何将后台缓冲区的内容复制到前台缓冲区 D3DSWAPEFFECT_DISCARD:清除后台缓存的内容
#pragma warning(push)
#pragma warning(disable:4800)
		m_bFullScreen					= (bool)bIsWindowed;
#pragma warning(pop)		

		if (bIsWindowed)//窗口模式
		{
			if (m_dwStyle)
				SetWindowLong(m_d3dpp.hDeviceWindow, GWL_STYLE, m_dwStyle);
			if (m_dwExStyle)
				SetWindowLong(m_d3dpp.hDeviceWindow, GWL_EXSTYLE, m_dwExStyle);
			SetWindowPlacement(m_d3dpp.hDeviceWindow, &m_WndPlace) ;
			if (!m_bEnableVsync)
				m_d3dpp.PresentationInterval	= D3DPRESENT_INTERVAL_IMMEDIATE;
			else
				m_d3dpp.PresentationInterval	= D3DPRESENT_INTERVAL_DEFAULT;
			m_d3dpp.FullScreen_RefreshRateInHz	= 0;							// 显示器刷新率，窗口模式该值必须为0
			
			m_d3dpp.BackBufferHeight			= nVideoHeight;
			m_d3dpp.BackBufferWidth				= nVideoWidth;
			m_d3dpp.EnableAutoDepthStencil		= FALSE;							// 关闭自动深度缓存
		}
		else
		{
			//全屏模式
			m_d3dpp.PresentationInterval		= D3DPRESENT_INTERVAL_IMMEDIATE;
			m_d3dpp.FullScreen_RefreshRateInHz	= d3ddm.RefreshRate;	
			
			m_d3dpp.EnableAutoDepthStencil		= FALSE;			
			m_d3dpp.BackBufferWidth				= GetSystemMetrics(SM_CXSCREEN);		// 获得屏幕宽
			m_d3dpp.BackBufferHeight			= GetSystemMetrics(SM_CYSCREEN);		// 获得屏幕高

			GetWindowPlacement(m_d3dpp.hDeviceWindow, &m_WndPlace ) ;
			m_dwExStyle	 = GetWindowLong( m_d3dpp.hDeviceWindow, GWL_EXSTYLE ) ;
			m_dwStyle	 = GetWindowLong( m_d3dpp.hDeviceWindow, GWL_STYLE ) ;
			m_dwStyle	 &= ~WS_MAXIMIZE & ~WS_MINIMIZE; // remove minimize/maximize style
			m_hMenu		 = GetMenu( m_d3dpp.hDeviceWindow ) ;
		}
		/*
		CreateDevice的BehaviorFlags参数选项：
		D3DCREATE_ADAPTERGROUP_DEVICE只对主显卡有效，让设备驱动输出给它所拥有的所有显示输出
		D3DCREATE_DISABLE_DRIVER_MANAGEMENT代替设备驱动来管理资源，这样在发生资源不足时D3D调用不会失败
		D3DCREATE_DISABLE_PRINTSCREEN:不注册截屏快捷键，只对Direct3D 9Ex
		D3DCREATE_DISABLE_PSGP_THREADING：强制计算工作必须在主线程上，vista以上有效
		D3DCREATE_ENABLE_PRESENTSTATS：允许GetPresentStatistics收集统计信息只对Direct3D 9Ex
		D3DCREATE_FPU_PRESERVE；强制D3D与线程使用相同的浮点精度，会降低性能
		D3DCREATE_HARDWARE_VERTEXPROCESSING：指定硬件进行顶点处理，必须跟随D3DCREATE_PUREDEVICE
		D3DCREATE_MIXED_VERTEXPROCESSING：指定混合顶点处理
		D3DCREATE_SOFTWARE_VERTEXPROCESSING：指定纯软的顶点处理
		D3DCREATE_MULTITHREADED：要求D3D是线程安全的，多线程时
		D3DCREATE_NOWINDOWCHANGES：拥有不改变窗口焦点
		D3DCREATE_PUREDEVICE：只试图使用纯硬件的渲染
		D3DCREATE_SCREENSAVER：允许被屏保打断只对Direct3D 9Ex
		D3DCREATE_HARDWARE_VERTEXPROCESSING, D3DCREATE_MIXED_VERTEXPROCESSING, and D3DCREATE_SOFTWARE_VERTEXPROCESSING中至少有一个一定要设置
		*/
		if (FAILED(hr = m_pDirect3D9->CreateDevice(D3DADAPTER_DEFAULT,
			D3DDEVTYPE_HAL,
			m_d3dpp.hDeviceWindow,
			vp | D3DCREATE_MULTITHREADED,
			&m_d3dpp,
			/* NULL,*/
			&m_pDirect3DDevice)))
		{
			DxTraceMsg("%s CreateDevice Failed.\nhr=%x", __FUNCTION__, hr);
			goto _Failed;
		}

		D3DPOOL nPoolArray[] = { D3DPOOL_DEFAULT, D3DPOOL_MANAGED ,	D3DPOOL_SYSTEMMEM ,	D3DPOOL_SCRATCH };
		bSucceed = false;
		int nPool = 0;
		while (nPool <= 3)
		{
			if (FAILED(hr = m_pDirect3DDevice->CreateOffscreenPlainSurface(nVideoWidth,
				nVideoHeight,
				nD3DFormat,
				nPoolArray[nPool],
				&m_pSurfaceRender,
				NULL)))
			{
				nPool++;
				continue;
			}
			else
			{
				m_nCurD3DPool = nPoolArray[nPool];
				bSucceed = true;
				break;
			}
		}
		if (!bSucceed)
		{
			DxTraceMsg("%s CreateOffscreenPlainSurface Failed.\nhr=%x", __FUNCTION__, hr);
			goto _Failed;
		}
		D3DSURFACE_DESC SrcSurfaceDesc;			
		m_pSurfaceRender->GetDesc(&SrcSurfaceDesc);
		// 保存参数
		m_nVideoWidth	 = nVideoWidth;
		m_nVideoHeight	 = nVideoHeight;
		m_nD3DFormat = nD3DFormat;		
		bSucceed = true;
		m_bInitialized = true;
_Failed:
		if (!bSucceed)
		{
			DxCleanup();
		}
		return bSucceed;
	}

	// 创建YUV缓存表面
	// 或使用硬解码则创建
// 	virtual bool CreateSurfaceYUVCache(D3DFORMAT nD3DFormat = (D3DFORMAT)MAKEFOURCC('Y', 'V', '1', '2'))
// 	{
// 		if (!m_pDirect3D9 || !m_pSurfaceRender)
// 		{
// 			return false;
// 		}
// 		D3DPOOL nPoolArray[] = { D3DPOOL_DEFAULT, D3DPOOL_MANAGED, D3DPOOL_SYSTEMMEM, D3DPOOL_SCRATCH };
// 		bSucceed = false;
// 			
// 		if (FAILED(hr = m_pDirect3DDevice->CreateOffscreenPlainSurface(nVideoWidth,
// 			nVideoHeight,
// 			nD3DFormat,
// 			m_nCurD3DPool,
// 			&m_pSurfaceYUVCache,
// 			NULL)))
// 		{
// 			return false;
// 		}
// 		m_hYUVCacheReady = CreateEvent(NULL, FALSE, FALSE, NULL);
// 		return true;
// 	}
	
// 	virtual bool TransferYUV(AVFrame *pAvFrame)
// 	{
// 		if (!pAvFrame)
// 			return false;
// 		CTryLock Trylock;
// 		
// 		if (!HandelDevLost())
// 			return false;
// 	
// 		// HandelDevLost仍无法使用m_pDirect3DDevice，则直接返回false
// 		if (!m_pDirect3DDevice)
// 			return false;
// 		HRESULT hr = -1;
// 		//CAutoLock lock(&m_csRender,false,__FUNCTION__,__LINE__);	
// 		switch (pAvFrame->format)
// 		{
// 		case  AV_PIX_FMT_DXVA2_VLD:
// 		{// 硬解码帧，可以直接显示
// 			IDirect3DSurface9* pRenderSurface = m_pSurfaceYUVCache;
// 			IDirect3DSurface9* pSurface = (IDirect3DSurface9 *)pAvFrame->data[3];
// 						
// 			D3DLOCKED_RECT SrcRect;
// 			D3DLOCKED_RECT DstRect;
// 			D3DSURFACE_DESC DXVASurfaceDesc, DstSurfaceDesc;
// 			pSurface->GetDesc(&DXVASurfaceDesc);
// 			m_pSurfaceRender->GetDesc(&DstSurfaceDesc);
// 			hr = pSurface->LockRect(&SrcRect, nullptr, D3DLOCK_READONLY);
// 			hr |= m_pSurfaceRender->LockRect(&DstRect, NULL, D3DLOCK_DONOTWAIT);
// 			//DxTraceMsg("hr = %08X.\n", hr);
// 
// 			if (FAILED(hr))
// 			{
// 				DxTraceMsg("%s line(%d) IDirect3DSurface9::LockRect failed:hr = %08X.\n", __FUNCTION__, __LINE__, hr);
// 				return false;
// 			}
// 			if (SrcRect.Pitch == DstRect.Pitch)
// 				gpu_memcpy(DstRect.pBits, SrcRect.pBits, SrcRect.Pitch*DstSurfaceDesc.Height * 3 / 2);
// 			else
// 			{
// 				// Y分量图像
// 				uint8_t *pSrcY = (uint8_t*)SrcRect.pBits;
// 				// UV分量图像
// 				uint8_t *pSrcUV = (uint8_t*)SrcRect.pBits + SrcRect.Pitch * DstSurfaceDesc.Height;
// 
// 				uint8_t *pDstY = (uint8_t *)DstRect.pBits;
// 				uint8_t *pDstUV = (uint8_t *)DstRect.pBits + DstRect.Pitch*DXVASurfaceDesc.Height;
// 
// 				// 复制Y分量
// 				for (UINT i = 0; i < DXVASurfaceDesc.Height; i++)
// 					gpu_memcpy(&pDstY[i*DstRect.Pitch], &pSrcY[i*SrcRect.Pitch], DXVASurfaceDesc.Width);
// 				for (UINT i = 0; i < DXVASurfaceDesc.Height / 2; i++)
// 					gpu_memcpy(&pDstUV[i*DstRect.Pitch], &pSrcUV[i*SrcRect.Pitch], DXVASurfaceDesc.Width);
// 
// 			}
// 			hr |= m_pSurfaceRender->UnlockRect();
// 			hr |= pSurface->UnlockRect();
// 			if (FAILED(hr))
// 			{
// 				DxTraceMsg("%s line(%d) IDirect3DSurface9::UnlockRect failed:hr = %08X.\n", __FUNCTION__, __LINE__, hr);
// 				return false;
// 			}
// 			SetEvent(m_hYUVCacheReady);
// 			break;
// 		}
// 		case AV_PIX_FMT_YUV420P:
// 		case AV_PIX_FMT_YUVJ420P:
// 		{// 软解码帧，只支持YUV420P格式		
// 			
// 			D3DLOCKED_RECT d3d_rect;
// 			D3DSURFACE_DESC Desc;
// 			hr = m_pSurfaceYUVCache->GetDesc(&Desc);
// 			hr |= m_pSurfaceYUVCache->LockRect(&d3d_rect, NULL, D3DLOCK_DONOTWAIT);
// 			if (FAILED(hr))
// 			{
// 				DxTraceMsg("%s line(%d) IDirect3DSurface9::LockRect failed:hr = %08X.\n", __FUNCTION__, __LINE__, hr);
// 				return false;
// 			}
// 			if ((pAvFrame->format == AV_PIX_FMT_YUV420P ||
// 				pAvFrame->format == AV_PIX_FMT_YUVJ420P) &&
// 				Desc.Format == (D3DFORMAT)MAKEFOURCC('Y', 'V', '1', '2'))
// 				CopyFrameYUV420P(&d3d_rect, Desc.Height, pAvFrame);
// 			else
// 			{
// 				if (!m_pPixelConvert)
// #if _MSC_VER > 1600
// 					m_pPixelConvert = make_shared<PixelConvert>(pAvFrame, Desc.Format);
// #else
// 					m_pPixelConvert = shared_ptr<PixelConvert>(new PixelConvert(pAvFrame, Desc.Format));
// #endif
// 				m_pPixelConvert->ConvertPixel(pAvFrame);
// 				if (m_pPixelConvert->GetDestPixelFormat() == AV_PIX_FMT_YUV420P)
// 					CopyFrameYUV420P(&d3d_rect, Desc.Height, m_pPixelConvert->pFrameNew);
// 				else
// 					memcpy((byte *)d3d_rect.pBits, m_pPixelConvert->pImage, m_pPixelConvert->nImageSize);
// 			}
// 			hr = m_pSurfaceYUVCache->UnlockRect();
// 			
// 			if (FAILED(hr))
// 			{
// 				DxTraceMsg("%s line(%d) IDirect3DSurface9::UnlockRect failed:hr = %08X.\n", __FUNCTION__, __LINE__, hr);
// 				return false;
// 			}
// 		}
// 		break;
// 		case AV_PIX_FMT_NONE:
// 		{
// 			DxTraceMsg("*************************************.\n");
// 			DxTraceMsg("*%s	Get a None picture Frame error	*.\n", __FUNCTION__);
// 			DxTraceMsg("*************************************.\n");
// 			return true;
// 		}
// 		break;
// 		default:
// 		{
// 			DxTraceMsg("%s Get a unsupport format frame:%d.\n", pAvFrame->format);
// 			return true;
// 		}
// 		}
// 		if (SUCCEEDED(hr))
// 			return true;
// 		else
// 			return HandelDevLost();
// 	}

// 	bool CreateSurface(UINT nVideoWidth, UINT nVideoHeight, D3DFORMAT nD3DFormat/* = (D3DFORMAT)MAKEFOURCC('N', 'V', '1', '2')*/)
// 	{
// 		if (!m_pDirect3D9 || !m_pDirect3DDevice)
// 			return false;
// 		int nScreenWidth = GetSystemMetrics(SM_CXFULLSCREEN);
// 		int nScreenHeight = GetSystemMetrics(SM_CXFULLSCREEN);
// 		HRESULT hr = m_pDirect3DDevice->CreateOffscreenPlainSurface(nVideoWidth, nVideoHeight, nD3DFormat, D3DPOOL_DEFAULT, &m_pDirect3DSurfaceRender, NULL);		
// 
// 		if (FAILED(hr))
// 			return false;
// 
// 		if (FAILED(hr))
// 		{
// 			DxCleanup();
// 			return false;
// 		}
// 		else
// 		{
// 			m_nVideoWidth = nVideoWidth;
// 			m_nVideoHeight = nVideoHeight;
// 			m_nD3DFormat = nD3DFormat;
// 			return true;
// 		}
// 	}
	
	virtual inline IDirect3DDevice9 *GetD3DDevice()
	{
		return m_pDirect3DDevice;
	}

	virtual inline IDirect3D9 *GetD3D9()
	{
		return m_pDirect3D9;
	}
	inline void SetD3DShared(bool bD3dShared = false)
	{
		m_bD3DShared = bD3dShared;
	}
	void SetFullScreenWnd(HWND hWnd = NULL)
	{
		CAutoLock lock(&m_csRender);
		m_hFullScreenWindow = hWnd;
	}

	virtual inline bool SetD3DShared(IDirect3D9 *pD3D9)
	{
		if (pD3D9)
		{
			m_bD3DShared = true;
			m_pDirect3D9 = pD3D9;
			return true;
		}
		else
			return false;
	}
	inline bool GetD3DShared()
	{
		return m_bD3DShared;
	}

	bool IsInited()
	{
		return m_bInitialized;
	}
	
	/*
	RECT	m_rcWindow;
#define RectWidth(rt)	(rt.right - rt.left)
#define RectHeight(rt)	(rt.bottom - rt.top)
	WINDOWPLACEMENT	m_windowedPWP;
	HWND	m_hParentWnd;
	// 暂时不要使用这个功能，因为目前尚未找到全屏换回窗口模式的方法
	inline void SwitchFullScreen(HWND hWnd = NULL)
	{
		CAutoLock lock(&m_csRender);
		m_d3dpp.Windowed = !m_d3dpp.Windowed;
		WNDPROC pOldWndProc = (WNDPROC)GetWindowLong(m_d3dpp.hDeviceWindow,GWL_WNDPROC);
		if (m_d3dpp.Windowed)
		{
			m_d3dpp.BackBufferFormat = m_nD3DFormat;
			m_d3dpp.FullScreen_RefreshRateInHz = 0;	
			if ( m_dwStyle != 0 )
				SetWindowLong(m_d3dpp.hDeviceWindow, GWL_STYLE, m_dwStyle );
			if ( m_dwExStyle != 0 )
				SetWindowLong(m_d3dpp.hDeviceWindow, GWL_EXSTYLE, m_dwExStyle );

			if ( m_hMenu != NULL )
				SetMenu(m_d3dpp.hDeviceWindow, m_hMenu );

			if ( m_windowedPWP.length == sizeof( WINDOWPLACEMENT ) )
			{
				if ( m_windowedPWP.showCmd == SW_SHOWMAXIMIZED )
				{
					ShowWindow (m_d3dpp.hDeviceWindow, SW_HIDE );
					m_windowedPWP.showCmd = SW_HIDE ;
					SetWindowPlacement(m_d3dpp.hDeviceWindow, &m_windowedPWP) ;
					ShowWindow (m_d3dpp.hDeviceWindow, SW_SHOWMAXIMIZED ) ;
					m_windowedPWP.showCmd = SW_SHOWMAXIMIZED ;
				}
				else
					SetWindowPlacement(m_d3dpp.hDeviceWindow, &m_windowedPWP ) ;
			}
		}
		else
		{
			m_d3dpp.FullScreen_RefreshRateInHz	 = D3DPRESENT_RATE_DEFAULT;
			m_d3dpp.BackBufferWidth				 = GetSystemMetrics(SM_CXSCREEN);
			m_d3dpp.BackBufferHeight			 = GetSystemMetrics(SM_CYSCREEN);

			GetWindowPlacement(m_d3dpp.hDeviceWindow, &m_windowedPWP ) ;
			m_dwExStyle = GetWindowLong(m_d3dpp.hDeviceWindow, GWL_EXSTYLE ) ;
			m_dwStyle = GetWindowLong(m_d3dpp.hDeviceWindow, GWL_STYLE ) ;
			m_dwStyle &= ~WS_MAXIMIZE & ~WS_MINIMIZE;
			SetWindowLong (m_d3dpp.hDeviceWindow, GWL_STYLE, WS_EX_TOPMOST | WS_POPUP |CS_DBLCLKS);
			SetWindowPos(m_d3dpp.hDeviceWindow,HWND_TOPMOST,0,0,1920,1080,SWP_SHOWWINDOW);
		}
		if (ResetDevice() &&
			RestoreDevice())
		{
			//m_pOldWndProc = (WNDPROC)GetWindowLong(m_d3dpp.hDeviceWindow,GWL_WNDPROC);
			//AttachWnd();			
		}
	}
	*/
	
	// 处理设备丢失
	// m_bManualReset	默认为false,只处理被动的设备丢失,为true时，则是主动制造设备丢失,以便适应窗口尺寸变化，如全屏放大等
	virtual bool HandelDevLost()
	{
		HRESULT hr = S_OK;

		if (!m_pDirect3DDevice)
			return false;
		
		hr = m_pDirect3DDevice->TestCooperativeLevel();
		if (FAILED(hr))
		{
			if (hr == D3DERR_DEVICELOST)
			{
				Sleep(25);
				return false;
			}
			else if (hr == D3DERR_DEVICENOTRESET)
			{
				if(ResetDevice())
				{
					return RestoreDevice();
				}
				else				
				{
					DxTrace("%s ResetDevice Failed,Not Try to Reinit D3D.\n",__FUNCTION__);
					DxCleanup();
					return InitD3D(m_d3dpp.hDeviceWindow, m_nVideoWidth, m_nVideoHeight, m_bFullScreen);
				}
			}
			else
			{
				DxTraceMsg("%s UNKNOWN DEVICE ERROR!\nhr=%x", __FUNCTION__,hr);
				return false;
			}			
		}
				
		return true;
	}
	void CopyFrameYUV420P(D3DLOCKED_RECT *pD3DRect,int nDescHeight,AVFrame *pFrame420P)
	//void CopyFrameYUV420P(byte *pDest,int nDestSize,int nStride,AVFrame *pFrame420P)
	{
		byte *pDest = (byte *)pD3DRect->pBits;
		int nStride = pD3DRect->Pitch;
		int nVideoHeight = min(pFrame420P->height,nDescHeight);
		int nSize = nVideoHeight * nStride;
		int nHalfSize = (nSize) >> 1;	
		byte *pDestY = pDest;										// Y分量起始地址
		byte *pDestV = pDest + nSize;								// U分量起始地址
		int nSizeofV = nHalfSize>>1;
		byte *pDestU = pDestV + (size_t)(nHalfSize >> 1);			// V分量起始地址
		int nSizoefU = nHalfSize>>1;
		

		// YUV420P的U和V分量对调，便成为YV12格式
		// 复制Y分量
//  		for (int i = 0; i < nVideoHeight; i++)
// 			gpu_memcpy(pDestY + i * nStride, /*nSize * 3 / 2 - i*nStride,*/ pFrame420P->data[0] + i * pFrame420P->linesize[0], pFrame420P->width);
// 
// 		// 复制YUV420P的U分量到目村的YV12的U分量
//  		for (int i = 0; i < nVideoHeight / 2; i++)
// 			gpu_memcpy(pDestU + i * nStride / 2,/* nSizoefU - i*nStride / 2,*/ pFrame420P->data[1] + i * pFrame420P->linesize[1], pFrame420P->width / 2);
// 
// 		// 复制YUV420P的V分量到目村的YV12的V分量
//  		for (int i = 0; i < nVideoHeight / 2; i++)
// 			gpu_memcpy(pDestV + i * nStride / 2, /*nSizeofV - i*nStride / 2,*/ pFrame420P->data[2] + i * pFrame420P->linesize[2], pFrame420P->width / 2);
		

		for (int i = 0; i < nVideoHeight; i++)
			memcpy(pDestY + i * nStride, /*nSize * 3 / 2 - i*nStride,*/ pFrame420P->data[0] + i * pFrame420P->linesize[0], pFrame420P->width);

		// 复制YUV420P的U分量到目村的YV12的U分量
		for (int i = 0; i < nVideoHeight / 2; i++)
			memcpy(pDestU + i * nStride / 2,/* nSizoefU - i*nStride / 2,*/ pFrame420P->data[1] + i * pFrame420P->linesize[1], pFrame420P->width / 2);

		// 复制YUV420P的V分量到目村的YV12的V分量
		for (int i = 0; i < nVideoHeight / 2; i++)
			memcpy(pDestV + i * nStride / 2, /*nSizeofV - i*nStride / 2,*/ pFrame420P->data[2] + i * pFrame420P->linesize[2], pFrame420P->width / 2);
	}

	void CopyFrameARGB(byte *pDest,int nDestSize,int nStride,AVFrame *pFrameARGB)
	{	
		for (int i = 0; i < pFrameARGB->height; i++)
			memcpy(pDest + i * nStride, /*nDestSize - i*nStride,*/pFrameARGB->data[0] + i * pFrameARGB->linesize[0], pFrameARGB->width);
	}
	
	virtual bool Render(AVFrame *pAvFrame/*, HWND hWnd = NULL, RECT *pClippedRT = nullptr, RECT *pRenderRt = nullptr*/)
	{
		//TraceMemory();
		DeclareRunTime(5);
		if (!pAvFrame)
			return false;
		CTryLock Trylock;
		if (!Trylock.TryLock(&m_csRender))
			return false;
		
		SaveRunTime();
		if (!HandelDevLost())
			return false;
		SaveRunTime();
		// HandelDevLost仍无法使用m_pDirect3DDevice，则直接返回false
		if (!m_pDirect3DDevice)
			return false;
		HRESULT hr = -1;
#ifdef _DEBUG
		double dfT1 = GetExactTime();
#endif
		//CAutoLock lock(&m_csRender,false,__FUNCTION__,__LINE__);	
		switch(pAvFrame->format)
		{
		case  AV_PIX_FMT_DXVA2_VLD:
			{// 硬解码帧，可以直接显示
				IDirect3DSurface9* pRenderSurface = m_pSurfaceRender;	
				IDirect3DSurface9* pSurface = (IDirect3DSurface9 *)pAvFrame->data[3];
				if (m_bD3DShared)
				{		
					if (!pSurface)
						return false;
					pRenderSurface = pSurface;
				}
				else
				{
					D3DLOCKED_RECT SrcRect;
					D3DLOCKED_RECT DstRect;
					D3DSURFACE_DESC DXVASurfaceDesc, DstSurfaceDesc;
					pSurface->GetDesc(&DXVASurfaceDesc);
					m_pSurfaceRender->GetDesc(&DstSurfaceDesc);
					hr = pSurface->LockRect(&SrcRect, nullptr, D3DLOCK_READONLY);					
					hr |= m_pSurfaceRender->LockRect(&DstRect, NULL, D3DLOCK_DONOTWAIT);
					//DxTraceMsg("hr = %08X.\n", hr);

					if (FAILED(hr))
					{
						DxTraceMsg("%s line(%d) IDirect3DSurface9::LockRect failed:hr = %08X.\n",__FUNCTION__,__LINE__,hr);
						return false;
					}
					if (SrcRect.Pitch == DstRect.Pitch)
						gpu_memcpy(DstRect.pBits, SrcRect.pBits, SrcRect.Pitch*DstSurfaceDesc.Height * 3 / 2);
					else
					{
						// Y分量图像
						uint8_t *pSrcY = (uint8_t*)SrcRect.pBits;
						// UV分量图像
						uint8_t *pSrcUV = (uint8_t*)SrcRect.pBits + SrcRect.Pitch * DstSurfaceDesc.Height;

						uint8_t *pDstY = (uint8_t *)DstRect.pBits;
						uint8_t *pDstUV = (uint8_t *)DstRect.pBits + DstRect.Pitch*DXVASurfaceDesc.Height;

						// 复制Y分量
						for (UINT i = 0; i < DXVASurfaceDesc.Height; i++)
							gpu_memcpy(&pDstY[i*DstRect.Pitch], &pSrcY[i*SrcRect.Pitch], DXVASurfaceDesc.Width);
						for (UINT i = 0; i < DXVASurfaceDesc.Height / 2; i++)
							gpu_memcpy(&pDstUV[i*DstRect.Pitch], &pSrcUV[i*SrcRect.Pitch], DXVASurfaceDesc.Width);

					}
					hr |= m_pSurfaceRender->UnlockRect();
					hr |= pSurface->UnlockRect();
					if (FAILED(hr))
					{
						DxTraceMsg("%s line(%d) IDirect3DSurface9::UnlockRect failed:hr = %08X.\n",__FUNCTION__,__LINE__,hr);
						return false;
					}
				}
				// 处理截图请求
				//TransferSnapShotSurface(pAvFrame);
				
#ifdef _DEBUG
				double dfT2 = GetExactTime();
				//DxTraceMsg("%s TimeSpan(T2-T1)\t%.6f\n",__FUNCTION__,dfT2 - dfT1);
#endif
				break;
			}
		case AV_PIX_FMT_YUV420P:
		case AV_PIX_FMT_YUVJ420P:		
			{// 软解码帧，只支持YUV420P格式		
				SaveRunTime();
				//TransferSnapShotSurface(pAvFrame);
				D3DLOCKED_RECT d3d_rect;
				D3DSURFACE_DESC Desc;
				hr = m_pSurfaceRender->GetDesc(&Desc);
				hr |= m_pSurfaceRender->LockRect(&d3d_rect, NULL, D3DLOCK_DONOTWAIT);
				SaveRunTime();
				//DxTraceMsg("hr = %08X.\n",hr);
				if (FAILED(hr))
				{
					DxTraceMsg("%s line(%d) IDirect3DSurface9::LockRect failed:hr = %08X.\n",__FUNCTION__,__LINE__,hr);
					return false;
				}
 				if ((pAvFrame->format == AV_PIX_FMT_YUV420P ||
					pAvFrame->format == AV_PIX_FMT_YUVJ420P) &&
					Desc.Format == (D3DFORMAT)MAKEFOURCC('Y', 'V', '1', '2'))
					CopyFrameYUV420P(&d3d_rect,Desc.Height, pAvFrame);
 				else
				{
					if (!m_pPixelConvert)
#if _MSC_VER > 1600
						m_pPixelConvert = make_shared<PixelConvert>(pAvFrame,Desc.Format);
#else
						m_pPixelConvert = shared_ptr<PixelConvert>(new PixelConvert(pAvFrame,Desc.Format));
#endif
					m_pPixelConvert->ConvertPixel(pAvFrame);
					if (m_pPixelConvert->GetDestPixelFormat() == AV_PIX_FMT_YUV420P)
						CopyFrameYUV420P(&d3d_rect,Desc.Height,m_pPixelConvert->pFrameNew);
					else					
						memcpy((byte *)d3d_rect.pBits,m_pPixelConvert->pImage,m_pPixelConvert->nImageSize);
				}
				hr = m_pSurfaceRender->UnlockRect();
				SaveRunTime();
				if (FAILED(hr))
				{
					DxTraceMsg("%s line(%d) IDirect3DSurface9::UnlockRect failed:hr = %08X.\n",__FUNCTION__,__LINE__,hr);
					return false;
				}
#ifdef _DEBUG
				double dfT2 = GetExactTime();
				//DxTraceMsg("%s TimeSpan(T2-T1)\t%.6f\n",__FUNCTION__,dfT2 - dfT1);
#endif
			}
			break;
		case AV_PIX_FMT_NONE:
			{
				DxTraceMsg("*************************************.\n");
				DxTraceMsg("*%s	Get a None picture Frame error	*.\n",__FUNCTION__);
				DxTraceMsg("*************************************.\n");
				return true;
			}
			break;
		default:
			{
				DxTraceMsg("%s Get a unsupport format frame:%d.\n",pAvFrame->format);
				return true;
			}
		}
#ifdef _DEBUG
		double dfT3 = GetExactTime();
#endif
		return true;
		
	}

	virtual bool Present(HWND hWnd = NULL, RECT *pClippedRT = nullptr, RECT *pRenderRt = nullptr)
	{
		DeclareRunTime(5);
		HWND hRenderWnd = m_d3dpp.hDeviceWindow;
		if (hWnd)
			hRenderWnd = hWnd;
		if (!IsNeedRender(hRenderWnd) && !m_bSnapFlag)
			return true;
		

		IDirect3DSurface9 * pBackSurface = NULL;
		m_pDirect3DDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
		m_pDirect3DDevice->BeginScene();
		HRESULT hr = m_pDirect3DDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackSurface);
		D3DSURFACE_DESC desc;

		if (FAILED(hr))
		{
			m_pDirect3DDevice->EndScene();
			DxTraceMsg("%s line(%d) IDirect3DDevice9::GetBackBuffer failed:hr = %08X.\n", __FUNCTION__, __LINE__, hr);
			return true;
		}
		pBackSurface->GetDesc(&desc);
		RECT dstrt = { 0, 0, desc.Width, desc.Height };
		RECT srcrt = { 0, 0, m_nVideoWidth, m_nVideoHeight};
		if (pClippedRT)
		{
			CopyRect(&srcrt, pClippedRT);
		}
		hr = m_pDirect3DDevice->StretchRect(m_pSurfaceRender, &srcrt, pBackSurface, &dstrt, D3DTEXF_LINEAR);

		// 处理外部分绘制接口
		ExternDrawCall(hWnd, pBackSurface, pRenderRt);
	
		pBackSurface->Release();
		
		m_pDirect3DDevice->EndScene();
		// Present(RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion)
		SaveRunTime();
		hr = m_pDirect3DDevice->Present(NULL, pRenderRt, hRenderWnd, NULL);
		SaveRunTime();

#ifdef _DEBUG
		double dfT4 = GetExactTime();
		//DxTraceMsg("%s TimeSpan(T3-T4)\t%.6f\n",__FUNCTION__,dfT4 - dfT3);
#endif
		if (SUCCEEDED(hr))
			return true;
		else
			return HandelDevLost();

	}
	// 设置是以固定比便显示视频
	// 当bScaleFixed为true,并且dfWHScale = 0时,则使用图像原始比例,比例值为Width/Height
	// 当bScaleFixed为true,并且dfWHScale 大于0时,则使用dfWHScale提供的比例显示，图像可能会被拉伸变形
	// 当bScaleFixed为false,dfWHScale参数将被忽略,此时像自动填满窗口客户区
	void SetScaleFixed(bool bSaleFixed = true,float fWHScale = 0.0f)
	{
		m_bVideoScaleFixed = bSaleFixed;
		m_fWHScale = fWHScale;
	}

	bool GetScale(float &fWHScale)
	{
		fWHScale = m_fWHScale;
		return m_bVideoScaleFixed;
	}
/*
	// 设置显示窗口是否要被子类化
	// bWndSubclass为ture，则AttachWnd时会显示窗口进行子类化，否则不会执行子类化操作
	// 此函数必须在AttachWnd前执行才有效
	inline void SetWndSubclass(bool bWndSubclass = true)
	{
		m_bWndSubclass = bWndSubclass;
	}
	inline bool IsWndSubclass()
	{
		return m_bWndSubclass;
	}
	
	bool AttachWnd()
	{
		CAutoLock lock(m_WndSurfaceMapcs->Get());
		WndSurfaceMap::iterator itFind = m_WndSurfaceMap.find(m_d3dpp.hDeviceWindow);
		if (itFind != m_WndSurfaceMap.end())
			return false;
		
 		m_pOldWndProc = (WNDPROC)GetWindowLong(m_d3dpp.hDeviceWindow,GWL_WNDPROC);		
 		if (!m_pOldWndProc)
 			return false;
 		else
 		{
 			if (SetWindowLong(m_d3dpp.hDeviceWindow,GWL_WNDPROC,(long)WndProc))
 			{
 				m_WndSurfaceMap.insert(pair<HWND,CDxSurface*>(m_d3dpp.hDeviceWindow,this));
 				return true;
 			}
 			else			
  				return false;
 		}
	}

	void DetachWnd()
	{
		if (!m_pOldWndProc)
			return;

		if (m_pOldWndProc)
			SetWindowLong(m_d3dpp.hDeviceWindow,GWL_WNDPROC,(long)m_pOldWndProc);
		CAutoLock lock(m_WndSurfaceMapcs->Get());
		m_WndSurfaceMap.erase(m_d3dpp.hDeviceWindow);
		m_pOldWndProc = NULL;
	}
	
	static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		DxTraceMsg("%s Message = %d(%04X).\n",__FUNCTION__,message,message);
		switch (message)
		{
// 		case WM_RENDERFRAME:
// 			{
// 				if (!wParam || !lParam)
// 					return -1;
// 
// 				AVFrame *pAvFrame = (AVFrame *)lParam;
// 				CDxSurface *pSurface = (CDxSurface *)wParam;
// 				if (!pSurface->IsInited())		// D3D设备尚未创建,说明未初始化
// 				{
// 					if (!pSurface->InitD3D(pAvFrame->width,pAvFrame->height))
// 					{
// 						assert(false);
// 						return 0;
// 					}
// 				}
//  				pSurface->Render(pAvFrame);	
// 				break;
// 			}
		case WM_LBUTTONDBLCLK:
			{// 切换窗口模式,要从m_WndSurfaceMap删除窗口句柄
				CAutoLock lock(m_WndSurfaceMapcs->Get());
				WndSurfaceMap::iterator itFind = m_WndSurfaceMap.find(hWnd);
				if (itFind == m_WndSurfaceMap.end())					
					return DefWindowProc(hWnd, message, wParam, lParam);	
				else
				{
					CDxSurface *pSurface = itFind->second;
					m_WndSurfaceMap.erase(itFind);
					SetWindowLong(pSurface->m_d3dpp.hDeviceWindow,GWL_WNDPROC,(LONG)pSurface->m_pOldWndProc);
					pSurface->SwitchFullScreen();
					return 0L;
				}
				break;
			}
		default:
			{
				CAutoLock lock(m_WndSurfaceMapcs->Get());
				WndSurfaceMap::iterator itFind = m_WndSurfaceMap.find(hWnd);
				if (itFind == m_WndSurfaceMap.end())					
					return DefWindowProc(hWnd, message, wParam, lParam);	
				else
				{
					CDxSurface *pSurface = itFind->second;
					return pSurface->m_pOldWndProc(hWnd, message, wParam, lParam);
				}
			}
		}
		return 0l;
	}
*/
public:
	// 1.检查指定的表面像素格式，是否在指定的适配器类型、适配器像素格式下可用。
	// GetAdapterDisplayMode,CheckDeviceType的应用
	bool GetBackBufferFormat(D3DDEVTYPE deviceType,BOOL bWindow, D3DFORMAT &fmt)
	{
		if(m_pDirect3D9 == NULL)
			return false;

		D3DDISPLAYMODE adapterMode;
		m_pDirect3D9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT,&adapterMode);
		if(D3D_OK != m_pDirect3D9->CheckDeviceType(D3DADAPTER_DEFAULT,deviceType, adapterMode.Format, fmt, bWindow))
			fmt = adapterMode.Format;
		return true;
	}

	// 2.根据适配器类型，获取顶点运算(变换和光照运算)的格式
	// D3DCAPS9结构体，GetDeviceCaps的应用
	bool GetDisplayVertexType(D3DDEVTYPE deviceType, int &nVertexType)
	{
		if(m_pDirect3D9 == NULL)
			return false;

		D3DCAPS9 caps;
		m_pDirect3D9->GetDeviceCaps(D3DADAPTER_DEFAULT, deviceType,&caps);
		if( caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT )
			nVertexType = D3DCREATE_HARDWARE_VERTEXPROCESSING;
		else
			nVertexType = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		return true;

	}

	char* StringFromGUID(GUID *pGuid)
	{
		static char szGuidStringA[64] = {0};
		WCHAR szGuidStringW[64] = {0};
		StringFromGUID2(*pGuid,szGuidStringW,64);
		WideCharToMultiByte(CP_ACP,0,szGuidStringW,0,(LPSTR)szGuidStringA,64,NULL,NULL);
		return szGuidStringA;
	}

	// 3.输出显卡信息,Description描述，厂商型号，Dircet3D的驱动Driver版本号，显卡的唯一标识号：DeviceIdentifier
	// GetAdapterCount()，GetAdapterIdentifier的使用。
	void PrintDisplayInfo()
	{
		if (m_pDirect3D9 == NULL)
			return;

		D3DADAPTER_IDENTIFIER9 adapterID; // Used to store device info
		DWORD dwDisplayCount = m_pDirect3D9->GetAdapterCount();
		for(DWORD i = 0; i < dwDisplayCount; i++)
		{
			if (m_pDirect3D9->GetAdapterIdentifier(i/*D3DADAPTER_DEFAULT*/, 0, &adapterID) != D3D_OK)
				return;

			DxTraceMsg("[%d]Driver: %s.\n", i, adapterID.Driver);
			DxTraceMsg("[%d]Description: %s\n", i, adapterID.Description);
			DxTraceMsg("[%d]Device Name: %s\n", i, adapterID.DeviceName);
			DxTraceMsg("[%d]Vendor id:%4x\n", i, adapterID.VendorId);
			DxTraceMsg("[%d]Device id: %4x\n", i, adapterID.DeviceId);
			DxTraceMsg("[%d]Product: %x\n", i, HIWORD(adapterID.DriverVersion.HighPart));
			DxTraceMsg("[%d]Version:%x\n", i, LOWORD(adapterID.DriverVersion.HighPart));
			DxTraceMsg("[%d]SubVersion: %x\n", i, HIWORD(adapterID.DriverVersion.LowPart));
			DxTraceMsg("[%d]Build: %x %d.%d.%d.%d\n", i, LOWORD(adapterID.DriverVersion.LowPart),
				HIWORD(adapterID.DriverVersion.HighPart),
				LOWORD(adapterID.DriverVersion.HighPart),
				HIWORD(adapterID.DriverVersion.LowPart),
				LOWORD(adapterID.DriverVersion.LowPart));
			DxTraceMsg("[%d]SubSysId: %x\n, Revision: %x\n,GUID %s\n, WHQLLevel:%d\n", i,
				adapterID.SubSysId,
				adapterID.Revision,
				StringFromGUID(&adapterID.DeviceIdentifier),
				adapterID.WHQLLevel);
		}
	}

	// 4.输出指定Adapter，显卡像素模式(不会与缓存表面格式做兼容考虑)的显卡适配器模式信息
	// GetAdapterModeCount,EnumAdapterModes的使用
	void PrintDisplayModeInfo(D3DFORMAT fmt)
	{
		if(m_pDirect3D9 == NULL)
		{
			DxTraceMsg("%s Direct3D9 not initialized.\n",__FUNCTION__);
			return;
		}
		// 显卡适配器模式的个数，主要是分辨率的差异
		DWORD nAdapterModeCount=m_pDirect3D9->GetAdapterModeCount(D3DADAPTER_DEFAULT, fmt);
		if(nAdapterModeCount == 0)
		{
			DxTraceMsg("%s D3DFMT_格式：%x不支持", __FUNCTION__,fmt);
		}
		for(DWORD i = 0; i < nAdapterModeCount; i++)
		{
			D3DDISPLAYMODE mode;
			if(D3D_OK == m_pDirect3D9->EnumAdapterModes(D3DADAPTER_DEFAULT,fmt, i,&mode))
				DxTraceMsg( "D3DDISPLAYMODE info, width:%u,height:%u, freshRate:%u, Format:%d \n",
							mode.Width, 
							mode.Height, 
							mode.RefreshRate,
							mode.Format);
		}
	}

	// 5.对于指定的资源类型，检查资源的使用方式，资源像素格式，在默认的显卡适配器下是否支持
	// GetAdapterDisplayMode，CheckDeviceFormat的使用
	bool CheckResourceFormat(DWORD nSrcUsage,D3DRESOURCETYPE srcType, D3DFORMAT srcFmt)
	{
		if(m_pDirect3D9 == NULL)
			return false;

		D3DDISPLAYMODE displayMode;
		m_pDirect3D9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT,&displayMode);
		if(D3D_OK == m_pDirect3D9->CheckDeviceFormat(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL, displayMode.Format, nSrcUsage, srcType, srcFmt))
			return true;
		return false;
	}

	// 6.对指定的表面像素格式，窗口模式，和显卡像素模式；检查对指定的多重采样类型支持不，且返回质量水平等级
	// CheckDeviceMultiSampleType的应用
	bool CheckMultiSampleType(D3DFORMAT surfaceFmt,BOOL bWindow, D3DMULTISAMPLE_TYPE &eSampleType, DWORD *pQualityLevel)
	{
		//变量MultiSampleType的值设为D3DMULTISAMPLE_NONMASKABLE，就必须设定成员变量MultiSampleQuality的质量等级值
		for (int i = eSampleType;i >= D3DMULTISAMPLE_NONE;i --)
		{
			if( SUCCEEDED(m_pDirect3D9->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT/*caps.AdapterOrdinal*/,
						D3DDEVTYPE_HAL/*caps.DeviceType*/,
						surfaceFmt,
						bWindow,
						(D3DMULTISAMPLE_TYPE)i,
						pQualityLevel)))
					{
						eSampleType = (D3DMULTISAMPLE_TYPE)i;
						return true;
					}
		}
		
		return false;
	}

	// 7.根据显卡适配器和目标缓存类型，检查指定深度缓存的格式是否支持
	// CheckDepthStencilMatch的应用
	bool CheckDepthBufferFormt(D3DFORMAT targetBufferFmt, D3DFORMAT depthFmt)
	{
		if(m_pDirect3D9 == NULL)
			return false;

		D3DDISPLAYMODE mode;
		m_pDirect3D9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode);
		if(D3D_OK == m_pDirect3D9->CheckDepthStencilMatch(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, mode.Format, targetBufferFmt, depthFmt))
			return true;
		return false;
	}

	bool TestDxCheck(HWND hWnd,int nWidth,int nHeight)
	{
		// 1.CheckDeviceFormat,显卡模式和表面会做兼容处理
		D3DFORMAT backBufferFormat = D3DFMT_A8R8G8B8;
		BOOL bIsWindowed = FALSE;
		D3DDEVTYPE deviceType = D3DDEVTYPE_HAL;

		if(!GetBackBufferFormat(D3DDEVTYPE_HAL, bIsWindowed, backBufferFormat))
		{
			DxTraceMsg("%s GetBackBufferFormat - failed.\n", __FUNCTION__);
			return false;
		}

		// 2.Check Vertex Proccessing Type
		int vp = 0;
		if(!GetDisplayVertexType(deviceType, vp))
		{
			DxTraceMsg("%s GetDisplayVertexType - failed.\n", __FUNCTION__);
			return false;
		}
		// 3.显示显卡的信息
		PrintDisplayInfo();

		D3DDISPLAYMODE d3ddm;
		if(FAILED(m_pDirect3D9->GetAdapterDisplayMode(D3DADAPTER_DEFAULT,&d3ddm)))
			return false;
		// 4.输出显卡适配器模式信息,不会与缓存表面格式做兼容考虑
		PrintDisplayModeInfo(d3ddm.Format);

		// 5.显卡模式和资源表面会做兼容处理
		int nUsageTexture = D3DUSAGE_WRITEONLY;
		D3DFORMAT fmtTexture = D3DFMT_A8R8G8B8;

		if(!CheckResourceFormat(nUsageTexture/*D3DUSAGE_DEPTHSTENCIL*/,
			D3DRTYPE_TEXTURE/*D3DRTYPE_SURFACE*/,d3ddm.Format/*D3DFMT_D15S1*/))
		{
			DxTraceMsg("%s CheckResourceFormat Texture Resource FMT failed.\n",__FUNCTION__);
			return false;
		}

		// 6.采样纹理
		D3DMULTISAMPLE_TYPE eSampleType = D3DMULTISAMPLE_16_SAMPLES;// 测试结果本机只是支持4,2类型的采样纹理
		DWORD dwQualityLevel = 0;
		if(!CheckMultiSampleType(d3ddm.Format, bIsWindowed,  eSampleType,&dwQualityLevel))
		{
			eSampleType = D3DMULTISAMPLE_NONE;
		}

		// 7.深度缓存检测
		D3DFORMAT depthStencilFmt = D3DFMT_D24X8/*D3DFMT_D15S1*/;
		if(!CheckDepthBufferFormt(d3ddm.Format, depthStencilFmt))
		{
			DxTraceMsg("%s CheckDepthBufferFormt Texture Resource FMT Failed.\n", __FUNCTION__);
			return false;
		}

		// Step 3: Fill out the D3DPRESENT_PARAMETERS structure.
		D3DPRESENT_PARAMETERS d3dpp;
		d3dpp.BackBufferWidth           = nWidth;
		d3dpp.BackBufferHeight          = nHeight;
		d3dpp.BackBufferFormat          = backBufferFormat;
		d3dpp.BackBufferCount           = 1;
		d3dpp.MultiSampleType           = eSampleType;				//D3DMULTISAMPLE_NONE;
		d3dpp.MultiSampleQuality        = dwQualityLevel;				// 不能用dwQualityLevel
		d3dpp.SwapEffect                = D3DSWAPEFFECT_DISCARD;
		d3dpp.hDeviceWindow             = hWnd;
		d3dpp.Windowed                  = bIsWindowed;
		d3dpp.EnableAutoDepthStencil    = true;
		d3dpp.AutoDepthStencilFormat    = depthStencilFmt;
		d3dpp.Flags                     = 0;
		d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
		d3dpp.PresentationInterval      = D3DPRESENT_INTERVAL_IMMEDIATE;
		return true;
	}
};

//#include "d3dfont.h"
// CDxSurfaceEx类，仅限于Windows Vista及以上操作系统下使用
// 其性能与稳定性比CDxSurface要强，维护也更方便

class CDxSurfaceEx :public CDxSurface
{
public:
	IDirect3D9Ex			*m_pDirect3D9Ex		/* = NULL*/;
	IDirect3DDevice9Ex		*m_pDirect3DDeviceEx	/*= NULL*/;
	pDirect3DCreate9Ex*		m_pDirect3DCreate9Ex;
public:
// 	CDxSurfaceEx(IDirect3D9Ex *pD3D9Ex)
// 		//:m_pDirect3D9Ex(NULL)*,
// 		m_pDirect3DDeviceEx(NULL),
// 		m_pDirect3DCreate9Ex(NULL)
// 	{
// 		InitializeCriticalSection(&m_csRender);
// 		m_nCordinateMode = Coordinte_Wnd;
// 		if (pD3D9Ex)
// 		{
// 			m_bD3DShared = true;
// 			m_pDirect3D9Ex = pD3D9Ex;
// 		}
// 	}
	CDxSurfaceEx()
		//:m_pDirect3D9Ex(NULL),
		:m_pDirect3DDeviceEx(NULL),
		m_pDirect3DCreate9Ex(NULL)
	{
		DeclareRunTime(5);

		m_nCordinateMode = Coordinte_Video;
		// 释放由基类创建的Direct3D9对象
		//SafeRelease(m_pDirect3D9);
		m_pDirect3DCreate9Ex = g_pD3D9Helper.m_pDirect3DCreate9Ex;	
		m_pDirect3D9Ex = g_pD3D9Helper.m_pDirect3D9Ex;
		
#ifdef _DEBUG
		//PrintDisplayInfo(m_pDirect3D9Ex);
#endif
	}
#ifdef _DEBUG
	virtual void OutputDxPtr()
	{
		DxTraceMsg("%s m_pDirect3D9Ex = %p\tm_pDirect3DDeviceEx = %p\tm_pDirect3DSurfaceRender = %p.\n", __FUNCTION__, m_pDirect3D9Ex, m_pDirect3DDeviceEx, m_pSurfaceRender);
	}
#endif

	// 参考文档：https://docs.microsoft.com/en-us/windows/win32/gdi/capturing-an-image
	int CopySurface(HDC hDC, int nWidth, int nHeight)
	{
		HDC hdcMemDC = NULL;
		HBITMAP hBitmapSurface = NULL;
		BITMAP bmpSurface;
		__try
		{
			DWORD dwT = timeGetTime();
			hdcMemDC = CreateCompatibleDC(hDC);
			if (!hdcMemDC)
			{
				DxTraceMsg("%s CreateCompatibleDC has failed.\n",__FUNCTION__);
				__leave;
			}
			hBitmapSurface = CreateCompatibleBitmap(hDC, nWidth, nHeight);

			if (!hBitmapSurface)
			{
				DxTraceMsg("%s CreateCompatibleBitmap Failed.\n",__FUNCTION__);
				__leave;
			}

			// Select the compatible bitmap into the compatible memory DC.
			SelectObject(hdcMemDC, hBitmapSurface);

			// Bit block transfer into our compatible memory DC.
			if (!BitBlt(hdcMemDC,0, 0,nWidth, nHeight,hDC,0, 0,	SRCCOPY))
			{
				DxTraceMsg("%s BitBlt has failed.\n",__FUNCTION__);
				__leave;
			}
			DWORD dwTS = MMTimeSpan(dwT);
			DxTraceMsg("%s TimeSpan1 = %d.\n", __FUNCTION__, dwTS);
			// Get the BITMAP from the HBITMAP
			GetObject(hBitmapSurface, sizeof(BITMAP), &bmpSurface);

			BITMAPINFOHEADER   bi;

			bi.biSize = sizeof(BITMAPINFOHEADER);
			bi.biWidth = bmpSurface.bmWidth;
			bi.biHeight = bmpSurface.bmHeight;
			bi.biPlanes = 1;
			bi.biBitCount = 32;
			bi.biCompression = BI_RGB;
			bi.biSizeImage = 0;
			bi.biXPelsPerMeter = 0;
			bi.biYPelsPerMeter = 0;
			bi.biClrUsed = 0;
			bi.biClrImportant = 0;

			m_nRGBBufferSize = ((bmpSurface.bmWidth * bi.biBitCount + 31) / 32) * 4 * bmpSurface.bmHeight;

			if (!m_pRGBBuffer)
				m_pRGBBuffer = new byte[m_nRGBBufferSize];
			if (!m_pRGBBuffer)
				__leave;

			int nScanLines = GetDIBits(hDC, hBitmapSurface, 0, (UINT)bmpSurface.bmHeight, m_pRGBBuffer, (BITMAPINFO *)&bi, DIB_RGB_COLORS);

			dwTS = MMTimeSpan(dwT);
			DxTraceMsg("%s TimeSpan2 = %d.\n", __FUNCTION__, dwTS);
		}
		__finally
		{
			//Clean up
			DeleteObject(hBitmapSurface);
			DeleteObject(hdcMemDC);
		}
		return 0;
	}

	BOOL GetRGBBuffer(byte **ppBuffer, int &nBuffersize)
	{
		TraceFunction();
		
		if (!m_pDirect3DDeviceEx || !m_pSurfaceRender)
			return FALSE;
		HRESULT hr = S_FALSE;
		if (!m_pRGBSurface)
		{
			hr = m_pDirect3DDeviceEx->CreateOffscreenPlainSurface(m_nVideoWidth,
				m_nVideoHeight,
				D3DFORMAT::D3DFMT_X8R8G8B8,
				D3DPOOL_SYSTEMMEM,
				&m_pRGBSurface,
				NULL);
			if (FAILED(hr))
				return FALSE;
		}

			
		IDirect3DSurface9 * pBackSurface = NULL;
		hr = m_pDirect3DDeviceEx->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackSurface);
		if (FAILED(hr))
			return FALSE;

		D3DSURFACE_DESC DescRGB;
		pBackSurface->GetDesc(&DescRGB);			

		hr = m_pDirect3DDeviceEx->GetRenderTargetData(pBackSurface, m_pRGBSurface);
		D3DLOCKED_RECT rgbRect;
		// A 方案，耗时30+ms,cpu占用15-18%
		/*LPD3DXBUFFER pD3DXBuffer = nullptr;		
		if (FAILED(D3DXSaveSurfaceToFileInMemory(&pD3DXBuffer, D3DXIFF_DIB, m_pRGBSurface, NULL, NULL)))
			return FALSE;
		
		LPVOID pImageBuffer = pD3DXBuffer->GetBufferPointer();
		
		m_nRGBBufferSize = pD3DXBuffer->GetBufferSize();
		if (!pImageBuffer || !m_nRGBBufferSize)
			return FALSE;		
		if (!m_pRGBBuffer)
			m_pRGBBuffer = new byte[m_nRGBBufferSize];
		if (!m_pRGBBuffer)
			return FALSE;
		memcpy(m_pRGBBuffer, pImageBuffer, m_nRGBBufferSize);
		pD3DXBuffer->Release();*/

		// B 方案 可直接获取RGB数据，耗时20-23ms左右 CPU占用10-15%
		/*
		HDC hDC = nullptr;
		m_pRGBSurface->GetDC(&hDC);

		CopySurface(hDC, DescRGB.Width, DescRGB.Height);
		m_pRGBSurface->ReleaseDC(hDC);
		*/

		// 方案 C 只能获取X8R8G8B8数据，耗时10-12ms,CPU占用4-6%
		hr = m_pRGBSurface->LockRect(&rgbRect, NULL, D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_READONLY);
		if (FAILED(hr))
			return FALSE;
		m_nRGBBufferSize = rgbRect.Pitch*DescRGB.Height;
		if (!m_pRGBBuffer)
			m_pRGBBuffer = new byte[m_nRGBBufferSize];
		if (!m_pRGBBuffer)
			return FALSE;
		DWORD dwT1 = timeGetTime();
		memcpy(m_pRGBBuffer, rgbRect.pBits, rgbRect.Pitch*DescRGB.Height);
		TraceMsgA("%s %TimeSpan Copy RGB = %d.\n", __FUNCTION__, MMTimeSpan(dwT1));
		m_pRGBSurface->UnlockRect();		
		nBuffersize = m_nRGBBufferSize;
		*ppBuffer = m_pRGBBuffer;

		// 方案D 详细见IPCPlayer.cpp line[3516~3535]
		return TRUE;
	}

	virtual long CreateFontW(LOGFONTW *pLogFont)
	{
		assert(m_pDirect3DDeviceEx!= nullptr);
		D3DXFONT_DESCW FontDesc;
		FontDesc.Height = pLogFont->lfHeight;
		FontDesc.CharSet = pLogFont->lfCharSet;
		FontDesc.Width = pLogFont->lfWidth;
		FontDesc.Weight = pLogFont->lfWeight;
		FontDesc.Italic = pLogFont->lfItalic;
		FontDesc.MipLevels = 1;
		FontDesc.OutputPrecision = pLogFont->lfOutPrecision;
		FontDesc.PitchAndFamily = pLogFont->lfPitchAndFamily;
		FontDesc.Quality = pLogFont->lfQuality;
		wcscpy_s(FontDesc.FaceName, 32, pLogFont->lfFaceName);
		ID3DXFont *pFont = nullptr;
		if (FAILED(D3DXCreateFontIndirectW(m_pDirect3DDeviceEx, &FontDesc, &pFont)))
		{
			assert(false);
			return 0;
		}
		map<long, OSDInfoPtr> vec;
		m_csMapOSD.Lock();
		m_MapOSD.insert(pair<long, map<long, OSDInfoPtr>>((long)pFont, vec));
		m_csMapOSD.Unlock();
		return (long)pFont;
	}

	void DestroyFont(long nFont)
	{
		if (nFont)
		{
			CAutoLock lock(m_csMapOSD.Get());
			auto itFind = m_MapOSD.find(nFont);
			if (itFind == m_MapOSD.end())
				return;
			m_MapOSD.erase(itFind);
			ID3DXFont *pFont = (ID3DXFont *)nFont;
			SafeRelease(pFont);
		}
	}

	long DrawTextA(long nFont, CHAR *szText, int nLength, RECT rtPostion, DWORD dwFormat, D3DCOLOR nColor)
	{
		assert(nFont != 0);
		if (!nFont)
			return 0;
		if (!szText || !nLength)
			return 0;
		int nNeedBuffSize = ::MultiByteToWideChar(CP_ACP, NULL, szText, nLength, NULL, 0);
		WCHAR *pTextW = new WCHAR[nNeedBuffSize + 1];
		shared_ptr<WCHAR> TextWPtr(pTextW);
		::MultiByteToWideChar(CP_ACP, 0, szText, nLength, pTextW, nNeedBuffSize);
		return DrawTextW(nFont, pTextW, nNeedBuffSize, rtPostion, dwFormat, nColor);
	}

	long DrawTextW(long nFont, WCHAR *szText, int nLength, RECT rtPostion, DWORD dwFormat, D3DCOLOR nColor)
	{
		assert(nFont != 0);
		if (!nFont)
			return 0;
		if (!szText || !nLength)
			return 0;
		CAutoLock lock(m_csMapOSD.Get());
		auto itFind = m_MapOSD.find(nFont);
		if (itFind == m_MapOSD.end())
			return 0;
		OSDInfoPtr OsdPtr = make_shared<OSDInfo>(szText, nLength, rtPostion, dwFormat, nColor);
		itFind->second.insert(pair<long, OSDInfoPtr>((long)OsdPtr.get(), OsdPtr));
		return (long)OsdPtr.get();
	}
	// nText is a handle ,not a real Text
	void RemoveText(long nFont, long nText)
	{
		assert(nFont != 0);
		if (!nFont)
			return;
		if (!nText)
			return;
		CAutoLock lock(m_csMapOSD.Get());
		auto itFind = m_MapOSD.find(nFont);
		if (itFind == m_MapOSD.end())
			return;
		auto itFind2 = itFind->second.find(nText);
		if (itFind2 == itFind->second.end())
			return;
		itFind->second.erase(itFind2);
	}
	virtual IDirect3DSurface9* LoadImage(WCHAR *szFileName)
	{
		// 以下代码加载背景图片
		IDirect3DSurface9 *pSurfaceBackImage = nullptr;
		D3DXIMAGE_INFO ImageInfo;
		if (szFileName)
		{
			HRESULT hr = D3DXGetImageInfoFromFileW(szFileName, &ImageInfo);
			if (SUCCEEDED(hr))
			{
				hr = m_pDirect3DDeviceEx->CreateOffscreenPlainSurface(ImageInfo.Width,
					ImageInfo.Height,
					/*ImageInfo.Format*/
					D3DFMT_A8R8G8B8,
					m_nCurD3DPool,
					&pSurfaceBackImage,
					NULL);
				if (SUCCEEDED(hr))
				{
					hr = D3DXLoadSurfaceFromFileW(
						pSurfaceBackImage,					//目标表面
						NULL,								//目标调色板
						NULL,								//目标矩形,NULL为加载整个表面
						szFileName,							//文件
						NULL,								//源矩形,NULL为复制整个图片
						D3DX_DEFAULT,						//过滤
						D3DCOLOR_ARGB(0,0, 255, 255),		//透明色
						&ImageInfo							//源图像信息
						);
					if (SUCCEEDED(hr))
					{
						return pSurfaceBackImage;
					}
				}
			}
		}
		SafeRelease(pSurfaceBackImage);
		return nullptr;
	}
	
	virtual void DxCleanup()
	{
		SafeRelease(m_pSnapshotSurface);
		if (!m_bD3DShared)
			SafeRelease(m_pSurfaceRender);
		SafeRelease(m_pDirect3DDeviceEx);
	}
	~CDxSurfaceEx()
	{
		/*m_csMapOSD.Lock();
		for (auto it = m_MapOSD.begin(); it != m_MapOSD.end();)
		{
			ID3DXFont *pFont = (ID3DXFont *)it->first;
			it->second.clear();
			SafeRelease(pFont);
			it = m_MapOSD.erase(it);
		}
		m_csMapOSD.Unlock();*/
		DxCleanup();
//		SafeRelease(m_pDirect3D9Ex);
// 		if (m_hD3D9)
// 		{
// 			FreeLibrary(m_hD3D9);
// 			m_hD3D9 = NULL;
// 		}
	}

	virtual bool PresetBackImage(IDirect3DSurface9 * pSurfaceBackImage, D3DXIMAGE_INFO &ImageInfo,HWND hWnd)
	{
		if (!m_pDirect3DDeviceEx)
			return false;
		IDirect3DSurface9 * pBackSurface = NULL;
		m_pDirect3DDeviceEx->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
		m_pDirect3DDeviceEx->BeginScene();
		HRESULT hr = m_pDirect3DDeviceEx->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackSurface);
		if (FAILED(hr))
		{
			m_pDirect3DDeviceEx->EndScene();
			return false;
		}
		D3DSURFACE_DESC Desc1, Desc2;
		pBackSurface->GetDesc(&Desc1);
		pSurfaceBackImage->GetDesc(&Desc2);
		RECT dstrt = { 0, 0, Desc1.Width, Desc1.Height };
		RECT srcrt = { 0, 0, ImageInfo.Width, ImageInfo.Height };

		hr = m_pDirect3DDeviceEx->StretchRect(pSurfaceBackImage, &srcrt, pBackSurface, &dstrt, D3DTEXF_LINEAR);
		//SaveRunTime();
		SafeRelease(pBackSurface);
		m_pDirect3DDeviceEx->EndScene();
		hr = m_pDirect3DDeviceEx->PresentEx(NULL, nullptr, m_hBackImageWnd, NULL, 0);
		return true;
	}

	// 调用InitD3D之前必须先调用AttachWnd函数关联视频显示窗口
	// nD3DFormat 必须为以下格式之一
	// MAKEFOURCC('Y', 'V', '1', '2')	默认格式,可以很方便地由YUV420P转换得到,而YUV420P是FFMPEG解码后得到的默认像素格式
	// MAKEFOURCC('N', 'V', '1', '2')	仅DXVA硬解码使用该格式
	// D3DFMT_R5G6B5
	// D3DFMT_X1R5G5B5
	// D3DFMT_A1R5G5B5
	// D3DFMT_R8G8B8
	// D3DFMT_X8R8G8B8
	// D3DFMT_A8R8G8B8
	bool InitD3D(HWND hWnd,
		int nVideoWidth,
		int nVideoHeight,
		BOOL bIsWindowed = TRUE,
		D3DFORMAT nD3DFormat = (D3DFORMAT)MAKEFOURCC('Y', 'V', '1', '2'),
		CRunlog *pRunlog = nullptr)
	{
		//TraceFunction();
		assert(hWnd != NULL);
		assert(IsWindow(hWnd));
		assert(nVideoWidth != 0 || nVideoHeight != 0);

		RECT rtOld,rtZoom;
		bool bZoomWnd = false;			// 是否需要扩大窗口,DirectX不能在窗口像素面积为0的窗口上工作
		GetWindowRect(hWnd, &rtOld);
		rtZoom = rtOld;
		if (RectWidth(rtZoom) == 0)
		{
			rtZoom.right = rtZoom.left + nVideoWidth;
			bZoomWnd = true;
		}
		if (RectHeight(rtZoom) == 0)
		{
			rtZoom.bottom = rtZoom.top + nVideoHeight;
			bZoomWnd = true;
		}
		struct _RestoreWnd
		{
			HWND hWnd;
			RECT rtRestore;
			_RestoreWnd(HWND hWnd,RECT rtRestore)
			{
				this->hWnd		 = hWnd;
				this->rtRestore	 = rtRestore;
			}
			~_RestoreWnd()
			{
				::MoveWindow(hWnd, rtRestore.left, rtRestore.top, RectWidth(rtRestore), RectHeight(rtRestore), false);
			}
		};
		// 创建成功后，恢复窗口原始尺寸
		//_RestoreWnd RestoreWnd(hWnd, rtOld);
		if (bZoomWnd)
			::MoveWindow(hWnd, rtZoom.left, rtZoom.top, RectWidth(rtZoom), RectHeight(rtZoom), false);
		

		HMONITOR	hCurrentMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST);
		UINT nD3dAdapterCount = m_pDirect3D9Ex->GetAdapterCount();
		m_nDisplayAdapter = D3DADAPTER_DEFAULT;
		for (UINT nAdapter = 0; nAdapter < nD3dAdapterCount; nAdapter ++)
			if (m_pDirect3D9Ex->GetAdapterMonitor(nAdapter) == hCurrentMonitor)
			{
				m_nDisplayAdapter = nAdapter;
				break;
			}
		
		//UINT nCurrentAdapter = D3DADAPTER_DEFAULT;
		//for (int  nAdapter = 0; nAdapter < g_pD3D9Helper.m_nAdapterCount; nAdapter ++)
		//{
		//	if (m_pDirect3D9Ex->GetAdapterMonitor(nAdapter) == hCurrentMonitor)
		//	{
		//		TraceMsgA("%s AdapterID = %d.\r\n", __FUNCTION__,nAdapter);
		//		m_nDisplayAdapter = nAdapter;
		//	}
		//}
		bool bSucceed = false;
#ifdef _DEBUG
		double dfTStart = GetExactTime();		
#endif
		D3DCAPS9 caps;
		m_pDirect3D9Ex->GetDeviceCaps(m_nDisplayAdapter, D3DDEVTYPE_HAL, &caps);
		int vp = 0;
		if (caps.DevCaps& D3DDEVCAPS_HWTRANSFORMANDLIGHT)		
			vp = D3DCREATE_HARDWARE_VERTEXPROCESSING;
		else		
			vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		/*
		caps.DevCaps的取值及含义
		D3DDEVCAPS_CANBLTSYSTONONLOCAL		Device supports blits from system-memory textures to nonlocal video-memory textures.
		D3DDEVCAPS_CANRENDERAFTERFLIP		Device can queue rendering commands after a page flip. Applications do not change their behavior if this flag is set; this capability means that the device is relatively fast.
		D3DDEVCAPS_DRAWPRIMITIVES2			Device can support at least a DirectX 5-compliant driver.
		D3DDEVCAPS_DRAWPRIMITIVES2EX		Device can support at least a DirectX 7-compliant driver.
		D3DDEVCAPS_DRAWPRIMTLVERTEX			Device exports an IDirect3DDevice9::DrawPrimitive-aware hal.
		D3DDEVCAPS_EXECUTESYSTEMMEMORY		Device can use execute buffers from system memory.
		D3DDEVCAPS_EXECUTEVIDEOMEMORY		Device can use execute buffers from video memory.
		D3DDEVCAPS_HWRASTERIZATION			Device has hardware acceleration for scene rasterization.
		D3DDEVCAPS_HWTRANSFORMANDLIGHT		Device can support transformation and lighting in hardware.
		D3DDEVCAPS_NPATCHES					Device supports N patches.
		D3DDEVCAPS_PUREDEVICE				Device can support rasterization, transform, lighting, and shading in hardware.
		D3DDEVCAPS_QUINTICRTPATCHES			Device supports quintic Bézier curves and B-splines.
		D3DDEVCAPS_RTPATCHES				Device supports rectangular and triangular patches.
		D3DDEVCAPS_RTPATCHHANDLEZERO		When this device capability is set, the hardware architecture does not require caching of any information, and uncached patches (handle zero) will be drawn as efficiently as cached ones. Note that setting D3DDEVCAPS_RTPATCHHANDLEZERO does not mean that a patch with handle zero can be drawn. A handle-zero patch can always be drawn whether this cap is set or not.
		D3DDEVCAPS_SEPARATETEXTUREMEMORIES	Device is texturing from separate memory pools.
		D3DDEVCAPS_TEXTURENONLOCALVIDMEM	Device can retrieve textures from non-local video memory.
		D3DDEVCAPS_TEXTURESYSTEMMEMORY		Device can retrieve textures from system memory.
		D3DDEVCAPS_TEXTUREVIDEOMEMORY		Device can retrieve textures from device memory.
		D3DDEVCAPS_TLVERTEXSYSTEMMEMORY		Device can use buffers from system memory for transformed and lit vertices.
		D3DDEVCAPS_TLVERTEXVIDEOMEMORY		Device can use buffers from video memory for transformed and lit vertices.
		*/
		HRESULT hr = S_OK;		
// 		D3DDISPLAYMODE d3ddm;
// 		if (FAILED(hr = m_pDirect3D9Ex->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &d3ddm)))
// 		{
// 			DxTraceMsg("%s GetAdapterDisplayMode Failed.\nhr=%x", __FUNCTION__, hr);
// 			goto _Failed;
// 		}
		
		ZeroMemory(&m_d3dpp, sizeof(D3DPRESENT_PARAMETERS));
		m_d3dpp.BackBufferFormat		= D3DFMT_UNKNOWN/*d3ddm.Format*/;
		m_d3dpp.BackBufferCount			= 1;
		m_d3dpp.Flags					= D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
		m_d3dpp.Windowed				= bIsWindowed;
		m_d3dpp.hDeviceWindow			= hWnd;
		m_d3dpp.MultiSampleQuality		= 0;
		m_d3dpp.MultiSampleType			= D3DMULTISAMPLE_NONE;					// 显示视频时，不宜使用多重采样，否则将导致画面错乱
#pragma warning(push)
#pragma warning(disable:4800)
		m_bFullScreen					= (bool)bIsWindowed;
#pragma warning(pop)		

		if (bIsWindowed)//窗口模式
		{
			if (m_dwStyle)
				SetWindowLong(m_d3dpp.hDeviceWindow, GWL_STYLE, m_dwStyle);
			if (m_dwExStyle)
				SetWindowLong(m_d3dpp.hDeviceWindow, GWL_EXSTYLE, m_dwExStyle);
			if (m_WndPlace.length == sizeof(WINDOWPLACEMENT))
				SetWindowPlacement(m_d3dpp.hDeviceWindow, &m_WndPlace) ;
			
			if (!m_bEnableVsync)
				m_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;
			else
				m_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_DEFAULT;
						
			m_d3dpp.FullScreen_RefreshRateInHz	= 0;							// 显示器刷新率，窗口模式该值必须为0
			m_d3dpp.SwapEffect					= D3DSWAPEFFECT_DISCARD;		// 指定系统如何将后台缓冲区的内容复制到前台缓冲区 D3DSWAPEFFECT_DISCARD:清除后台缓存的内容
			m_d3dpp.BackBufferHeight			= nVideoHeight;
			m_d3dpp.BackBufferWidth				= nVideoWidth;
			m_d3dpp.EnableAutoDepthStencil		= FALSE;							// 关闭自动深度缓存
			m_d3dpp.MultiSampleType				= D3DMULTISAMPLE_NONE;
		}
		else
		{
			//全屏模式
			m_d3dpp.PresentationInterval		= D3DPRESENT_INTERVAL_IMMEDIATE;
			//m_d3dpp.FullScreen_RefreshRateInHz	= d3ddm.RefreshRate;	
			m_d3dpp.SwapEffect					= D3DSWAPEFFECT_DISCARD; 
			m_d3dpp.EnableAutoDepthStencil		= FALSE;			
			m_d3dpp.BackBufferWidth				= GetSystemMetrics(SM_CXSCREEN);		// 获得屏幕宽
			m_d3dpp.BackBufferHeight			= GetSystemMetrics(SM_CYSCREEN);		// 获得屏幕高

			GetWindowPlacement(m_d3dpp.hDeviceWindow, &m_WndPlace ) ;
			m_dwExStyle	 = GetWindowLong( m_d3dpp.hDeviceWindow, GWL_EXSTYLE ) ;
			m_dwStyle	 = GetWindowLong( m_d3dpp.hDeviceWindow, GWL_STYLE ) ;
			m_dwStyle	 &= ~WS_MAXIMIZE & ~WS_MINIMIZE; // remove minimize/maximize style
			m_hMenu		 = GetMenu( m_d3dpp.hDeviceWindow ) ;
		}
		/*
		CreateDevice的BehaviorFlags参数选项：
		D3DCREATE_ADAPTERGROUP_DEVICE		只对主显卡有效，让设备驱动输出给它所拥有的所有显示输出
		D3DCREATE_DISABLE_DRIVER_MANAGEMENT	代替设备驱动来管理资源，这样在发生资源不足时D3D调用不会失败
		D3DCREATE_DISABLE_PRINTSCREEN:		不注册截屏快捷键，只对Direct3D 9Ex
		D3DCREATE_DISABLE_PSGP_THREADING：	强制计算工作必须在主线程上，vista以上有效
		D3DCREATE_ENABLE_PRESENTSTATS：		允许GetPresentStatistics收集统计信息只对Direct3D 9Ex
		D3DCREATE_FPU_PRESERVE；				强制D3D与线程使用相同的浮点精度，会降低性能
		D3DCREATE_HARDWARE_VERTEXPROCESSING：指定硬件进行顶点处理，必须跟随D3DCREATE_PUREDEVICE
		D3DCREATE_MIXED_VERTEXPROCESSING：	指定混合顶点处理
		D3DCREATE_SOFTWARE_VERTEXPROCESSING：指定纯软的顶点处理
		D3DCREATE_MULTITHREADED：			要求D3D是线程安全的，多线程时
		D3DCREATE_NOWINDOWCHANGES：			拥有不改变窗口焦点
		D3DCREATE_PUREDEVICE：				只试图使用纯硬件的渲染
		D3DCREATE_SCREENSAVER：				允许被屏保打断只对Direct3D 9Ex
		D3DCREATE_HARDWARE_VERTEXPROCESSING, D3DCREATE_MIXED_VERTEXPROCESSING, and D3DCREATE_SOFTWARE_VERTEXPROCESSING中至少有一个一定要设置
		*/
		if (m_pDirect3DDeviceEx)
			SafeRelease(m_pDirect3DDeviceEx);

		// 检查是否能够启用多重采样
		// hr = m_pDirect3D9Ex->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, d3ddm.Format, FALSE, D3DMULTISAMPLE_4_SAMPLES, NULL);
		DWORD dwQualityLevel = 0, dwQualityLevelDepth = 0;
//		查询抗锯齿的级别
// 		for (int i = D3DMULTISAMPLE_16_SAMPLES; i > D3DMULTISAMPLE_NONE; --i)
// 		{
// 			if (SUCCEEDED(m_pDirect3D9Ex->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT,
// 				D3DDEVTYPE_HAL,
// 				nD3DFormat,
// 				1,
// 				(D3DMULTISAMPLE_TYPE)i,
// 				&dwQualityLevel)) &&
// 				SUCCEEDED(m_pDirect3D9Ex->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT,
// 				D3DDEVTYPE_HAL,
// 				D3DFMT_D24S8,
// 				1,
// 				(D3DMULTISAMPLE_TYPE)i,
// 				&dwQualityLevelDepth)))
// 			{
// 				DxTraceMsg(_T("Support multi sample level %d, quality level %u, %u"), i, dwQualityLevel, dwQualityLevelDepth);
// 				//params.MultiSampleType = (D3DMULTISAMPLE_TYPE)i;
// 				//params.MultiSampleQuality = min(dwQualityLevel, dwQualityLevelDepth) - 1;
// 
// 				break;
// 			}
// 		}
		//OSDInfoPtr pOSD = make_shared<OSDInfo>();
		TraceMsgA("%s m_nDisplayAdapter = %d.\n",__FUNCTION__, m_nDisplayAdapter);
		if (FAILED(hr = m_pDirect3D9Ex->CreateDeviceEx(m_nDisplayAdapter,
			D3DDEVTYPE_HAL,
			m_d3dpp.hDeviceWindow,
			vp | D3DCREATE_MULTITHREADED,
			&m_d3dpp,
			NULL,
			&m_pDirect3DDeviceEx)))
		{
			if (pRunlog)
				pRunlog->Runlog("%s Failed in CreateDeviceEx On Adapter[%d] with Flag D3DCREATE_MULTITHREADED.hr = %x,Now Try Create without the Flag!\n", __FUNCTION__,m_nDisplayAdapter,hr);
			vp = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
			if (FAILED(hr = m_pDirect3D9Ex->CreateDeviceEx(m_nDisplayAdapter,
				D3DDEVTYPE_HAL,
				m_d3dpp.hDeviceWindow,
				vp,
				&m_d3dpp,
				NULL,
				&m_pDirect3DDeviceEx)))
			{
				if (pRunlog)
					pRunlog->Runlog("%s CreateDeviceEx Failed On Adapter[%d].\thr=%x.\n", __FUNCTION__, m_nDisplayAdapter, hr);
				DxTraceMsg("%s CreateDeviceEx Failed On Adapter[%d].\thr=%x.\n", __FUNCTION__,m_nDisplayAdapter, hr);
				goto _Failed;
			}
		}
		if (!m_bD3DShared)
		{
			SafeRelease(m_pSurfaceRender);
			if (FAILED(hr = m_pDirect3DDeviceEx->CreateOffscreenPlainSurface(nVideoWidth,
				nVideoHeight,
				nD3DFormat,
				D3DPOOL_DEFAULT,
				&m_pSurfaceRender,
				NULL)))
			{
				if (pRunlog)
					pRunlog->Runlog("%s CreateOffscreenPlainSurface Failed.\thr=%x.\n", __FUNCTION__, hr);
				DxTraceMsg("%s CreateOffscreenPlainSurface Failed.\thr=%x.\n", __FUNCTION__, hr);
				goto _Failed;
			}
#ifdef _DEBUG
			if (m_pSurfaceRender)
			{
				D3DSURFACE_DESC desc;
				m_pSurfaceRender->GetDesc(&desc);
				DxTraceMsg("%s Surface Width = %d\tHeight = %d.\n", __FUNCTION__, desc.Width, desc.Height);
			}
#endif
		}

		//hr = m_pDirect3DDeviceEx->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
		//hr = m_pDirect3DDeviceEx->SetSamplerState(0, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC);
		hr = m_pDirect3DDeviceEx->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS,TRUE);
		hr = m_pDirect3DDeviceEx->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		// 保存参数
		m_nVideoWidth	 = nVideoWidth;
		m_nVideoHeight	 = nVideoHeight;
		m_nD3DFormat	 = nD3DFormat;		
		bSucceed		 = true;
		m_bInitialized	 = true;
		{
			// 以下代码加载背景图片
			IDirect3DSurface9 *pSurfaceBackImage = nullptr;
			D3DXIMAGE_INFO ImageInfo;
			if (m_pszBackImageFileW)
			{
				HRESULT hr = D3DXGetImageInfoFromFileW(m_pszBackImageFileW, &ImageInfo);
				if (SUCCEEDED(hr))
				{
					hr = m_pDirect3DDeviceEx->CreateOffscreenPlainSurface(ImageInfo.Width,
						ImageInfo.Height,
						/*ImageInfo.Format*/
						D3DFMT_X8R8G8B8,
						m_nCurD3DPool,
						&pSurfaceBackImage,
						NULL);
					if (SUCCEEDED(hr))
					{
						hr = D3DXLoadSurfaceFromFileW(
							pSurfaceBackImage,					//目标表面
							NULL,								//目标调色板
							NULL,								//目标矩形,NULL为加载整个表面
							m_pszBackImageFileW,				//文件
							NULL,								//源矩形,NULL为复制整个图片
							D3DX_DEFAULT,						//过滤
							D3DCOLOR_XRGB(0, 0, 0),				//透明色
							&ImageInfo							//源图像信息
							);
						if (SUCCEEDED(hr))
						{
							PresetBackImage(pSurfaceBackImage, ImageInfo, m_d3dpp.hDeviceWindow);
						}
					}
				}
			}
			SafeRelease(pSurfaceBackImage);
		}
_Failed:
		if (!bSucceed)
		{
			DxCleanup();
		}
		return bSucceed;
	}

// 	UINT GetMaxMultiSampleQualityLevel(D3DFORMAT nColorFmt, D3DFORMAT depth_format, bool is_window_mode)
// 	{
// 		if (m_pDirect3D9Ex == 0)
// 			return 0;
// 		DWORD	msqAAQuality = 0;
// 		DWORD	msqAAQuality2 = 0;
// 		if (SUCCEEDED(m_pDirect3D9Ex->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, nColorFmt, is_window_mode, D3DMULTISAMPLE_NONMASKABLE, &msqAAQuality)))
// 		{
// 			if (SUCCEEDED(m_pDirect3D9Ex->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, depth_format, is_window_mode, D3DMULTISAMPLE_NONMASKABLE, &msqAAQuality2)))
// 			{
// 				return __min(msqAAQuality, msqAAQuality2);
// 			}
// 			return 0;
// 		}
// 		return 0;
// 	}
	
	// 矩阵顺转旋转90度
	void MatrixRocate90(byte *pSrc, byte *pDest, int nWidth, int nHeight, int nStride)
	{
		int nRowPos = 0;
		for (int nRow = 0; nRow < nWidth; nRow++)
		{
			int nColPos = 0;
			for (int nCol = nHeight-1; nCol >=0 ; nCol--)
			{// 算法优化，把乘法运算转换为加法运算，运算速度可提高约5~10倍
				//pDest[nRow*nStride + nCol] = pSrc[nCol*nWidth + nRow];
				pDest[nRowPos + nCol] = pSrc[nColPos + nRow];
				nColPos += nWidth;
			}
			nRowPos += nStride;
		}
	}

	// 矩阵顺时针旋转270度，即逆时针旋转90度
	void MatrixRocate270(byte *pSrc, byte *pDest, int nWidth, int nHeight,int nStride)
	{
		int nRowPos = 0;
		for (int nRow = nWidth; nRow >0 ; nRow--)
		{
			int nColPos = 0;
			for (int nCol = 0; nCol < nHeight; nCol++)
			{// 算法优化，把乘法运算转换为加法运算，运算速度可提高约5~10倍
				//pDest[nRow*nStride + nCol] = pSrc[nCol*nWidth + nRow];
				pDest[nRowPos + nCol] = pSrc[nColPos + nRow];
				nColPos += nWidth;
			}
			nRowPos += nStride;
		}
	}

	// 矩阵顺时针旋转180度
	void MatrixRocate180(byte *pSrc, byte *pDest, int nWidth, int nHeight, int nStride)
	{
		int nDstRowPos = 0;
		int nSrcRowPos = nWidth*(nHeight-1);
		for (int nRow = 0; nRow < nHeight; nRow++)
		{
			int nDstColPos = 0;
			int nSrcColPos = nWidth;
			for (int nCol = 0; nCol < nWidth; nCol++)
			{// 算法优化，把乘法运算转换为加减法运算，运算速度可提高约5~10倍
				//pDest[nRow*nStride + nCol] = pSrc[(nHeight - nRow)*nWidth + nWidth - nCol];
				pDest[nDstRowPos + nCol] = pSrc[nSrcRowPos + nWidth - nCol];
				//nDstColPos += nWidth;
			}
			nDstRowPos += nStride;
			nSrcRowPos -= nWidth;
		}
	}

	// YUV420图像顺时针旋转90度,并转为YV12图像
	// pSrcYUV		YUV420图像YUV分量的地址
	// pDestYUV		转换后YV12图像YUV分量的地址
	// nLinesize	YUV420图像的每个分量的数据长度
	// nWidth		YUV420图像的宽度
	// nHeight		YUV420图像的高度
	// nStride		YV12数据目标数据每一行数据的步长
	
	void YUV420Rocate90(byte *pSrcYUV[4], byte *pDestYUV[4], int nLinesize[4], int nWidth, int nHeight,int nStride)
	{
		MatrixRocate90(pSrcYUV[0], pDestYUV[0], nLinesize[0], nHeight,nStride);
		MatrixRocate90(pSrcYUV[1], pDestYUV[2], nLinesize[1], nHeight / 2, nStride/2);
		MatrixRocate90(pSrcYUV[2], pDestYUV[1], nLinesize[2], nHeight / 2,nStride/2);
	}
	// YUV420图像顺时针旋转270度，即逆时针旋转90，并转为YV12图像
	void YUV420Rocate270(byte *pSrcYUV[4], byte *pDestYUV[4], int nLinesize[4], int nWidth, int nHeight,int nStride)
	{
		MatrixRocate270(pSrcYUV[0], pDestYUV[0], nLinesize[0], nHeight,nStride);
		MatrixRocate270(pSrcYUV[1], pDestYUV[2], nLinesize[2], nHeight / 2,nStride/2);
		MatrixRocate270(pSrcYUV[2], pDestYUV[1], nLinesize[1], nHeight / 2,nStride/2);
	}
	// YUV420图像旋转180度，并转为YV12图像
	void YUV420Rocate180(byte *pSrcYUV[4], byte *pDestYUV[4], int nLinesize[4], int nWidth, int nHeight, int nStride)
	{
		MatrixRocate180(pSrcYUV[0], pDestYUV[0], nLinesize[0], nHeight, nStride);
		MatrixRocate180(pSrcYUV[1], pDestYUV[2], nLinesize[2], nHeight / 2, nStride / 2);
		MatrixRocate180(pSrcYUV[2], pDestYUV[1], nLinesize[1], nHeight / 2, nStride / 2);
	}
	virtual inline IDirect3DDevice9Ex *GetD3DDevice()
	{
		return m_pDirect3DDeviceEx;
	}

	virtual inline IDirect3D9Ex *GetD3D9()
	{
		return m_pDirect3D9Ex;
	}

	// D3dDirect9Ex下,取代ResetDevice()函数
	bool  ResetDevice()
	{
		HRESULT hr = S_OK;
		// 使用D3d9DeviceEx时，无须重新创建表面资源
		if (!m_d3dpp.Windowed)
		{
			D3DDISPLAYMODEEX	DispMode;
			DispMode.Size			= sizeof(D3DDISPLAYMODEEX);
			DispMode.Width			= GetSystemMetrics(SM_CXSCREEN);
			DispMode.Height			= GetSystemMetrics(SM_CYSCREEN);
			DispMode.RefreshRate	= m_d3dpp.FullScreen_RefreshRateInHz;
			DispMode.Format			= m_d3dpp.BackBufferFormat;
			DispMode.ScanLineOrdering= D3DSCANLINEORDERING_PROGRESSIVE;
			hr = m_pDirect3DDeviceEx->ResetEx(&m_d3dpp,&DispMode);
		}
		else
			hr = m_pDirect3DDeviceEx->ResetEx(&m_d3dpp,NULL);
		if (FAILED(hr))
		{
			DxTraceMsg("%s IDirect3DDevice9Ex::ResetEx Failed,hr = %08X.\n",__FUNCTION__,hr);
			
			return false;
		}
		return true;
	}

	// D3dDirect9Ex下，该成员不再有效
	virtual bool RestoreDevice()
	{
		return true;
	}
	bool HandelDevLost()
	{
		HRESULT hr = S_OK;
		if (!m_pDirect3DDeviceEx)
			return false;

		hr = m_pDirect3DDeviceEx->CheckDeviceState(m_d3dpp.hDeviceWindow);
		switch(hr)
		{
		case S_OK:
			{
				return true;
			}
			break;
		case S_PRESENT_MODE_CHANGED:
			{
				DxTraceMsg("%s The display mode has changed.\n",__FUNCTION__);
				//assert(false);
				if (!ResetDevice())
				{
					DxCleanup();
					return InitD3D(m_d3dpp.hDeviceWindow, m_nVideoWidth, m_nVideoHeight, m_bFullScreen);
				}
			}
			break;
		case S_PRESENT_OCCLUDED:	// 窗口被其它窗口模式的画面遮挡或全屏画面被最小化,若为全屏窗口，
			// 则此时可停止渲染，直接到收到WM_ACTIVATEAPP时，可重新开始演染
			{
				DxTraceMsg("%s The window is occluded.\n",__FUNCTION__);
			}
			break;
		case D3DERR_DEVICELOST:
			{
				DxTraceMsg("%s The device has been lost.\n",__FUNCTION__);
				return ResetDevice();
			}
			break;
		case D3DERR_DEVICENOTRESET:	// 设备丢失,但不再需要重新创建所有资源
			{
				DxTraceMsg("%s The device is not Reset.\n",__FUNCTION__);
				return ResetDevice();
			}
			break;
		case D3DERR_DEVICEHUNG:		// 需要复位IDirect3DDeviceEx对象,但不需要重建IDirect3DEx对象
			{
				DxTraceMsg("%s The device is hung.\n",__FUNCTION__);
				DxCleanup();
				return InitD3D(m_d3dpp.hDeviceWindow,m_nVideoWidth,m_nVideoHeight,m_bFullScreen);	
			}
			break;
		case D3DERR_DEVICEREMOVED:	// 需要重新创建IDirect3DEx对象
			{
				DxTraceMsg("%s if the device has been removed.\n",__FUNCTION__);
				DxCleanup();
//				SafeRelease(m_pDirect3D9Ex);
// 				hr = m_pDirect3DCreate9Ex(D3D_SDK_VERSION, &m_pDirect3D9Ex);
// 				if (FAILED(hr))
// 					return false;
				return InitD3D(m_d3dpp.hDeviceWindow,m_nVideoWidth,m_nVideoHeight,m_bFullScreen);	
			}
			break;
		default:
			return false;
		}

		return true;
	}

#define RenderTimeout	100 //ms
	bool Render(AVFrame *pAvFrame,int nRocate = 0)
	{
		//DeclareRunTime(5);
		if (!pAvFrame)
			return false;
		
		//SaveRunTime();
		HRESULT hr = -1;		
		D3DLOCKED_RECT SrcRect;
		DWORD dwTNow = timeGetTime();
		if (!HandelDevLost())
		{
			return false;
			//SaveRunTime();
		}
		// HandelDevLost仍无法使用m_pDirect3DDevice，则直接返回false
		if (!m_pDirect3DDeviceEx)
			return false;
		
		switch(pAvFrame->format)
		{
		case  AV_PIX_FMT_DXVA2_VLD:
			{// 硬解码帧，可以直接显示
				if (m_bD3DShared)
				{	
					m_pSurfaceRender = (IDirect3DSurface9 *)pAvFrame->data[3];
				}
				else
				{
					IDirect3DSurface9* pSrcSurface = (IDirect3DSurface9 *)pAvFrame->data[3];
					IDirect3DSurface9* pDstSurface = m_pSurfaceRender;
					D3DSURFACE_DESC SrcSurfaceDesc, DstSurfaceDesc;
					pSrcSurface->GetDesc(&SrcSurfaceDesc);
					pDstSurface->GetDesc(&DstSurfaceDesc);
					hr = pSrcSurface->LockRect(&SrcRect, nullptr, D3DLOCK_READONLY);
					D3DLOCKED_RECT DstRect;
					hr |= pDstSurface->LockRect(&DstRect, NULL, D3DLOCK_DONOTWAIT);
					if (FAILED(hr))
					{
						DxTraceMsg("%s line(%d) IDirect3DSurface9::LockRect failed:hr = %08X.\n",__FUNCTION__,__LINE__,hr);
						return false;
					}
					if (SrcRect.Pitch == DstRect.Pitch)
						gpu_memcpy(DstRect.pBits, SrcRect.pBits, SrcRect.Pitch*DstSurfaceDesc.Height * 3 / 2);
					else
					{
						// Y分量图像
						uint8_t *pSrcY = (uint8_t*)SrcRect.pBits;
						// UV分量图像
						uint8_t *pSrcUV = (uint8_t*)SrcRect.pBits + SrcRect.Pitch * SrcSurfaceDesc.Height;

						uint8_t *pDstY = (uint8_t *)DstRect.pBits;
						uint8_t *pDstUV = (uint8_t *)DstRect.pBits + DstRect.Pitch*DstSurfaceDesc.Height;

						// 复制Y分量
						for (UINT i = 0; i < SrcSurfaceDesc.Height; i++)
							gpu_memcpy(&pDstY[i*DstRect.Pitch], &pSrcY[i*SrcRect.Pitch], SrcSurfaceDesc.Width);
						for (UINT i = 0; i < SrcSurfaceDesc.Height/2; i++)
							gpu_memcpy(&pDstUV[i*DstRect.Pitch], &pSrcUV[i*SrcRect.Pitch], SrcSurfaceDesc.Width);
					
					}
					hr |= m_pSurfaceRender->UnlockRect();
					hr |= pSrcSurface->UnlockRect();
					//DxTraceMsg("hr = %08X.\n", hr);
					if (FAILED(hr))
					{
						DxTraceMsg("%s line(%d) IDirect3DSurface9::UnlockRect failed:hr = %08X.\n",__FUNCTION__,__LINE__,hr);
						return false;
					}
				}

				// 处理截图请求
				TransferSnapShotSurface(pAvFrame);
				// 处理外部分绘制接口
// 				ExternDrawCall(hWnd,pRenderRt);
// 
// 				IDirect3DSurface9 * pBackSurface = NULL;
// 				//m_pDirect3DDeviceEx->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
// 				m_pDirect3DDeviceEx->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
// 				m_pDirect3DDeviceEx->BeginScene();
// 				hr = m_pDirect3DDeviceEx->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackSurface);
// 				D3DSURFACE_DESC desc;
// 
// 				if (FAILED(hr))
// 				{
// 					m_pDirect3DDeviceEx->EndScene();
// 					DxTraceMsg("%s line(%d) IDirect3DDevice9Ex::GetBackBuffer failed:hr = %08X.\n",__FUNCTION__,__LINE__,hr);
// 					return true;
// 				}
// 				pBackSurface->GetDesc(&desc);
// 				RECT dstrt = { 0, 0, desc.Width, desc.Height };
// 				RECT srcrt = { 0, 0, pAvFrame->width, pAvFrame->height };	
// 				if (pClippedRT)
// 				{
// 					CopyRect(&srcrt, pClippedRT);
// 				}
// 				hr = m_pDirect3DDeviceEx->StretchRect(pRenderSurface, &srcrt, pBackSurface, &dstrt, D3DTEXF_NONE/*D3DTEXF_LINEAR*/);
// 				pBackSurface->Release();
// 				m_pDirect3DDeviceEx->EndScene();
				break;
			}
		default:		
			{// 软解码帧，只支持YUV420P格式
				//SaveRunTime();
				TransferSnapShotSurface(pAvFrame);
				D3DLOCKED_RECT d3d_rect;
				D3DSURFACE_DESC Desc;
				//SaveRunTime();
				hr = m_pSurfaceRender->GetDesc(&Desc);
				hr |= m_pSurfaceRender->LockRect(&d3d_rect, NULL, D3DLOCK_DONOTWAIT);
				if (FAILED(hr))
				{
					//DxTraceMsg("%s line(%d) IDirect3DSurface9::LockRect failed:hr = %08X.\n",__FUNCTION__,__LINE__,hr);
					return false;
				}
				if (!nRocate)
				{
					assert(Desc.Width == pAvFrame->width);
					assert(Desc.Height == pAvFrame->height);
				}
				
				//SaveRunTime();
				if ((pAvFrame->format == AV_PIX_FMT_YUV420P ||
					pAvFrame->format == AV_PIX_FMT_YUVJ420P) &&
					Desc.Format == (D3DFORMAT)MAKEFOURCC('Y', 'V', '1', '2'))
				{
					if (nRocate == 0)
						CopyFrameYUV420P(&d3d_rect, Desc.Height, pAvFrame);
					else
					{
						byte *pData[4] = { 0 };
						byte *pDest = (byte *)d3d_rect.pBits;						
						int nStride = d3d_rect.Pitch;
						int nSize = Desc.Height * nStride;
						int nHalfSize = (nSize) >> 1;
						
						pData[0] = pDest;										// Y分量起始地址
						pData[1] = pDest + nSize;								// U分量起始地址
						pData[2] = pData[1] + (size_t)(nHalfSize >> 1);			// V分量起始地址
						switch (nRocate)
						{
						case 1:
						default:
							YUV420Rocate90(pAvFrame->data, pData, pAvFrame->linesize, pAvFrame->width, pAvFrame->height,nStride);
							break;
						case 2:
							YUV420Rocate180(pAvFrame->data, pData, pAvFrame->linesize, pAvFrame->width, pAvFrame->height, nStride);
							break;
						case 3:	
						case 4:
							YUV420Rocate270(pAvFrame->data, pData, pAvFrame->linesize, pAvFrame->width, pAvFrame->height, nStride);
							break;
						}
					}
				}
				else
				{
					if (!m_pPixelConvert)
#if _MSC_VER >= 1600
						m_pPixelConvert = make_shared<PixelConvert>(pAvFrame,Desc.Format);
#else
						m_pPixelConvert = shared_ptr<PixelConvert>(new PixelConvert(pAvFrame,Desc.Format));
#endif 
					m_pPixelConvert->ConvertPixel(pAvFrame);
					if (m_pPixelConvert->GetDestPixelFormat() == AV_PIX_FMT_YUV420P)
						CopyFrameYUV420P(&d3d_rect,Desc.Height, m_pPixelConvert->pFrameNew);
					else					
						memcpy_s((byte *)d3d_rect.pBits, d3d_rect.Pitch*Desc.Height, m_pPixelConvert->pImage, m_pPixelConvert->nImageSize);
				}
				//SaveRunTime();
				hr = m_pSurfaceRender->UnlockRect();
				if (FAILED(hr))
				{
					//DxTraceMsg("%s line(%d) IDirect3DSurface9::UnlockRect failed:hr = %08X.\n",__FUNCTION__,__LINE__,hr);
					return false;
				}
				//SaveRunTime();
				// 处理外部分绘制接口
// 				ExternDrawCall(hWnd,pRenderRt);
// 				SaveRunTime();
// 				IDirect3DSurface9 * pBackSurface = NULL;	
// 				//m_pDirect3DDeviceEx->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
// 				m_pDirect3DDeviceEx->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
// 				m_pDirect3DDeviceEx->BeginScene();
// 				hr = m_pDirect3DDeviceEx->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackSurface);
// 				SaveRunTime();
// 				if (FAILED(hr))
// 				{
// 					m_pDirect3DDeviceEx->EndScene();
// 					//DxTraceMsg("%s line(%d) IDirect3DDevice9Ex::GetBackBuffer failed:hr = %08X.\n",__FUNCTION__,__LINE__,hr);
// 					return true;
// 				}
// 				pBackSurface->GetDesc(&Desc);
// 				RECT dstrt = { 0, 0, Desc.Width, Desc.Height };
// 				RECT srcrt = { 0, 0, pAvFrame->width, pAvFrame->height };
// 				if (pClippedRT)
// 				{
// 					CopyRect(&srcrt, pClippedRT);
// 				}
// 				hr = m_pDirect3DDeviceEx->StretchRect(m_pSurfaceRender, &srcrt, pBackSurface, &dstrt, D3DTEXF_LINEAR);
// 				SaveRunTime();
// 				SafeRelease(pBackSurface);
// 				m_pDirect3DDeviceEx->EndScene();
// 				SaveRunTime();
			}
			break;
		case AV_PIX_FMT_NONE:
			{
				DxTraceMsg("*************************************.\n");
				DxTraceMsg("*	Get a None picture Frame error	*.\n");
				DxTraceMsg("*************************************.\n");
				return true;
			}
			break;
		}
		return true;
		// Present(RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion)
// 		if (hWnd)
// 			hr |= m_pDirect3DDeviceEx->PresentEx(NULL, pRenderRt, hWnd, NULL,0);
// 		else
// 			// (PresentEx)(RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion,DWORD dwFlags)
// 			hr |= m_pDirect3DDeviceEx->PresentEx(NULL, NULL,m_d3dpp.hDeviceWindow, NULL,0);
// 		SaveRunTime();
// 		if (SUCCEEDED(hr))
// 			return true;
// 		else
// 		{
// 			SaveRunTime();
// 			bool bRet =  HandelDevLost();
// 			SaveRunTime();
// 			return bRet;
// 		}
	}

	// 添加一个多边形
	// 操作成功时，返回多边形的索引值，失败返回0
	virtual long AddPolygon(POINT *pPtArray, int nCount, WORD *pInputIndexArray, D3DCOLOR nColor)
	{
		if (!m_pDirect3DDeviceEx)
			return 0;
		PolygonVertex *pVertexArray = new PolygonVertex[nCount];
		for (int i = 0; i < nCount; i++)
		{
			pVertexArray[i].x = (float)pPtArray[i].x;
			pVertexArray[i].y = (float)pPtArray[i].y;
			pVertexArray[i].rhw = 1.0f;
			pVertexArray[i].color = nColor;
		}
		WORD *pIndexArray = new WORD[(nCount - 2) * 3];
		for (int i = 0; i < nCount - 2; i++)
		{
			pIndexArray[i*3] = 0;
			pIndexArray[i * 3 + 1] = pInputIndexArray[i + 1];
			pIndexArray[i * 3 + 2] = pInputIndexArray[i + 2];
		}
		bool bSucceed = false;
		DxPolygonPtr pDxPolygon = make_shared<DxPolygon>();
		pDxPolygon->nVertexCount = nCount - 1;
		pDxPolygon->nTriangleCount = nCount - 2;

		if (FAILED(m_pDirect3DDeviceEx->CreateVertexBuffer(sizeof(PolygonVertex)*nCount, 0, D3DFVF_VERTEX, D3DPOOL_DEFAULT, &pDxPolygon->VertexBuff, NULL)))
			goto __Failure;

		// Fill the vertex buffer.
		void *ptr;
		if (FAILED(pDxPolygon->VertexBuff->Lock(0, sizeof(PolygonVertex)*nCount, (void**)&ptr, 0)))
			return false;

		memcpy(ptr, pVertexArray, sizeof(PolygonVertex)*nCount);
		pDxPolygon->VertexBuff->Unlock();

		INT nIndexSize = (nCount - 2) * 3 * sizeof(WORD);
		void *pIndex_ptr;
		if (FAILED(m_pDirect3DDeviceEx->CreateIndexBuffer(nIndexSize, 0, D3DFMT_INDEX16, D3DPOOL_DEFAULT, &pDxPolygon->IndexBuff, NULL)))
			goto __Failure;

		if (FAILED(pDxPolygon->IndexBuff->Lock(0, nIndexSize, (void**)&pIndex_ptr, 0)))
			goto __Failure;
		memcpy(pIndex_ptr, pIndexArray, nIndexSize);
		pDxPolygon->IndexBuff->Unlock();
		bSucceed = true;

__Failure:
		SafeDeleteArray(pVertexArray);
		SafeDeleteArray(pIndexArray);
		if (bSucceed)
		{
			m_pCsMapPolygon->Lock();
			m_mapPolygon.insert(pair<long, DxPolygonPtr >((long)pDxPolygon.get(), pDxPolygon));
			m_pCsMapPolygon->Unlock();
			return (long)pDxPolygon.get();
		}
		else
			return 0;
	}

	// 添加一组线条坐标
	// 返回值为索条索引值，删除该线条时需要用到这个索引值
	// 添加失败时返回0
	virtual long AddD3DLineArray(POINT *pPointArray, int nCount, float fWidth, D3DCOLOR nColor)
	{
		if (!m_pD3DXLine)
		{
			// 创建Direct3D线对象  
			if (FAILED(D3DXCreateLine(m_pDirect3DDeviceEx, &m_pD3DXLine)))
			{
				return 0;
			}
		}
		D3DLineArrayPtr pLineArray = make_shared<D3DLineArray>();
		pLineArray->fWidth = fWidth;
		pLineArray->nColor = nColor;
		pLineArray->nCount = nCount;
		pLineArray->pLineArray = new D3DXVECTOR2[nCount];
		for (int i = 0; i < nCount; i++)
		{
			pLineArray->pLineArray[i].x = (float)pPointArray[i].x;
			pLineArray->pLineArray[i].y = (float)pPointArray[i].y;
		}
		m_pCsListLine->Lock();
		m_listLine.push_back(pLineArray);
		m_pCsListLine->Unlock();
		return (long)pLineArray.get();
	}

	bool Present(HWND hWnd = NULL, RECT *pClippedRT = nullptr, RECT *pRenderRt = nullptr)
	{
		HWND hRenderWnd = m_d3dpp.hDeviceWindow;
		//DeclareRunTime(5);
		if (hWnd)
			hRenderWnd = hWnd;
		if (!IsNeedRender(hRenderWnd))
			return true;
		HRESULT hr = S_OK;
		
		//SaveRunTime();
		IDirect3DSurface9 * pBackSurface = NULL;
		m_pDirect3DDeviceEx->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
		m_pDirect3DDeviceEx->BeginScene();

		hr = m_pDirect3DDeviceEx->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBackSurface);
		//SaveRunTime();
		if (FAILED(hr))
		{
			m_pDirect3DDeviceEx->EndScene();
			return true;
		}
		D3DSURFACE_DESC DescDst,DescSrc;
		pBackSurface->GetDesc(&DescDst);
		m_pSurfaceRender->GetDesc(&DescSrc);
		RECT dstrt = { 0, 0, DescDst.Width, DescDst.Height };
		RECT srcrt = { 0, 0, m_nVideoWidth, m_nVideoHeight };

		if (pClippedRT)
		{
			CopyRect(&srcrt, pClippedRT);
		}

		hr = m_pDirect3DDeviceEx->StretchRect(m_pSurfaceRender, &srcrt, pBackSurface, &dstrt, D3DTEXF_LINEAR);
		
		// 处理外部分绘制接口
		ExternDrawCall(hWnd, pBackSurface,pRenderRt);
		SafeRelease(pBackSurface);
		
		ProcessD3DXDraw(hWnd);

		m_pDirect3DDeviceEx->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
		m_pDirect3DDeviceEx->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		m_pCsMapPolygon->Lock();
		for (auto it = m_mapPolygon.begin(); it != m_mapPolygon.end();it ++)
		{
			m_pDirect3DDeviceEx->SetStreamSource(0, it->second->VertexBuff, 0, sizeof(PolygonVertex));
			m_pDirect3DDeviceEx->SetFVF(D3DFVF_VERTEX);
			m_pDirect3DDeviceEx->SetIndices(it->second->IndexBuff);
			m_pDirect3DDeviceEx->DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, 0, it->second->nVertexCount, 0, it->second->nTriangleCount);
		}
		m_pCsMapPolygon->Unlock();
	

		m_pDirect3DDeviceEx->EndScene();
		
		if (hRenderWnd)
			hr |= m_pDirect3DDeviceEx->PresentEx(NULL, pRenderRt, hRenderWnd, NULL, 0);
		//else
		//	// (PresentEx)(RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion,DWORD dwFlags)
		//	hr |= m_pDirect3DDeviceEx->PresentEx(NULL, pRenderRt, m_d3dpp.hDeviceWindow, NULL, 0);
		//SaveRunTime();
		if (SUCCEEDED(hr))
			return true;
		else
		{
			//SaveRunTime();
			bool bRet = HandelDevLost();
			//SaveRunTime();
			return bRet;
		}
	}

	// 1.检查指定的表面像素格式，是否在指定的适配器类型、适配器像素格式下可用。
	// GetAdapterDisplayMode,CheckDeviceType的应用
	bool GetBackBufferFormat(D3DDEVTYPE deviceType, BOOL bWindow, D3DFORMAT &fmt)
	{
		if (m_pDirect3D9 == NULL)
			return false;

		D3DDISPLAYMODE adapterMode;
		m_pDirect3D9Ex->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &adapterMode);
		if (D3D_OK != m_pDirect3D9Ex->CheckDeviceType(D3DADAPTER_DEFAULT, deviceType, adapterMode.Format, fmt, bWindow))
			fmt = adapterMode.Format;
		return true;
	}

	// 2.根据适配器类型，获取顶点运算(变换和光照运算)的格式
	// D3DCAPS9结构体，GetDeviceCaps的应用
	bool GetDisplayVertexType(D3DDEVTYPE deviceType, int &nVertexType)
	{
		if (m_pDirect3D9 == NULL)
			return false;

		D3DCAPS9 caps;
		m_pDirect3D9Ex->GetDeviceCaps(D3DADAPTER_DEFAULT, deviceType, &caps);
		if (caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT)
			nVertexType = D3DCREATE_HARDWARE_VERTEXPROCESSING;
		else
			nVertexType = D3DCREATE_SOFTWARE_VERTEXPROCESSING;
		return true;

	}



	// 3.输出显卡信息,Description描述，厂商型号，Dircet3D的驱动Driver版本号，显卡的唯一标识号：DeviceIdentifier
	// GetAdapterCount()，GetAdapterIdentifier的使用。
	void PrintDisplayInfo()
	{
		if (m_pDirect3D9Ex == NULL)
			return;

		D3DADAPTER_IDENTIFIER9 adapterID; // Used to store device info
		DWORD dwDisplayCount = m_pDirect3D9Ex->GetAdapterCount();
		for (DWORD i = 0; i < dwDisplayCount; i++)
		{
			if (m_pDirect3D9Ex->GetAdapterIdentifier(i/*D3DADAPTER_DEFAULT*/, 0, &adapterID) != D3D_OK)
				return;

			DxTraceMsg("[%d]Driver: %s.\n", i,adapterID.Driver);
			DxTraceMsg("[%d]Description: %s\n", i, adapterID.Description);
			DxTraceMsg("[%d]Device Name: %s\n", i, adapterID.DeviceName);
			DxTraceMsg("[%d]Vendor id:%4x\n", i, adapterID.VendorId);
			DxTraceMsg("[%d]Device id: %4x\n", i, adapterID.DeviceId);
			DxTraceMsg("[%d]Product: %x\n", i, HIWORD(adapterID.DriverVersion.HighPart));
			DxTraceMsg("[%d]Version:%x\n", i, LOWORD(adapterID.DriverVersion.HighPart));
			DxTraceMsg("[%d]SubVersion: %x\n", i, HIWORD(adapterID.DriverVersion.LowPart));
			DxTraceMsg("[%d]Build: %x %d.%d.%d.%d\n", i, LOWORD(adapterID.DriverVersion.LowPart),
				HIWORD(adapterID.DriverVersion.HighPart),
				LOWORD(adapterID.DriverVersion.HighPart),
				HIWORD(adapterID.DriverVersion.LowPart),
				LOWORD(adapterID.DriverVersion.LowPart));
			DxTraceMsg("[%d]SubSysId: %x\n, Revision: %x\n,GUID %s\n, WHQLLevel:%d\n", i,
				adapterID.SubSysId,
				adapterID.Revision,
				StringFromGUID(&adapterID.DeviceIdentifier),
				adapterID.WHQLLevel);
		}
	}

	// 4.输出指定Adapter，显卡像素模式(不会与缓存表面格式做兼容考虑)的显卡适配器模式信息
	// GetAdapterModeCount,EnumAdapterModes的使用
	void PrintDisplayModeInfo(D3DFORMAT fmt)
	{
		if (m_pDirect3D9Ex == NULL)
		{
			DxTraceMsg("%s Direct3D9 not initialized.\n", __FUNCTION__);
			return;
		}
		// 显卡适配器模式的个数，主要是分辨率的差异
		DWORD nAdapterModeCount = m_pDirect3D9Ex->GetAdapterModeCount(D3DADAPTER_DEFAULT, fmt);
		if (nAdapterModeCount == 0)
		{
			DxTraceMsg("%s D3DFMT_格式：%x不支持", __FUNCTION__, fmt);
		}
		for (DWORD i = 0; i < nAdapterModeCount; i++)
		{
			D3DDISPLAYMODE mode;
			if (D3D_OK == m_pDirect3D9->EnumAdapterModes(D3DADAPTER_DEFAULT, fmt, i, &mode))
				DxTraceMsg("D3DDISPLAYMODE info, width:%u,height:%u, freshRate:%u, Format:%d \n",
				mode.Width,
				mode.Height,
				mode.RefreshRate,
				mode.Format);
		}
	}

	// 5.对于指定的资源类型，检查资源的使用方式，资源像素格式，在默认的显卡适配器下是否支持
	// GetAdapterDisplayMode，CheckDeviceFormat的使用
	bool CheckResourceFormat(DWORD nSrcUsage, D3DRESOURCETYPE srcType, D3DFORMAT srcFmt)
	{
		if (m_pDirect3D9 == NULL)
			return false;

		D3DDISPLAYMODE displayMode;
		m_pDirect3D9Ex->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &displayMode);
		if (D3D_OK == m_pDirect3D9Ex->CheckDeviceFormat(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, displayMode.Format, nSrcUsage, srcType, srcFmt))
			return true;
		return false;
	}

	// 6.对指定的表面像素格式，窗口模式，和显卡像素模式；检查对指定的多重采样类型支持不，且返回质量水平等级
	// CheckDeviceMultiSampleType的应用
	bool CheckMultiSampleType(D3DFORMAT surfaceFmt, BOOL bWindow, D3DMULTISAMPLE_TYPE &eSampleType, DWORD *pQualityLevel)
	{
		//变量MultiSampleType的值设为D3DMULTISAMPLE_NONMASKABLE，就必须设定成员变量MultiSampleQuality的质量等级值
		for (int i = eSampleType; i >= D3DMULTISAMPLE_NONE; i--)
		{
			if (SUCCEEDED(m_pDirect3D9Ex->CheckDeviceMultiSampleType(D3DADAPTER_DEFAULT/*caps.AdapterOrdinal*/,
				D3DDEVTYPE_HAL/*caps.DeviceType*/,
				surfaceFmt,
				bWindow,
				(D3DMULTISAMPLE_TYPE)i,
				pQualityLevel)))
			{
				eSampleType = (D3DMULTISAMPLE_TYPE)i;
				return true;
			}
		}

		return false;
	}

	// 7.根据显卡适配器和目标缓存类型，检查指定深度缓存的格式是否支持
	// CheckDepthStencilMatch的应用
	bool CheckDepthBufferFormt(D3DFORMAT targetBufferFmt, D3DFORMAT depthFmt)
	{
		if (m_pDirect3D9 == NULL)
			return false;

		D3DDISPLAYMODE mode;
		m_pDirect3D9Ex->GetAdapterDisplayMode(D3DADAPTER_DEFAULT, &mode);
		if (D3D_OK == m_pDirect3D9->CheckDepthStencilMatch(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, mode.Format, targetBufferFmt, depthFmt))
			return true;
		return false;
	}
	// 解码抓图，把Surface中的图像数据保存到文件中，此截图得到的图像是原始的图像
	virtual bool SaveSurfaceToFileW(WCHAR *szFilePath, D3DXIMAGE_FILEFORMAT D3DImageFormat = D3DXIFF_JPG)
	{
		if (!m_pDirect3DDeviceEx ||
			!m_pSurfaceRender)
			return false;
		if (!szFilePath || wcslen(szFilePath) <= 0)
			return false;

		SetEvent(m_hEventSnapShot);
		m_bSnapFlag = true;

		CAutoLock lock(&m_csSnapShot);
		m_D3DXIFF = D3DImageFormat;
		HRESULT hr = S_OK;
		if (!m_pSnapshotSurface)
		{
			HRESULT hr = m_pDirect3DDeviceEx->CreateOffscreenPlainSurface(m_nVideoWidth,
				m_nVideoHeight,
				D3DFMT_A8R8G8B8,
				D3DPOOL_SYSTEMMEM,
				&m_pSnapshotSurface,
				NULL);
			if (FAILED(hr))
			{
				DxTraceMsg("%s IDirect3DSurface9::GetDesc failed,hr = %08X.\n", __FUNCTION__, hr);
				return false;
			}
		}

		wcscpy(m_szSnapShotPath, szFilePath);
		// 截图数据尚未就绪,则置信截图事件
		if (WaitForSingleObject(m_hEventFrameReady, 1000) == WAIT_TIMEOUT)
			return false;
		// 截图数据已就绪			

		hr = D3DXSaveSurfaceToFileW(szFilePath, m_D3DXIFF, m_pSnapshotSurface, NULL, NULL);
		if (FAILED(hr))
		{
			DxTraceMsg("%s D3DXSaveSurfaceToFile Failed,hr = %08X.\n", __FUNCTION__, hr);
			return false;
		}
		m_bSnapFlag = false;
		return true;
	}
};
typedef shared_ptr<CDxSurfaceEx> CDxSurfaceExPtr;

