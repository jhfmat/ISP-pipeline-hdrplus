#include "HDRPlus_BlackWhiteLevel.h"
void CHDRPlus_BlackWhiteLevel::Forward(MultiUshortImage *pInRawImage, TGlobalControl *pControl)
{
	int nWidth = pInRawImage->GetImageWidth();
	int nHeight = pInRawImage->GetImageHeight();
	pControl->nWP = pInRawImage->GetMaxVal();
	float white_factor = 65535.f / (float)(pControl->nWP - pControl->nBLC);
	unsigned short table[65536];
	for (int k = 0; k < 65536; k++)
	{
		int tmp = k * white_factor;
		table[k] = CLIP(tmp, 0, 65535);
	}
#pragma omp parallel for
	for (int y = 0; y < nHeight; y++)
	{
		unsigned short *pRawLine = pInRawImage->GetImageLine(y);
		int x = 0;
		for (; x < nWidth; x++)
		{
			int tmp = pRawLine[0] - pControl->nBLC;
			if (tmp < 0)
			{
				tmp = 0;
			}
			pRawLine[0] = table[tmp];
			pRawLine++;
		}
	}
	pControl->nWP = 65535;
	pControl->nBLC = 0;
}
