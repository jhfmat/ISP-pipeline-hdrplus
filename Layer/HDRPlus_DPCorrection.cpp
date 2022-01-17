#include "HDRPlus_DPCorrection.h"
int CHDRPlus_DPCorrection::ProcessWindow(int nWindow[][5],int nMax[5], int nMin[5])
{
	int Y, Max, Min;
	bool bDPC = false;
	Y = nWindow[1][2];
	int nWPCThre = m_nWPCThre;
	int nBPCThre = m_nBPCThre;
	if (Y >= m_nBLC)
	{
		nWPCThre += ((Y - m_nBLC)*m_nWPCRatioT) >> 8;
		nBPCThre += ((Y - m_nBLC)*m_nBPCRatioT) >> 8;
	}
	Max = MAX2(MAX2(nMax[0], nMax[4]), MAX2(nWindow[0][2], nWindow[2][2]));
	Min = MIN2(MIN2(nMin[0], nMin[4]), MIN2(nWindow[0][2], nWindow[2][2]));
	if (m_bWPCEnable == 1 && Y > Max + nWPCThre)
	{
		Y = Max;
	}
	if (m_bBPCEnable == 1 && Y + nBPCThre < Min)
	{
		Y = Min;
	}
	return Y;
}
void CHDRPlus_DPCorrection::ProcessLine(unsigned short *pInLines[], unsigned short *pOutLine, int nWidth)
{
	unsigned short *pIn[3];
	int nWindow[3][5];
	int nSum[5];
	int nMax[5];
	int nMin[5];
	for (int i = 0; i < 3; i++)
	{
		pIn[i] = pInLines[i * 2];
		nWindow[i][0] = pIn[i][2];
		nWindow[i][1] = pIn[i][3];
		nWindow[i][2] = *(pIn[i]++);
		nWindow[i][3] = *(pIn[i]++);
	}
	for (int i = 0; i < 4; i++)
	{
		nSum[i] = nWindow[0][i] + nWindow[1][i] + nWindow[2][i];
		if (m_bWPCEnable == 1)
		{
			nMax[i] = MAX2(nWindow[0][i], MAX2(nWindow[1][i], nWindow[2][i]));
		}
		if (m_bBPCEnable == 1)
		{
			nMin[i] = MIN2(nWindow[0][i], MIN2(nWindow[1][i], nWindow[2][i]));
		}
	}
	int x = 0;
	for (; x < nWidth - 2; x++)
	{
		for (int i = 0; i < 3; i++)
		{
			nWindow[i][4] = *(pIn[i]++);
			nSum[4] = nWindow[0][4] + nWindow[1][4] + nWindow[2][4];
			if (m_bWPCEnable == 1)
			{
				nMax[4] = MAX2(nWindow[0][4], MAX2(nWindow[1][4], nWindow[2][4]));
			}
			if (m_bBPCEnable == 1)
			{
				nMin[4] = MIN2(nWindow[0][4], MIN2(nWindow[1][4], nWindow[2][4]));
			}
		}
		*(pOutLine++) = (unsigned short)ProcessWindow(nWindow, nMax,  nMin);
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				nWindow[i][j] = nWindow[i][j + 1];
			}
		}
		for (int j = 0; j < 4; j++)
		{
			nSum[j] = nSum[j + 1];
			if (m_bWPCEnable == 1)
			{
				nMax[j] = nMax[j + 1];
			}
			if (m_bBPCEnable == 1)
			{
				nMin[j] = nMin[j + 1];
			}
		}
	}
	for (; x < nWidth; x++)
	{
		for (int i = 0; i < 3; i++)
		{
			nWindow[i][4] = nWindow[i][0];
			nSum[4] = nSum[0];
			if (m_bWPCEnable == 1)
			{
				nMax[4] = nMax[0];
			}
			if (m_bBPCEnable == 1)
			{
				nMin[4] = nMin[0];
			}
		}
		*(pOutLine++) = (unsigned short)ProcessWindow(nWindow, nMax, nMin);
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				nWindow[i][j] = nWindow[i][j + 1];
			}
		}
		for (int j = 0; j < 4; j++)
		{
			nSum[j] = nSum[j + 1];
			if (m_bWPCEnable == 1)
			{
				nMax[j] = nMax[j + 1];
			}
			if (m_bBPCEnable == 1)
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
	pOutImage->m_nRawMAXS = m_nMAXS =65535;
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