#include "HDRPlus_Contrast.h"
void CHDRPlus_Contrast::Forward(MultiUshortImage *pRGBImage)
{
	float ContrastStrength = (float)m_ContrastStrength/(float)16;
	int nWidth = pRGBImage->GetImageWidth();
	int nHeight = pRGBImage->GetImageHeight();
	float scale = 0.8f + 0.3f / min(1.f, ContrastStrength);
	float inner_constant = 3.141592f / (2.f * scale);
	float sin_constant = sin(inner_constant);
	float slope = 65535.f / (2.f * sin_constant);
	float constant = slope * sin_constant;
	float factor = 3.141592f / (scale * 65535.f);
	float white_scale = 65535.f / (65535.f - m_nBlacklevel);
	unsigned short Table[65536];
	for (int k = 0; k < 65536; k++)
	{
		long int  tmp = ((slope * sin(factor * k - inner_constant) + constant) - m_nBlacklevel)* white_scale;
		Table[k] = CLIP(tmp, 0, 65535);
	}
#pragma omp parallel for 
	for (int y = 0; y < nHeight; y++)
	{
		unsigned short *pRawline = pRGBImage->GetImageLine(y);
		for (int x = 0; x < nWidth; x++)
		{
			pRawline[0] = Table[pRawline[0]];
			pRawline[1] = Table[pRawline[1]];
			pRawline[2] = Table[pRawline[2]];
			pRawline += 3;
		}
	}
}