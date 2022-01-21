#include "HDRPlus_WhiteBalance.h"
void CHDRPlus_WhiteBalance::Forward(MultiUshortImage *pInRawImage, TGlobalControl *pControl)
{
	int nWidth = pInRawImage->GetImageWidth();
	int nHeight = pInRawImage->GetImageHeight();
	const float Rgain = (float)pControl->nAWBGain[0] / (float)256.0;// coeffs[0] / coeffs[1];
	const float Ggain0 = (float)pControl->nAWBGain[1] / (float)256.0;// 1.f;
	const float Ggain1 = (float)pControl->nAWBGain[2] / (float)256.0;// 1.f;
	const float Bgain = (float)pControl->nAWBGain[3] / (float)256.0;// coeffs[2] / coeffs[1];
	printf("Rgain Bgain %f %f\n", Rgain, Bgain);
	m_nMin = pControl->nBLC;
	m_nMax = pControl->nWP;
	unsigned short *tableRgain=new unsigned short[m_nMax+1];
	unsigned short *tableBGain= new unsigned short[m_nMax + 1];
	for (int k = 0; k < 65536; k++)
	{
		int tmp = k * Rgain;
		tableRgain[k] = CLIP(tmp, m_nMin, m_nMax);
		tmp = k * Bgain ;
		tableBGain[k] = CLIP(tmp, m_nMin, m_nMax);
	}
#pragma omp parallel for
	for (int y = 0; y < nHeight; y += 2)
	{
		unsigned short *pInRawLine0;
		unsigned short *pInRawLine1;
		switch (pControl->nCFAPattern)
		{
		case BGGR:
			pInRawLine0 = pInRawImage->GetImageLine(y+1) + 1;
			pInRawLine1 = pInRawImage->GetImageLine(y);
			break;
		case GBRG:
			pInRawLine0 = pInRawImage->GetImageLine(y+1);
			pInRawLine1 = pInRawImage->GetImageLine(y) + 1;
			break;
		case GRBG:
			pInRawLine0 = pInRawImage->GetImageLine(y) + 1;
			pInRawLine1 = pInRawImage->GetImageLine(y + 1);
			break;
		case RGGB:
			pInRawLine0 = pInRawImage->GetImageLine(y);
			pInRawLine1 = pInRawImage->GetImageLine(y+1) + 1;
			break;
		default:
			printf("Please enter correctly RawTYPE\n");
			break;
		}
		for (int x = 0; x < nWidth; x += 2)
		{
			pInRawLine0[0] = tableRgain[pInRawLine0[0]];
			pInRawLine1[0] = tableBGain[pInRawLine1[0]];
			pInRawLine0 += 2;
			pInRawLine1 += 2;
		}
	}
	delete[]tableRgain;
	delete[]tableBGain;
}