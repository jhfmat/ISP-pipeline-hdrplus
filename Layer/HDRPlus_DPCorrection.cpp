#include "HDRPlus_DPCorrection.h"
/*
获取块里面Max和min，如果中心值Y大于max+thre则Y= max Y+thre<min Y=min 返回Y值作为去除坏点后的值
像素值在这个块里面要小于等于最大的大于等于最小的
*/
int CHDRPlus_DPCorrection::ProcessBlock(unsigned int nBlock[][5],unsigned int nMax[5], unsigned int nMin[5])
{
	unsigned int Y, Max, Min;
	bool bDPC = false;
	Y = nBlock[1][2];
	int nWhitePointCThre = m_nWhitePointCThre;
	int nBlackPointCThre = m_nBlackPointCThre;
	if (Y >= m_nBLC)
	{
		nWhitePointCThre += ((Y - m_nBLC)*m_nWhitePointLRatioT) >> 8;
		nBlackPointCThre += ((Y - m_nBLC)*m_nBlackPointLRatioT) >> 8;
	}
	Max = MAX2(MAX2(nMax[0], nMax[4]), MAX2(nBlock[0][2], nBlock[2][2]));
	Min = MIN2(MIN2(nMin[0], nMin[4]), MIN2(nBlock[0][2], nBlock[2][2]));
	if (m_bWhitePointCEnable == 1 && Y > Max + nWhitePointCThre)//nWhitePointCThre越大则进来少去白点少
	{
		Y = Max;
	}
	if (m_bBlackPointCEnable == 1 && Y + nBlackPointCThre < Min)//nBlackPointCThre越大则进来的少去黑点少
	{
		Y = Min;
	}
	return Y;
}
void CHDRPlus_DPCorrection::ProcessLine(unsigned short *pInLines[], unsigned short *pOutLine, int nWidth)
{
	unsigned short *pIn[3];
	unsigned int nBlock[3][5];
	unsigned int nSum[5];
	unsigned int nMax[5];
	unsigned int nMin[5];
	for (int i = 0; i < 3; i++)
	{
		pIn[i] = pInLines[i * 2];
		nBlock[i][0] = pIn[i][2];
		nBlock[i][1] = pIn[i][3];
		nBlock[i][2] = *(pIn[i]++);
		nBlock[i][3] = *(pIn[i]++);
	}
	for (int i = 0; i < 4; i++)
	{
		nSum[i] = nBlock[0][i] + nBlock[1][i] + nBlock[2][i];
		if (m_bWhitePointCEnable == 1)
		{
			nMax[i] = MAX2(nBlock[0][i], MAX2(nBlock[1][i], nBlock[2][i]));
		}
		if (m_bBlackPointCEnable == 1)
		{
			nMin[i] = MIN2(nBlock[0][i], MIN2(nBlock[1][i], nBlock[2][i]));
		}
	}
	int x = 0;
	for (; x < nWidth - 2; x++)
	{
		for (int i = 0; i < 3; i++)
		{
			nBlock[i][4] = *(pIn[i]++);
			nSum[4] = nBlock[0][4] + nBlock[1][4] + nBlock[2][4];
			if (m_bWhitePointCEnable == 1)
			{
				nMax[4] = MAX2(nBlock[0][4], MAX2(nBlock[1][4], nBlock[2][4]));
			}
			if (m_bBlackPointCEnable == 1)
			{
				nMin[4] = MIN2(nBlock[0][4], MIN2(nBlock[1][4], nBlock[2][4]));
			}
		}
		*(pOutLine++) = (unsigned short)ProcessBlock(nBlock, nMax,  nMin);
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				nBlock[i][j] = nBlock[i][j + 1];
			}
		}
		for (int j = 0; j < 4; j++)
		{
			nSum[j] = nSum[j + 1];
			if (m_bWhitePointCEnable == 1)
			{
				nMax[j] = nMax[j + 1];
			}
			if (m_bBlackPointCEnable == 1)
			{
				nMin[j] = nMin[j + 1];
			}
		}
	}
	for (; x < nWidth; x++)
	{
		for (int i = 0; i < 3; i++)
		{
			nBlock[i][4] = nBlock[i][0];
			nSum[4] = nSum[0];
			if (m_bWhitePointCEnable == 1)
			{
				nMax[4] = nMax[0];
			}
			if (m_bBlackPointCEnable == 1)
			{
				nMin[4] = nMin[0];
			}
		}
		*(pOutLine++) = (unsigned short)ProcessBlock(nBlock, nMax, nMin);
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				nBlock[i][j] = nBlock[i][j + 1];
			}
		}
		for (int j = 0; j < 4; j++)
		{
			nSum[j] = nSum[j + 1];
			if (m_bWhitePointCEnable == 1)
			{
				nMax[j] = nMax[j + 1];
			}
			if (m_bBlackPointCEnable == 1)
			{
				nMin[j] = nMin[j + 1];
			}
		}
	}
}
bool CHDRPlus_DPCorrection::Forward(MultiUshortImage *pInImage, MultiUshortImage *pOutImage, TGlobalControl *pControl)
{
	int nWidth = pInImage->GetImageWidth();
	int nHeight = pInImage->GetImageHeight();
	unsigned short *pInLines[5];
	unsigned short *pBuffer = NULL;
	if (pOutImage->GetImageWidth() != nWidth || pOutImage->GetImageHeight() != nHeight)
	{
		if (!pOutImage->CreateImage(nWidth, nHeight, pInImage->m_nRawBits))return false;
	}
	pOutImage->CopyParameters(pInImage);
	pOutImage->m_nRawBLC = m_nBLC = pControl->nBLC;
	pOutImage->m_nRawMAXS = m_nMAXS = pControl->nWP;
	int nProcs = omp_get_num_procs();
	pBuffer = new unsigned short[nWidth * 5 * nProcs];
	if (pBuffer == NULL)return false;
	int loop = 0;
#pragma omp parallel for num_threads(nProcs)  firstprivate(loop) private(pInLines)
	for (int y = 0; y < nHeight; y++)
	{
		int nThreadId = omp_get_thread_num();
		if (loop == 0)
		{
			pInLines[0] = pBuffer + nWidth * 5 * nThreadId;
			pInLines[1] = pBuffer + nWidth * 5 * nThreadId + nWidth * 1;
			pInLines[2] = pBuffer + nWidth * 5 * nThreadId + nWidth * 2;
			pInLines[3] = pBuffer + nWidth * 5 * nThreadId + nWidth * 3;
			pInLines[4] = pBuffer + nWidth * 5 * nThreadId + nWidth * 4;
			if (y == 0)
			{
				memcpy(pInLines[0], pInImage->GetImageLine(2), nWidth * sizeof(unsigned short));
				memcpy(pInLines[1], pInImage->GetImageLine(3), nWidth * sizeof(unsigned short));
				memcpy(pInLines[2], pInImage->GetImageLine(0), nWidth * sizeof(unsigned short));
				memcpy(pInLines[3], pInImage->GetImageLine(1), nWidth * sizeof(unsigned short));
			}
			else
			{
				memcpy(pInLines[0], pInImage->GetImageLine(y - 2), nWidth * sizeof(unsigned short));
				memcpy(pInLines[1], pInImage->GetImageLine(y - 1), nWidth * sizeof(unsigned short));
				memcpy(pInLines[2], pInImage->GetImageLine(y + 0), nWidth * sizeof(unsigned short));
				memcpy(pInLines[3], pInImage->GetImageLine(y + 1), nWidth * sizeof(unsigned short));
			}
			loop++;
		}
		if (y < nHeight - 2)
		{
			memcpy(pInLines[4], pInImage->GetImageLine(y + 2), nWidth * sizeof(unsigned short));
		}
		else
		{
			pInLines[4] = pInLines[0];
		}
		ProcessLine(pInLines, pOutImage->GetImageLine(y), nWidth);
		if (y < nHeight - 2)
		{
			unsigned short *pTemp = pInLines[0];
			for (int i = 0; i < 4; i++)
			{
				pInLines[i] = pInLines[i + 1];
			}
			pInLines[4] = pTemp;
		}
		else
		{
			for (int i = 0; i < 4; i++)
			{
				pInLines[i] = pInLines[i + 1];
			}
		}
	}
	delete[] pBuffer;
	return true;
}