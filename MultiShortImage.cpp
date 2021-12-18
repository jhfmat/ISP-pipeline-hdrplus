#include "MultiShortImage.h"
bool MultiShortImage::CreateImage(int nWidth, int nHeight, int nCh, int nBit)
{
	m_nBit = (nBit > 15) ? (15) : (nBit);
	m_nMAXS = (1 << m_nBit) - 1;
	m_nBLC = 0;
	return SetImageSize(nWidth, nHeight, nCh);
}
bool MultiShortImage::CreateImage(int width, int height, int channel)
{
	return SetImageSize(width, height, channel);
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
	pOutBGRImage->m_nMAXS = m_nMAXS;
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
bool MultiShortImage::SaveSingleChannelToBitmapFile(char *pFileName, int nChannel, int nInScale, int nOutScale, int nOffset)
{
	int x, y, Y, tY, E;
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int nCh = GetImageDim();
	SingleUcharImage YImage;
	if (!YImage.CreateImage(nWidth, nHeight))return false;
	if (nChannel<0)nChannel = 0;
	if (nChannel >= nCh)nChannel = nCh - 1;
	E = 0;
	for (y = 0; y<nHeight; y++)
	{
		short *pInLine = GetImageLine(y);
		unsigned char *pOutLine = YImage.GetImageLine(y);
		for (x = 0; x<nWidth; x++)
		{
			Y = pInLine[nChannel];
			pInLine += nCh;
			Y = Y * nOutScale + E;
			tY = Y / nInScale;
			E = Y - tY * nInScale;
			Y = tY + nOffset;
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