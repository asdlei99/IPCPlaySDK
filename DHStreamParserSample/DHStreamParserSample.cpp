// DHStreamParserSample.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <memory>
#include <list>
#include "svacparser/DhStreamParser.h"

enum IPC_FRAME_TYPE{
	IPC_GOV_FRAME = -1, ///< GOV֡
	IPC_IDR_FRAME = 1,  ///< IDR֡��
	IPC_I_FRAME,        ///< I֡��
	IPC_P_FRAME,        ///< P֡��
	IPC_B_FRAME,        ///< B֡��
	IPC_JPEG_FRAME,     ///< JPEG֡
	IPC_711_ALAW,       ///< 711 A�ɱ���֡
	IPC_711_ULAW,       ///< 711 U�ɱ���֡
	IPC_726,            ///< 726����֡
	IPC_AAC,            ///< AAC����֡��
	IPC_MAX,
} ;

using namespace std;
using namespace std::tr1;
typedef shared_ptr <DH_FRAME_INFO> DH_FRAME_INFO_PTR;
list<DH_FRAME_INFO_PTR> FrameList;
shared_ptr<DhStreamParser> pDHStreamParser = nullptr;
int _tmain(int argc, _TCHAR* argv[])
{
	if (!pDHStreamParser)
	{
		pDHStreamParser = make_shared<DhStreamParser>();
	}


}


int DataCallBack(unsigned char *pBuffer, unsigned long nLength, void *pUser)
{
	pDHStreamParser->InputData(pBuffer, nLength);
	DH_FRAME_INFO *pDHFrame = nullptr;
	do
	{
		pDHFrame = pDHStreamParser->GetNextFrame();
		if (!pDHFrame)	// �Ѿ�û�����ݿ��Խ�������ֹѭ��
			break;
		if (pDHFrame->nType != DH_FRAME_TYPE_VIDEO)
			break;
// 		int nStatus = InputStream(pDHFrame->pContent,
// 			pDHFrame->nSubType == DH_FRAME_TYPE_VIDEO_I_FRAME ? IPC_I_FRAME : IPC_P_FRAME,
// 			pDHFrame->nLength,
// 			pDHFrame->nRequence,
// 			(time_t)pDHFrame->nTimeStamp);
// 		if (0 == nStatus)
// 		{
// 			pDHFrame = nullptr;
// 			continue;
// 		}
// 		else
// 			return nStatus;

	} while (true);
	return 0;
}