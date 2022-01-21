#include "MultiShortImage.h"
bool MultiShortImage::CreateImage(int nWidth, int nHeight, int nCh, int nBit)
{
	m_nBit = (nBit > 15) ? (15) : (nBit);
	m_nMAXS = (1 << m_nBit) - 1;
	m_nBLC = 0;
	return SetImageSize(nWidth, nHeight, nCh);
}
bool MultiShortImage::CreateImageFillValue(int nWidth, int nHeight,int nDim, int nValue)
{
	if (!SetImageSize(nWidth, nHeight, nDim))return false;
	FillValue(nValue);
	return true;
}
bool MultiShortImage::CreateImage(int width, int height, int channel)
{
	return SetImageSize(width, height, channel);
}
bool MultiShortImage::CreateImageWithData(int nWidth, int nHeight,int nDim,  short *pInputData)
{
	if (!SetImageSize(nWidth, nHeight, nDim))return false;
	memcpy(GetImageData(), pInputData, nWidth*nHeight*nDim*sizeof(short));
	return true;
}
void MultiShortImage::CopyParameter(MultiShortImage *pInputImage)
{
	m_nMAXS = pInputImage->m_nMAXS;
	m_nBLC = pInputImage->m_nBLC;
	m_nBit = pInputImage->m_nBit;
}
bool MultiShortImage::Clone(MultiShortImage *pInputImage)
{
	if (!SetImageSize(pInputImage->GetImageWidth(), pInputImage->GetImageHeight(), pInputImage->GetImageDim()))return false;
	CopyParameter(pInputImage);
	memcpy(m_pImgData, pInputImage->GetImageData(), sizeof(short)*m_nWidth*m_nHeight*m_nChannel);
	return true;
}
bool MultiShortImage::GetRectHistogram(int nHist[], int maxvalue, int nLeft, int nTop, int nRight, int nBottom)
{
	int nDim = GetImageDim();
	if (nDim == 1)
	{
		if (m_pImgData == NULL)return false;
		for (int g = 0; g < maxvalue; g++)
		{
			nHist[g] = 0;
		}
		nLeft = MAX2(0, nLeft);
		nTop = MAX2(0, nTop);
		nRight = MIN2(nRight, m_nWidth);
		nBottom = MIN2(nBottom, m_nHeight);
		if (nBottom <= nTop || nRight <= nLeft)return false;
		for (int y = nTop; y < nBottom; y++)
		{
			short *pY = GetImageLine(y) + nLeft;
			for (int x = nLeft; x < nRight; x++)
			{
				short g = *(pY++);
				nHist[g]++;
			}
		}
	}
	return true;
}
bool MultiShortImage::GetEachBlockAverageValue(MultiShortImage *pOutImage, int nRadius)
{
	int Width = GetImageWidth();
	int Height = GetImageHeight();
	int Stride = Width*1;
	if (!pOutImage->CreateImage(Width, Height,1))return false;
	CImageData_UINT32 Integral;
	Integral.SetImageSize((Width + 1),(Height + 1),1);
	GetImageIntegralSData(GetImageData(), Integral.GetImageData(), Width, Height, Stride);
	int nProcs = omp_get_num_procs();
#pragma omp parallel for  num_threads(nProcs) schedule(dynamic,16)  
	for (int Y = 0; Y < Height; Y++)
	{
		int Y1 = max(Y - nRadius, 0);
		int Y2 = min(Y + nRadius + 1, Height - 1);
		unsigned int *LineP1 = Integral.GetImageLine(Y1);
		unsigned int *LineP2 = Integral.GetImageLine(Y2);
		short *LinePD = pOutImage->GetImageLine(Y);
		for (int X = 0; X < Width; X++)
		{
			int X1 = max(X - nRadius, 0);
			int X2 = min(X + nRadius + 1, Width);
			unsigned int Sum = LineP2[X2] - LineP1[X2] - LineP2[X1] + LineP1[X1];
			unsigned int PixelCount = (X2 - X1) * (Y2 - Y1);
			LinePD[X] = (Sum + (PixelCount >> 1)) / PixelCount;
		}
	}
	return true;
}
bool MultiShortImage::BGRHToBGR(MultiUcharImage *pOutBGRImage, bool bAddH)
{
	int i, x, y, g, G, E[3];
	if (!pOutBGRImage->CreateImage(m_nWidth, m_nHeight))return false;
	E[0] = E[1] = E[2] = m_nMAXS / 2;
	for (y = 0; y < m_nHeight; y++)
	{
		short *pInLine = GetImageLine(y);
		unsigned char *pOutLine = pOutBGRImage->GetImageLine(y);
		for (x = 0; x < m_nWidth; x++)
		{
			for (i = 0; i < 3; i++)
			{
				g = pInLine[i];
				if (m_nChannel > 3 && bAddH)
				{
					g += pInLine[3];
				}
				if (g < 0)g = 0;
				if (m_nMAXS != 255)
				{
					G = g * 255 + E[i];
					g = G / m_nMAXS;
					E[i] = G - g * m_nMAXS;
				}
				if (g > 255)g = 255;
				*(pOutLine++) = (unsigned char)g;
			}
			pInLine += m_nChannel;
		}
	}
	return true;
}
bool MultiShortImage::BGRHToBGR(MultiShortImage *pOutBGRImage)
{
	int i, x, y, g;
	if (!pOutBGRImage->CreateImage(m_nWidth, m_nHeight, 3))return false;
	pOutBGRImage->m_nMAXS =  m_nMAXS;
	for (y = 0; y < m_nHeight; y++)
	{
		short *pInLine = GetImageLine(y);
		short *pOutLine = pOutBGRImage->GetImageLine(y);
		for (x = 0; x < m_nWidth; x++)
		{
			for (i = 0; i < 3; i++)
			{
				g = pInLine[i];
				if (m_nChannel > 3)
				{
					g += pInLine[3];
				}
				if (g < 0)g = 0;
				if (g > m_nMAXS)g = m_nMAXS;
				*(pOutLine++) = (short)g;
			}
			pInLine += m_nChannel;
		}
	}
	return true;
}
bool MultiShortImage::BGRHToRGB(MultiUshortImage *pOutRGBImage)
{
	if (!pOutRGBImage->CreateImage(m_nWidth, m_nHeight, 3,16))return false;
	pOutRGBImage->m_nRawMAXS = 32767;
	for (int y = 0; y < m_nHeight; y++)
	{
		short *pInLine = GetImageLine(y);
		unsigned short *pOutLine = pOutRGBImage->GetImageLine(y);
		for (int x = 0; x < m_nWidth; x++)
		{
			unsigned short RGB[3];
			for (int i = 0; i < 3; i++)
			{
				int g = pInLine[i];
				if (m_nChannel > 3)
				{
					g += pInLine[3];
				}
				if (g < 0)g = 0;
				if (g > 32767)g = 32767;
				RGB[i] = (unsigned short)g;
			}
			*(pOutLine++) = RGB[0];
			*(pOutLine++) = RGB[1];
			*(pOutLine++) = RGB[2];
			pInLine += m_nChannel;
		}
	}
	return true;
}
bool MultiShortImage::ApplyWeight(MultiUshortImage *pWeightImage, int ScaleBit)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int nDim = GetImageDim();
	#pragma omp parallel for 
	for (int y = 0; y < nHeight; y++)
	{
		short *pInLine = GetImageLine(y);
		unsigned short *pInWeightLine = pWeightImage->GetImageLine(y);
		if (nDim==1)
		{
			for (int x = 0; x < nWidth; x++)
			{
				long long int tmpOut = pInLine[0] * pInWeightLine[0];
				//tmpOut >>= ScaleBit;
				tmpOut = tmpOut/4096;
				pInLine[0] = CLIP(tmpOut, -32768, 32767);
				pInLine++;
				pInWeightLine++;
			}
		}
		else
		{
			for (int x = 0; x < nWidth; x++)
			{
				for (int c = 0; c < nDim; c++)
				{
					int tmpOut = pInLine[c] * pInWeightLine[c];
					tmpOut >>= ScaleBit;
					pInLine[c] = CLIP(tmpOut, -32768, 32767);
				}
				pInLine += nDim;
				pInWeightLine += nDim;
			}
		}
	}
	return true;
}
bool MultiShortImage::AddImage(MultiShortImage *pRefImage)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int nDim = GetImageDim();
	#pragma omp parallel for 
	for (int y = 0; y < nHeight; y++)
	{
		short *pInLine = GetImageLine(y);
		short *pInRefLine = pRefImage->GetImageLine(y);
		if (nDim==1)
		{
			for (int x = 0; x < nWidth; x++)
			{
				int tmpOut = pInLine[0] + pInRefLine[0];
				pInLine[0] = CLIP(tmpOut, -32768, 32767);
				pInLine++;
				pInRefLine++;
			}
		}
		else
		{
			for (int x = 0; x < nWidth; x++)
			{
				for (int c = 0; c < nDim; c++)
				{
					int tmpOut = pInLine[0] + pInRefLine[0];
					pInLine[c] = CLIP(tmpOut, -32768, 32767);
				}
				pInLine += nDim;
				pInRefLine += nDim;
			}
		}
	}
	return true;
}
bool MultiShortImage::SaveSingleChannelToBitmapFile(char *pFileName, int nChannel, int nInScale, int nOutScale, int nOffset)
{
	int x, y, Y, tY;
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int nCh = GetImageDim();
	SingleUcharImage YImage;
	if (!YImage.CreateImage(nWidth, nHeight))return false;
	if (nChannel<0)nChannel = 0;
	if (nChannel >= nCh)nChannel = nCh - 1;
	for (y = 0; y<nHeight; y++)
	{
		short *pInLine = GetImageLine(y);
		unsigned char *pOutLine = YImage.GetImageLine(y);
		for (x = 0; x<nWidth; x++)
		{
			Y = pInLine[nChannel];
			pInLine += nCh;
			Y = Y * nOutScale ;
			tY = Y / nInScale;
			Y = abs(tY + nOffset);		
			if (Y < 0)Y = 0;	if (Y > 255)Y = 255;
			*(pOutLine++) = (unsigned char)Y;
		}
	}
	return YImage.SaveGrayToBitmapFile(pFileName);
}
bool MultiShortImage::SaveBGRHToBitmapFile(char *pFileName, int nB, int nG, int nR, int nH)
{
	int i, x, y, BGRH[4], E[3];
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int nCh = GetImageDim();
	int nMAXS = m_nMAXS;
	MultiUcharImage BGRImage;
	if (!BGRImage.CreateImage(nWidth, nHeight))return false;
	if (nB<0)nB = 0;	if (nB >= nCh)nB = nCh - 1;
	if (nG<0)nG = 0;	if (nG >= nCh)nG = nCh - 1;
	if (nR<0)nR = 0;	if (nR >= nCh)nR = nCh - 1;
	BGRH[3] = E[0] = E[1] = E[2] = 0;
	for (y = 0; y<nHeight; y++)
	{
		short *pInLine = GetImageLine(y);
		unsigned char *pOutLine = BGRImage.GetImageLine(y);
		for (x = 0; x<nWidth; x++)
		{
			BGRH[0] = pInLine[nB];
			BGRH[1] = pInLine[nG];
			BGRH[2] = pInLine[nR];
			if (nH >= 0 && nH<nCh)BGRH[3] = pInLine[nH];
			pInLine += nCh;
			for (i = 0; i<3; i++)
			{
				BGRH[i] += BGRH[3];
				if (BGRH[i]<0)BGRH[i] = 0;	if (BGRH[i]>nMAXS)BGRH[i] = nMAXS;
				BGRH[i] = BGRH[i] * 255 + E[i];
				int t = BGRH[i] / nMAXS;
				if (t>255)t = 255;
				*(pOutLine++) = (unsigned char)t;
				E[i] = rand() % nMAXS;
			}
		}
	}
	return BGRImage.SaveBGRToBitmapFile(pFileName);
}