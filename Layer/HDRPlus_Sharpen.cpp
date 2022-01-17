#include "HDRPlus_Sharpen.h"
bool CHDRPlus_Sharpen::RGBToYUV(MultiUshortImage *pRGBImage, MultiUshortImage *YImage, MultiUshortImage *UImage, MultiUshortImage *VImage)
{
	int nWidth = pRGBImage->GetImageWidth();
	int nHeight = pRGBImage->GetImageHeight();
	if (YImage->GetImageWidth() != nWidth || YImage->GetImageHeight() != nHeight)
	{
		if (!YImage->SetImageSize(nWidth, nHeight, 1))return false;
	}
	if (UImage->GetImageWidth() != nWidth || UImage->GetImageHeight() != nHeight)
	{
		if (!UImage->SetImageSize(nWidth, nHeight, 1))return false;
	}
	if (VImage->GetImageWidth() != nWidth || VImage->GetImageHeight() != nHeight)
	{
		if (!VImage->SetImageSize(nWidth, nHeight, 1))return false;
	}
#pragma omp parallel for 
	for (int y = 0; y < nHeight; y++)
	{
		unsigned short *pRGBline = pRGBImage->GetImageLine(y);
		unsigned short *pYline = YImage->GetImageLine(y);
		unsigned short *pUline = UImage->GetImageLine(y);
		unsigned short *pVline = VImage->GetImageLine(y);
		for (int x = 0; x < nWidth; x++)
		{
			long long int  yuv[3];
			yuv[0] = (pRGBline[2] * 29 + pRGBline[1] * 150 + pRGBline[0] * 77 + 128) >> 8;
			yuv[1] = (pRGBline[2] * 128 - pRGBline[1] * 85 - pRGBline[0] * 43) / 256;	yuv[1] += 32768;
			yuv[2] = (pRGBline[0] * 128 - pRGBline[1] * 107 - pRGBline[2] * 21) / 256; yuv[2] += 32768;
			/*	pYline[0] = 0.298900f * pRGBline[0] + 0.587000f *pRGBline[1] + 0.114000f * pRGBline[2];
				pUline[0] = -0.168935f * pRGBline[0] - 0.331655f * pRGBline[1] + 0.500590f * pRGBline[2];
				pVline[0] = 0.499813f * pRGBline[0] - 0.418531f * pRGBline[1] - 0.081282f * pRGBline[2];*/
			pYline[0] = CLIP(yuv[0], 0, 65535);
			pUline[0] = CLIP(yuv[1], 0, 65535);
			pVline[0] = CLIP(yuv[2], 0, 65535);
			pYline++;
			pUline++;
			pVline++;
			pRGBline += 3;
		}
	}
	return true;
}
void CHDRPlus_Sharpen::Forward(MultiUshortImage *pRGBImage)
{
	float strength = (float)m_nSharpenStrength /(float)16;
	int nWidth = pRGBImage->GetImageWidth();
	int nHeight = pRGBImage->GetImageHeight();
	int sizeImage = nWidth * nHeight;
	MultiUshortImage YImage, UImage, VImage, SmallImage, LargeImage;
	RGBToYUV(pRGBImage, &YImage, &UImage, &VImage);
	SmallImage.SetImageSize(nWidth, nHeight, 1);
	LargeImage.SetImageSize(nWidth, nHeight, 1);
	StoSSmooth7Image(YImage.GetImageData(), SmallImage.GetImageData(), nWidth, nHeight);
	StoSSmooth7Image(SmallImage.GetImageData(), LargeImage.GetImageData(), nWidth, nHeight);
#pragma omp parallel for 
	for (int y = 0; y < nHeight; y++)
	{
		unsigned short *pRGBline = pRGBImage->GetImageLine(y);
		unsigned short *pYline = YImage.GetImageLine(y);
		unsigned short *pUline = UImage.GetImageLine(y);
		unsigned short *pVline = VImage.GetImageLine(y);
		unsigned short *pSmallline = SmallImage.GetImageLine(y);
		unsigned short *pLargeline = LargeImage.GetImageLine(y);
		for (int x = 0; x < nWidth; x++)
		{
			long long int YUV[3], RGB[3];
			YUV[0] = pYline[0] + strength * (pSmallline[0] - pLargeline[0]);
			//int R = pYline[0] + 1.403f * pVline[0];
			//int G = pYline[0] - .344f * pUline[0] - 0.714f * pVline[0];
			//int B = pYline[0] + 1.770f * pUline[0];
			//pRGBline[0] = CLIP(R, 0, 65535);
			//pRGBline[1] = CLIP(G, 0, 65535);
			//pRGBline[2] = CLIP(B, 0, 65535);
			YUV[1] = pUline[0];
			YUV[2] = pVline[0];
			YUV[1] -= 32768;
			YUV[2] -= 32768;
			RGB[2] = YUV[0] * 2048 + YUV[1] * (4096 - 467);
			RGB[0] = YUV[0] * 2048 + YUV[2] * (4096 - 1225);
			RGB[1] = YUV[0] * 2048 - YUV[1] * 705 - YUV[2] * 1463;
			for (int i = 0; i < 3; i++)
			{
				if (RGB[i] < 0)RGB[i] = 0;
				RGB[i] >>= 11;
				if (RGB[i] > 65535)RGB[i] = 65535;
				pRGBline[i] = (unsigned short)RGB[i];
			}

			pYline++;
			pUline++;
			pVline++;
			pSmallline++;
			pLargeline++;
			pRGBline += 3;
		}
	}
}