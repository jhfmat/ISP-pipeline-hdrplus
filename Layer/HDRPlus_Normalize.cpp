#include "HDRPlus_Normalize.h"
bool CHDRPlus_Normalize::Forward(MultiUshortImage *pInRGBImage, MultiUcharImage *pOutRGBImage)
{
	int nWidth = pInRGBImage->GetImageWidth();
	int nHeight = pInRGBImage->GetImageHeight();
	int nDim = pInRGBImage->GetImageDim();
	if (pOutRGBImage->GetImageWidth() != nWidth || pOutRGBImage->GetImageHeight() != nHeight)
	{
		if (!pOutRGBImage->CreateImage(nWidth, nHeight))return false;
	}
	unsigned char nShift = 16 - m_nOutBit;
	unsigned char OutRGBMAXS = (1 << m_nOutBit) - 1;
	int Stride = nWidth * nDim;
#pragma omp parallel for
	for (int y = 0; y < nHeight; y++)
	{
		unsigned short *pInRGBline = pInRGBImage->GetImageLine(y);
		unsigned char *pOutRGBline = pOutRGBImage->GetImageLine(y);
		for (int x = 0; x < Stride; x++)
		{
			unsigned short Y = pInRGBline[0];
			Y >>= nShift;
			if (Y > OutRGBMAXS)Y = OutRGBMAXS;
			pOutRGBline[0] = (unsigned char)Y;
			pInRGBline++;
			pOutRGBline++;
		}
	}
	return true;
}

