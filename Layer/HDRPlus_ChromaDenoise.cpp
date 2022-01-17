#include "HDRPlus_ChromaDenoise.h"
bool CHDRPlus_ChromaDenoise::RGBToYUV(MultiUshortImage *pRGBImage, MultiUshortImage *YImage, MultiUshortImage *UImage, MultiUshortImage *VImage)
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
bool CHDRPlus_ChromaDenoise::YUVToRGB(MultiUshortImage *YImage, MultiUshortImage *UImage, MultiUshortImage *VImage, MultiUshortImage *pRGBImage)
{
	int nWidth = YImage->GetImageWidth();
	int nHeight = YImage->GetImageHeight();
	int nDim = YImage->GetImageDim();
	if (pRGBImage->GetImageWidth() != nWidth || pRGBImage->GetImageHeight() != nHeight)
	{
		if (!pRGBImage->CreateImage(nWidth, nHeight, 3, 16))return false;
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
			/*int R = pYline[0] + 1.403f * pVline[0];
			int G = pYline[0] - .344f * pUline[0] - .714f * pVline[0];
			int B = pYline[0] + 1.770f * pUline[0];*/
			long long int YUV[3], RGB[3];
			YUV[0] = pYline[0];
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
			pRGBline += 3;
		}
	}
	return true;
}
bool CHDRPlus_ChromaDenoise::DesaturateNoise(MultiUshortImage *pUVImage)
{
	MultiUshortImage pBlurUVImage, pBlurUVImage1;
	int nWidth = pUVImage->GetImageWidth();
	int nHeight = pUVImage->GetImageHeight();
	int nDim = pUVImage->GetImageDim();
	if (pBlurUVImage.GetImageWidth() != nWidth || pBlurUVImage.GetImageHeight() != nHeight)
	{
		if (!pBlurUVImage.SetImageSize(nWidth, nHeight, nDim))return false;
	}
	if (pBlurUVImage1.GetImageWidth() != nWidth || pBlurUVImage1.GetImageHeight() != nHeight)
	{
		if (!pBlurUVImage1.SetImageSize(nWidth, nHeight, nDim))return false;
	}
	SToSSmoothx15(pUVImage->GetImageData(), pBlurUVImage.GetImageData(), nWidth, nHeight);
	SToSSmoothx15(pBlurUVImage.GetImageData(), pBlurUVImage1.GetImageData(), nWidth, nHeight);
	float ratiothre = (float)m_nRatioThre / (float)16;//Factor;// 1.4f;//default=1.4
	float threshold = m_nThreshold;// 25000.f;// 25000.f;//default=25000.f
#pragma omp parallel for
	for (int y = 0; y < nHeight; y++)
	{
		unsigned short *pOutYUVline = pUVImage->GetImageLine(y);
		unsigned short *pBlurYUVline = pBlurUVImage1.GetImageLine(y);
		for (int x = 0; x < nWidth; x++)
		{
			float blur = pBlurYUVline[0] - 32768;
			float Input = pOutYUVline[0] - 32768;
			float Ratio = blur / Input;
			if (abs(Ratio) < ratiothre&&abs(Input) < threshold&&blur < threshold)
			{
				pOutYUVline[0] = 0.7f * pBlurYUVline[0] + 0.3f * pOutYUVline[0];
			}
			pOutYUVline++;
			pBlurYUVline++;
		}
	};
	return true;
}
void CHDRPlus_ChromaDenoise::IncreaseSaturation(MultiUshortImage *pUVImage, float len)
{
	int nWidth = pUVImage->GetImageWidth();
	int nHeight = pUVImage->GetImageHeight();
	int nDim = pUVImage->GetImageDim();
#pragma omp parallel for
	for (int y = 0; y < nHeight; y++)
	{
		unsigned short *pYUVline = pUVImage->GetImageLine(y);
		for (int x = 0; x < nWidth; x++)
		{
			int tmp = pYUVline[0] - 32768;
			tmp = len * tmp;
			pYUVline[0] = CLIP(tmp, -32768, 32767);
			pYUVline[0] += 32768;
			pYUVline++;
		}
	}
}
bool CHDRPlus_ChromaDenoise::Forward(MultiUshortImage *pRGBImage,TGlobalControl *pControl)
{
	int nGain = pControl->nCameraGain;//nGain x128
	float Amount;
	if (m_bAutoAmount)
	{
		if (m_nGainOption == 0)
		{
			nGain = pControl->nCameraGain;//iso gain
		}
		else if (m_nGainOption == 1)
		{
			nGain = (pControl->nCameraGain *pControl->nDigiGain + 64) >> 7;//iso+digigain
		}
		else
		{
			nGain = pControl->nEQGain;//lenshadingÖ®ºóµÄ
		}
		if (nGain < m_nGainList[0])
		{
			Amount = m_nAmountGainList[0];
		}
		else if (nGain < m_nGainList[1])
		{
			int dG = (nGain - m_nGainList[0]);
			Amount = m_nAmountGainList[0];
			Amount += (m_nAmountGainList[1] - m_nAmountGainList[0])*dG / (m_nGainList[1] - m_nGainList[0]);
		}
		else if (nGain < m_nGainList[2])
		{
			int dG = (nGain - m_nGainList[1]);
			Amount = m_nAmountGainList[1];
			Amount += (m_nAmountGainList[2] - m_nAmountGainList[1])*dG / (m_nGainList[2] - m_nGainList[1]);
		}
		else
		{
			Amount = m_nAmountGainList[2];
		}
	}
	else
	{
		Amount = m_nManualAmount;
	}
	Amount = Amount / 16.0;
	m_nYnoiseBilateralThre =  m_nYnoiseBilateralThre * Amount;
	printf("%d %f\n", nGain, Amount);
	MultiUshortImage YImage, UImage, VImage;
	RGBToYUV(pRGBImage, &YImage, &UImage, &VImage);
	int pass = 0;
	if (m_nDenoiseTimes > 0)
	{
		YImage.Bilateral5x5SingleImage(m_nYnoiseBilateralThre);
	}
	//pass++;
	while (pass < m_nDenoiseTimes)
	{
		/*if (pass==0)
		{
			UImage.GetEachBlockAverageValue(5);
			VImage.GetEachBlockAverageValue(5);
		}
		else*/
		{
			DesaturateNoise(&UImage);
			DesaturateNoise(&VImage);
		}
		pass++;
	}
	if (m_nDenoiseTimes > 2)
	{
		float len = (float)m_nAddSaturation/(float)16;
		IncreaseSaturation(&UImage, len);
		IncreaseSaturation(&VImage, len);
	}
	return YUVToRGB(&YImage, &UImage, &VImage, pRGBImage);
}
