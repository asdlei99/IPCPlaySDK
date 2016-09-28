
#pragma once
#include "Common.h"
#ifdef _DEBUG
#define _MaxTimeCount	50
/// @brief	����׷�ٴ���ִ��ʱ����࣬��Ҫ���ڵ���
/// @remark ��Щʱ�䣬Ҫ׷�������Щ�����ִ��ʱ�䳬��Ԥ�ڣ������ÿִ��һ�ξ�����Ļ���
/// ��Դ�����������Ӱ��,���Ҫ����һ��������������Ļ����������Ӱ�콵�͵����Խ��ܵĳ̶�
/// @code Sample:
///     ��ʼ�������
///		TimeTrace DecodeTimeTrace("DecodeTime", __FUNCTION__);
///		....
///		��¼����ʱ��
///		DecodeTimeTrace.SaveTime(dfDecodeTimespa);
///		if (DecodeTimeTrace.IsFull())
///			DecodeTimeTrace.OutputTime(0.005f);

struct TimeTrace
{
	char szName[128];
	char szFunction[128];
	double dfTimeArray[_MaxTimeCount];
	int	   nTimeCount;
	double dfInputTime;
	int nMaxCount;
private:
	explicit TimeTrace()
	{
	}
public:
	TimeTrace(char *szNameT, char *szFunctionT, int nMaxCount = _MaxTimeCount)
	{
		ZeroMemory(this, sizeof(TimeTrace));
		strcpy(szName, szNameT);
		strcpy(szFunction, szFunctionT);
		this->nMaxCount = nMaxCount;
	}
	void Zero()
	{
		ZeroMemory(this, sizeof(TimeTrace));
	}
	inline bool IsFull()
	{
		return (nTimeCount >= nMaxCount);
	}
	inline void SaveTime()
	{
		if (dfInputTime != 0.0f)
			dfTimeArray[nTimeCount++] = GetExactTime() - dfInputTime;
		dfInputTime = GetExactTime();
	}
	inline void SaveTime(double dfTimeSpan)
	{
		dfTimeArray[nTimeCount++] = dfTimeSpan;
	}
	void OutputTime(float fMaxTime = 0.0f, bool bReset = true)
	{
		char szOutputText[1024] = { 0 };
		double dfAvg = 0.0f;
		int nCount = 0;
		if (nTimeCount < 1)
			return;
		TraceMsgA("%s %s Interval:\n", szFunction, szName);
		for (int i = 0; i < nTimeCount; i++)
		{
			if (fMaxTime > 0)
			{
				if (dfTimeArray[i] >= fMaxTime)
				{
					sprintf(&szOutputText[strlen(szOutputText)], "%.3f\t", dfTimeArray[i]);
					nCount++;
				}
			}
			else
			{
				sprintf(&szOutputText[strlen(szOutputText)], "%.3f\t", dfTimeArray[i]);
				nCount++;
			}

			dfAvg += dfTimeArray[i];
			if ((nCount + 1) % 25 == 0)
			{
				TraceMsgA("%s %s\n", szFunction, szOutputText);
				ZeroMemory(szOutputText, 1024);
			}
		}
		dfAvg /= nTimeCount;
		//if (dfAvg >= fMaxTime)
		TraceMsgA("%s Avg %s = %.6f.\n", szFunction, szName, dfAvg);
		if (bReset)
			nTimeCount = 0;
	}
};
#endif