#include "HDRPlus_ColorCorect.h"
void CHDRPlus_ColorCorect::Forward(MultiUshortImage *pRGBImage, TGlobalControl *pControl)
{
	m_nMin = pControl->nBLC;
	m_nMax = pControl->nWP;
	int nWidth = pRGBImage->GetImageWidth();
	int nHeight = pRGBImage->GetImageHeight();
	short Sccm[3][3];
	for (int n = 0; n < 3; n++)
	{
		for (int m = 0; m < 3; m++)
		{
			Sccm[n][m] = pControl->nCCM[n][m] * 4096;
		}
	}
#pragma omp parallel for 
	for (int y = 0; y < nHeight; y++)
	{
		unsigned short *pRGBline = pRGBImage->GetImageLine(y);
		for (int x = 0; x < nWidth; x++)
		{
			int R = (int)(pRGBline[0] * Sccm[0][0] + pRGBline[1] * Sccm[0][1] + pRGBline[2] * Sccm[0][2]);
			int G = (int)(pRGBline[0] * Sccm[1][0] + pRGBline[1] * Sccm[1][1] + pRGBline[2] * Sccm[1][2]);
			int B = (int)(pRGBline[0] * Sccm[2][0] + pRGBline[1] * Sccm[2][1] + pRGBline[2] * Sccm[2][2]);
			R >>= 12;
			G >>= 12;
			B >>= 12;
			pRGBline[0] = CLIP(R, m_nMin, m_nMax);
			pRGBline[1] = CLIP(G, m_nMin, m_nMax);
			pRGBline[2] = CLIP(B, m_nMin, m_nMax);
			pRGBline += 3;
		}
	}
}
