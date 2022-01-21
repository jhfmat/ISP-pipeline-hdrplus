#include "HDRPlus_GammaCorrect.h"
void CHDRPlus_GammaCorrect::Forward(MultiUshortImage *pRGBImage, TGlobalControl *pControl)
{
	m_nMin = pControl->nBLC;
	m_nMax = pControl->nWP;
	int nWidth = pRGBImage->GetImageWidth();
	int nHeight = pRGBImage->GetImageHeight();
	int cutoff = 200;                   // ceil(0.00304 * UINT16_MAX)
	float gamma_toe = 12.92;
	float gamma_pow = 0.416667;         // 1 / 2.4
	float gamma_fac = 680.552897;       // 1.055 * UINT16_MAX ^ (1 - gamma_pow);
	float gamma_con = -3604.425;        // -0.055 * UINT16_MAX
	unsigned short *GammaTable=new unsigned short[m_nMax+1];
	for (int k = 0; k < m_nMax+1; k++)
	{
		long  int tmp;
		if (k < cutoff)
		{
			tmp = gamma_toe * k;
		}
		else
		{
			tmp = gamma_fac * pow(k, gamma_pow) + gamma_con;
		}
		GammaTable[k] = CLIP(tmp, m_nMin, m_nMax);
	}
#pragma omp parallel for
	for (int y = 0; y < nHeight; y++)
	{
		unsigned short *pRGBLine = pRGBImage->GetImageLine(y);
		for (int x = 0; x < nWidth; x++)
		{
			pRGBLine[0] = GammaTable[pRGBLine[0]];
			pRGBLine[1] = GammaTable[pRGBLine[1]];
			pRGBLine[2] = GammaTable[pRGBLine[2]];
			pRGBLine += 3;
		}
	}
	delete[]GammaTable;
}