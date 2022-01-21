#include "MultiIntImage.h"
bool MultIntImage::CreateImage(int nWidth, int nHeight, int nChannel)
{
	return SetImageSize(nWidth, nHeight, nChannel);
}
bool MultIntImage::CreateImageWithData(int nWidth, int nHeight,int nChannel, int *pInputData)
{
	if (!SetImageSize(nWidth, nHeight, nChannel))return false;
	memcpy(GetImageData(), pInputData, nWidth*nHeight*nChannel*sizeof(int));
	return true;
}
bool MultIntImage::CreateImageFillValue(int nWidth, int nHeight, int nValue)
{
	if (!SetImageSize(nWidth, nHeight, 1))return false;
	FillValue(nValue);
	return true;
}
bool MultIntImage::Clone(MultIntImage *pInputImage)
{
	int nSize= pInputImage->GetImageSize();
	if(!CreateImage(pInputImage->GetImageWidth(), pInputImage->GetImageHeight(), pInputImage->GetImageDim()))
	{
		return false;
	}
	memcpy(m_pImgData, pInputImage->GetImageData(), sizeof(int)*nSize);
	m_nMAXS= pInputImage->m_nMAXS;
	m_nBLC= pInputImage->m_nBLC;
	return true;
}
bool MultIntImage::BGRHToBGR(MultiUcharImage *pOutBGRImage)
{
	int i, x, y, BGRH[4], E[3];
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int nCh = GetImageDim();
	int nMAXS = m_nMAXS;
	if (nCh < 4)return false;
	if (!pOutBGRImage->CreateImage(nWidth, nHeight))return false;
	BGRH[3] = E[0] = E[1] = E[2] = 0;
	for (y = 0; y<nHeight; y++)
	{
		int *pInLine = GetImageLine(y);
		unsigned char *pOutLine = pOutBGRImage->GetImageLine(y);
		for (x = 0; x<nWidth; x++)
		{
			BGRH[0] = pInLine[0];
			BGRH[1] = pInLine[1];
			BGRH[2] = pInLine[2];
			BGRH[3] = pInLine[3];
			pInLine += nCh;
			for (i = 0; i<3; i++)
			{
				BGRH[i] += BGRH[3];
				if (BGRH[i]<0)BGRH[i] = 0;	if (BGRH[i]>nMAXS)BGRH[i] = nMAXS;
				BGRH[i] = BGRH[i] * 255 + E[i];
				int t = BGRH[i] / nMAXS;
				if (t < 0)t = 0;	if (t > 255)t = 255;
				*(pOutLine++) = (unsigned char)t;
				E[i] = BGRH[i] - t * nMAXS;
			}
		}
	}
	return true;
}
bool MultIntImage::BGRHToBGR(MultiUshortImage *pOutBGRImage)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int nCh = GetImageDim();
	if (nCh < 4)return false;
	if (!pOutBGRImage->CreateImage(nWidth, nHeight,3,16))return false;
	#pragma omp parallel for
	for (int y = 0; y < nHeight; y++)
	{
		int *pInLine = GetImageLine(y);
		unsigned short *pOutLine = pOutBGRImage->GetImageLine(y);
		for (int x = 0; x < nWidth; x++)
		{
			int BGRH[4];
			BGRH[0] = pInLine[0];
			BGRH[1] = pInLine[1];
			BGRH[2] = pInLine[2];
			BGRH[3] = pInLine[3];
			pInLine += nCh;
			for (int i = 0; i < 3; i++)
			{
				BGRH[i] += BGRH[3];
				if (BGRH[i] < 0)BGRH[i] = 0;	
				if (BGRH[i] > m_nMAXS)BGRH[i] = m_nMAXS;
				*(pOutLine++) = (unsigned short)BGRH[i];
			}
		}
	}
	return true;
}
bool MultIntImage::SaveBGRHToBitmapFile(char *pFileName, int nB, int nG, int nR, int nH)
{
	int i, x, y, BGRH[4], E[3];
	int nWidth=GetImageWidth();
	int nHeight=GetImageHeight();
	int nCh=GetImageDim();
	int nMAXS=m_nMAXS;
	MultiUcharImage BGRImage;
	if(!BGRImage.CreateImage(nWidth, nHeight))return false; 
	if(nB<0)nB=0;	if(nB>=nCh)nB=nCh-1;
	if(nG<0)nG=0;	if(nG>=nCh)nG=nCh-1;
	if(nR<0)nR=0;	if(nR>=nCh)nR=nCh-1;
	BGRH[3]=E[0]=E[1]=E[2]=0;
	for(y=0; y<nHeight; y++)
	{
		int *pInLine=GetImageLine(y);
		unsigned char *pOutLine=BGRImage.GetImageLine(y);
		for(x=0; x<nWidth; x++)
		{
			BGRH[0]=pInLine[nB];
			BGRH[1]=pInLine[nG];
			BGRH[2]=pInLine[nR];
			if(nH>=0&&nH<nCh)BGRH[3]=pInLine[nH];
			pInLine+=nCh;
			for(i=0; i<3; i++)
			{
				BGRH[i]+=BGRH[3];
				if(BGRH[i]<0)BGRH[i]=0;	if(BGRH[i]>nMAXS)BGRH[i]=nMAXS;
				BGRH[i]=BGRH[i]*255+E[i];
				int t=BGRH[i]/nMAXS;
				if(t>255)t=255;
				*(pOutLine++)=(unsigned char) t;
				E[i]=rand()%nMAXS;
			}
		}
	}
	return BGRImage.SaveBGRToBitmapFile(pFileName);
}
bool MultIntImage::SaveSingleChannelToBitmapFile(char *pFileName, int nChannel, int nInScale, int nOutScale, int nOffset)
{
	int x, y;
	long long Y, tY, E;
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int Ch = GetImageDim();
	SingleUcharImage YImage;
	if (!YImage.CreateImage(nWidth, nHeight))return false;
	if (nChannel<0)nChannel = 0;
	if (nChannel >= Ch)nChannel = Ch - 1;
	E = 0;
	for (y = 0; y<nHeight; y++)
	{
		int *pInLine = GetImageLine(y);
		unsigned char *pOutLine = YImage.GetImageLine(y);
		for (x = 0; x<nWidth; x++)
		{
			Y = pInLine[nChannel];
			pInLine += Ch;
			Y = Y * nOutScale + E;
			tY = Y / nInScale;
			E = Y - tY * nInScale;
			Y = tY + nOffset;
			//Y = (Y*nOutScale) / nInScale + nOffset;
			if (Y < 0)Y = 0;	if (Y > 255)Y = 255;
			*(pOutLine++) = (unsigned char)Y;
		}
	}
	return YImage.SaveGrayToBitmapFile(pFileName);
}