#include "MultiUcharImage.h"
#include "YUV420Image.h"
bool MultiUcharImage::CreateImage(int nWidth, int nHeight)
{
	return SetImageSize(nWidth, nHeight, 3);
}
bool MultiUcharImage::CreateImageWithData(int nWidth, int nHeight, unsigned char *pInputData)
{
	if(!SetImageSize(nWidth, nHeight, 3))return false;
	memcpy(GetImageData(), pInputData, nWidth*nHeight * 3);
	return true;
}
bool MultiUcharImage::CreateImageFillValue(int nWidth, int nHeight, int nValue)
{
	if (!SetImageSize(nWidth, nHeight, 3))return false;
	FillValue(nValue);
	return true;
}
bool MultiUcharImage::Clone(MultiUcharImage *pInputImage)
{
	int nWidth= pInputImage->GetImageWidth();
	int nHeight= pInputImage->GetImageHeight();
	if(!CreateImage(nWidth, nHeight))return false;
	memcpy(m_pImgData, pInputImage->GetImageData(), nWidth*nHeight * 3);
	return true;
}
float MultiUcharImage::GetRGBtoGrayMeanBrightness()
{
	int nHeight = GetImageHeight();
	int nWidth = GetImageWidth();
	uint64_t GetRGBBrightness = 0;
	for (int i = 0; i < nHeight; i++)
	{
		int rgb[3];
		unsigned char *pRGBline = GetImageLine(i);
		for (int j = 0; j < nWidth; j++)
		{
			rgb[0] = *pRGBline++;
			rgb[1] = *pRGBline++;
			rgb[2] = *pRGBline++;
			int Y = (rgb[2] * 29 + rgb[1] * 150 + rgb[0] * 77 + 128) >> 8;
			if (Y < 0)
				Y = 0;
			if (Y > 255)
				Y = 255;
			GetRGBBrightness += Y;
		}
	}
	return  (GetRGBBrightness / (float)(nHeight*nWidth));
}
bool MultiUcharImage::GetHistogram(unsigned int pHistB[], unsigned int pHistG[],unsigned int pHistR[],unsigned char nMaxValue)
{
	if (m_pImgData == NULL)return false;
	for (int g = 0; g < nMaxValue; g++)
	{
		pHistB[g] = pHistG[g] = pHistR[g] = 0;
	}
	unsigned char *pBGR = m_pImgData;
	for (int y = 0; y < m_nHeight; y++)
	{
		for (int x = 0; x < m_nWidth; x++)
		{
			unsigned char r = *(pBGR++);
			unsigned char g = *(pBGR++);
			unsigned char b = *(pBGR++);
			pHistB[r]++;
			pHistG[g]++;
			pHistR[b]++;
		}
	}
	return true;
}
bool MultiUcharImage::GetRectHistogram(unsigned int *nHistR, unsigned int *nHistG, unsigned int *nHistB,unsigned int maxvalue, int nLeft, int nTop, int nRight, int nBottom)
{
	int Dim = GetImageDim();
	if (m_pImgData == NULL)return false;
	for (int g = 0; g < maxvalue; g++)
	{
		nHistR[g] = 0;
		nHistG[g] = 0;
		nHistB[g] = 0;
	}
	nLeft = MAX2(0, nLeft);
	nTop = MAX2(0, nTop);
	nRight = MIN2(nRight, m_nWidth);
	nBottom = MIN2(nBottom, m_nHeight);
	if (nBottom <= nTop || nRight <= nLeft)return false;
	for (int y = nTop; y < nBottom; y++)
	{
		unsigned char *pY = GetImageLine(y);
		for (int x = nLeft; x < nRight; x++)
		{
			unsigned char r = pY[x*Dim +0];
			unsigned char g = pY[x*Dim + 1];
			unsigned char b = pY[x*Dim + 2];
			nHistR[r]++;
			nHistG[g]++;
			nHistB[b]++;
		}
	}
	return true;
}
void MultiUcharImage::HistMapping(int map[3][256], MultiUcharImage *pOutImage)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	if (pOutImage != this)
		pOutImage->CreateImage(nWidth, nHeight);
	if (nHeight > 300)
	{
#pragma omp parallel for
		for (int y = 0; y < nHeight; y++)
		{
			unsigned char *srcLine = GetImageLine(y);
			unsigned char *dstLine = pOutImage->GetImageLine(y);
			for (int x = 0; x < nWidth; x++)
			{
				for (int cl = 0; cl < 3; cl++)
					dstLine[3 * x + cl] = (unsigned char)map[cl][srcLine[3 * x + cl]];//查表用映射map将像素值映射到想要的大小
			}
		}
	}
	else
	{
		for (int y = 0; y < nHeight; y++)
		{
			unsigned char *srcLine = GetImageLine(y);
			unsigned char *dstLine = pOutImage->GetImageLine(y);
			for (int x = 0; x < nWidth; x++)
			{
				for (int cl = 0; cl < 3; cl++)
					dstLine[3 * x + cl] = (unsigned char)map[cl][srcLine[3 * x + cl]];//no matter dst==src or not
			}
		}
	}
}
float MultiUcharImage::GetMeanHue()
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	float nSumHSV0 = 0;
	float nSumHSV1 = 0;
	float nSumHSV2 = 0;
	#pragma omp parallel for  reduction(+:nSumHSV0,nSumHSV1,nSumHSV2)
	for (int y=0;y<nHeight;y++)
	{
		unsigned char *pInline = GetImageLine(y);
		for (int x = 0; x < nWidth; x++)
		{
			float tmpHSV[3] = { 0,0,0 };
			unsigned char RGB[3];
			RGB[0] = pInline[0];
			RGB[1] = pInline[1];
			RGB[2] = pInline[2];
			RGBToHSVf((float)RGB[0], (float)RGB[1], (float)RGB[2], (float)tmpHSV[0], (float)tmpHSV[1], (float)tmpHSV[2]);
			nSumHSV0 += tmpHSV[0];
			nSumHSV1 += tmpHSV[1];
			nSumHSV2 += tmpHSV[2];
			pInline += 3;
		}
	}
	nSumHSV0 = nSumHSV0 / (float)(nHeight*nWidth);
	nSumHSV1 = nSumHSV1 / (float)(nHeight*nWidth);
	nSumHSV2 = nSumHSV2 / (float)(nHeight*nWidth);
	return nSumHSV0;
}
float MultiUcharImage::GetMeanHSV_V()
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	float nSumHSV[3] = { 0,0,0 };
	float nSumHSV0 = 0;
	float nSumHSV1 = 0;
	float nSumHSV2 = 0;
#pragma omp parallel for  reduction(+:nSumHSV0,nSumHSV1,nSumHSV2)
	for (int y = 0; y < nHeight; y++)
	{
		unsigned char *pInline = GetImageLine(y);
		for (int x = 0; x < nWidth; x++)
		{
			float tmpHSV[3] = { 0,0,0 };
			unsigned char RGB[3];
			RGB[0] = pInline[0];
			RGB[1] = pInline[1];
			RGB[2] = pInline[2];
			RGBToHSVf((float)RGB[0], (float)RGB[1], (float)RGB[2], (float)tmpHSV[0], (float)tmpHSV[1], (float)tmpHSV[2]);
			nSumHSV0 += tmpHSV[0];
			nSumHSV1 += tmpHSV[1];
			nSumHSV2 += tmpHSV[2];
			pInline += 3;
		}
	}
	nSumHSV0 = nSumHSV0 / (float)(nHeight*nWidth);
	nSumHSV1 = nSumHSV1 / (float)(nHeight*nWidth);
	nSumHSV2 = nSumHSV2 / (float)(nHeight*nWidth);
	return nSumHSV2;
}
bool MultiUcharImage::GetAverageRGB(double &R, double &G, double &B)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int nSumHue = 0;
	double nSumR = 0;
	double nSumG = 0;
	double nSumB = 0;
#pragma omp parallel for reduction(+:nSumR,nSumG,nSumB)
	for (int y = 0; y < nHeight; y++)
	{
		unsigned char *pInline = GetImageLine(y);
		for (int x = 0; x < nWidth; x++)
		{
			unsigned char RGB[3];
			RGB[0] = pInline[0];
			RGB[1] = pInline[1];
			RGB[2] = pInline[2];
			nSumR += RGB[0];
			nSumG += RGB[1];
			nSumB += RGB[2];
			pInline += 3;
		}
	}
	R = nSumR / (double)(nHeight*nWidth);
	G = nSumG / (double)(nHeight*nWidth);
	B = nSumB / (double)(nHeight*nWidth);
	return true;
}
bool MultiUcharImage::GetSingleChannelImage(SingleUcharImage * pOutImage)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int nCh = GetImageDim();
	if (pOutImage->GetImageWidth() != nWidth || pOutImage->GetImageHeight() != nHeight)
	{
		if (!pOutImage->CreateImage(nWidth, nHeight))return false;
	}
	int nProcs = omp_get_num_procs();
	#pragma omp parallel for  num_threads(nProcs) schedule(dynamic,16)  
	for (int y=0;y< nHeight;y++)
	{
		unsigned char *pInLine = GetImageLine(y);
		unsigned char *pOutLine = pOutImage->GetImageLine(y);
		for (int x = 0; x < nWidth; x++)
		{
			*pOutLine++ = pInLine[0];
			pInLine += nCh;
		}
	}
	return true;
}
bool MultiUcharImage::GetAllChannelImage(SingleUcharImage *pOutImage)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int nCh = GetImageDim();
	if (nCh == 3)
	{
		if (pOutImage[0].GetImageWidth() != nWidth || pOutImage[0].GetImageHeight() != nHeight)
		{
			if (!pOutImage[0].CreateImage(nWidth, nHeight))return false;
			if (!pOutImage[1].CreateImage(nWidth, nHeight))return false;
			if (!pOutImage[2].CreateImage(nWidth, nHeight))return false;		
		}
		int nProcs = omp_get_num_procs();
		#pragma omp parallel for  num_threads(nProcs) schedule(dynamic,16)  
		for (int y = 0; y < nHeight; y++)
		{
			unsigned char *pInLine = GetImageLine(y);
			unsigned char *pOutLine0 = pOutImage[0].GetImageLine(y);
			unsigned char *pOutLine1 = pOutImage[1].GetImageLine(y);
			unsigned char *pOutLine2 = pOutImage[2].GetImageLine(y);
			for (int x = 0; x < nWidth; x++)
			{
				*pOutLine0++ = pInLine[0];
				*pOutLine1++ = pInLine[1];
				*pOutLine2++ = pInLine[2];
				pInLine += 3;
			}
		}
	}
	else
	{
		printf("can not support this dim \n");
		return false;
	}
	return true;
}
bool MultiUcharImage::ReplaceSingleChannelImage(SingleUcharImage * pInputImage)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int nCh = GetImageDim();
	if (pInputImage->GetImageWidth() != nWidth || pInputImage->GetImageHeight() != nHeight)
	{
		if (!pInputImage->CreateImage(nWidth, nHeight))return false;
	}
	int nProcs = omp_get_num_procs();
	#pragma omp parallel for  num_threads(nProcs) schedule(dynamic,16)  
	for (int y = 0; y < nHeight; y++)
	{
		unsigned char *pOutLine = GetImageLine(y);
		unsigned char *pInline = pInputImage->GetImageLine(y);
		for (int x = 0; x < nWidth; x++)
		{
			pOutLine[0]= *pInline++;
			pOutLine += nCh;
		}
	}
	return true;
}
bool MultiUcharImage::ReplaceAllChannelImage(SingleUcharImage *pInputImage)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int nCh = GetImageDim();
	if (nCh == 3)
	{
		if (pInputImage[0].GetImageWidth() != nWidth || pInputImage[0].GetImageHeight() != nHeight)
		{
			if (!pInputImage[0].CreateImage(nWidth, nHeight))return false;
			if (!pInputImage[1].CreateImage(nWidth, nHeight))return false;
			if (!pInputImage[2].CreateImage(nWidth, nHeight))return false;
		}
		int nProcs = omp_get_num_procs();
		#pragma omp parallel for  num_threads(nProcs) schedule(dynamic,16)  
		for (int y = 0; y < nHeight; y++)
		{
			unsigned char *pOutLine = GetImageLine(y);
			unsigned char *pInline0 = pInputImage[0].GetImageLine(y);
			unsigned char *pInline1 = pInputImage[1].GetImageLine(y);
			unsigned char *pInline2 = pInputImage[2].GetImageLine(y);
			for (int x = 0; x < nWidth; x++)
			{
				 pOutLine[0] = *pInline0++;
				 pOutLine[1] = *pInline1++;
				 pOutLine[2] = *pInline2++;
				 pOutLine += 3;
			}
		}
	}
	else
	{
		printf("can not support this dim \n");
		return false;
	}
	return true;
}
bool MultiUcharImage::BoxFilterImage(MultiUcharImage * pOutImage, int nRadius)
{
	SingleUcharImage ChImage[3];
	SingleUcharImage OutChImage[3];
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	if (pOutImage->GetImageWidth() != nWidth || pOutImage->GetImageHeight() != nHeight)
	{
		if (!pOutImage->CreateImage(nWidth, nHeight))return false;
	}
	GetAllChannelImage(ChImage);
	ChImage[0].BoxFilterImage(&OutChImage[0],nRadius);
	ChImage[1].BoxFilterImage(&OutChImage[1], nRadius);
	ChImage[2].BoxFilterImage(&OutChImage[2], nRadius);
	pOutImage->ReplaceAllChannelImage(OutChImage);
	return true;
}
bool MultiUcharImage::GaussianBlur3x3Image(MultiUcharImage * pOutImage)
{
	/*
	kernel
	1 2 1
	2 4 2
	1 2 1
	*/
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int nCh = GetImageDim();
	if (pOutImage->GetImageWidth() != nWidth || pOutImage->GetImageHeight() != nHeight)
	{
		if (!pOutImage->CreateImage(nWidth, nHeight))return false;
	}
	int nProcs = omp_get_num_procs();
	unsigned short *pBuffer = new unsigned short[(nWidth * 3)*nProcs*nCh];
	int nThreads = nProcs;
	unsigned short *pLines[3];
	int loop = 0;
	#pragma omp parallel for num_threads(nProcs) firstprivate(loop) private(pLines) 
	for (int y = 0; y < nHeight; y++)
	{
		int i;
		int nThreadId = omp_get_thread_num();
		if (loop == 0)
		{
			for (i = 0; i < 3; i++)
			{
				pLines[i] = pBuffer + (nThreadId * 3 + i)*nWidth*nCh;
			}
			for (i = -1; i < 1; i++)
			{
				HGaussianLine3(GetImageLine(y + i), pLines[i + 1], nWidth, nCh);
			}
			loop++;
		}
		HGaussianLine3(GetImageLine(y + 1), pLines[2], nWidth, nCh);
		VGaussianLine3(pLines, pOutImage->GetImageLine(y), nWidth, nCh);
		unsigned short *pTemp = pLines[0];
		for (i = 0; i < 2; i++)
		{
			pLines[i] = pLines[i + 1];
		}
		pLines[2] = pTemp;
	}
	delete[]pBuffer;
	return true;
}
bool MultiUcharImage::GaussianBlur5x5Image(MultiUcharImage * pOutImage)
{
	/*
	kernel
	1  4  6  4  1
	4 16 24 16  4
	6 24 36 24  6
	4 16 24 16  4
	1  4  6  4  1
	*/
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int nCh = GetImageDim();
	if (pOutImage->GetImageWidth() != nWidth || pOutImage->GetImageHeight() != nHeight)
	{
		if (!pOutImage->CreateImage(nWidth, nHeight))return false;
	}
	int nProcs = omp_get_num_procs();
	unsigned short *pBuffer = new unsigned short[(nWidth * 5)*nProcs*nCh];
	int nThreads = nProcs;
	unsigned short *pLines[5];
	int loop = 0;
#pragma omp parallel for num_threads(nProcs) firstprivate(loop) private(pLines) 
	for (int y = 0; y < nHeight; y++)
	{
		int i;
		int nThreadId = omp_get_thread_num();
		if (loop == 0)
		{
			for (i = 0; i < 5; i++)
			{
				pLines[i] = pBuffer + (nThreadId * 5 + i)*nWidth*nCh;
			}
			for (i = -2; i < 2; i++)
			{
				HGaussianLine5(GetImageLine(y + i), pLines[i + 2], nWidth, nCh);
			}
			loop++;
		}
		unsigned char *pInLine = GetImageLine(y + 2);
		unsigned char  *pOutLine = pOutImage->GetImageLine(y);
		HGaussianLine5(pInLine, pLines[4], nWidth, nCh);
		VGaussianLine5(pLines, pOutLine, nWidth, nCh);
		unsigned short *pTemp = pLines[0];
		for (i = 0; i < 4; i++)
		{
			pLines[i] = pLines[i + 1];
		}
		pLines[4] = pTemp;
	}
	delete[]pBuffer;
	return true;
}
bool MultiUcharImage::Bilateral5x5Image(MultiUcharImage *pOutImage, int nThre)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int nCh = GetImageDim();
	if (pOutImage->GetImageWidth() != nWidth || pOutImage->GetImageHeight() != nHeight)
	{
		if (!pOutImage->CreateImage(nWidth, nHeight))return false;
	}
	const int nMask[5][5] =
	{
		{ 1,  4,  6,  4, 1},
		{ 4, 16, 24, 16, 4},
		{ 6, 24, 36, 24, 6},
		{ 4, 16, 24, 16, 4},
		{ 1,  4,  6,  4, 1}
	};
	unsigned int nInvNoise = (1U << 28) / (nThre*nThre);
	int nThread = omp_get_num_procs();
#pragma omp parallel for num_threads(nThread) 
	for (int y = 0; y < nHeight; y++)
	{
		int i, j, x, Y0,Y, R,G,B;
		unsigned char *pIn = GetImageLine(y);
		unsigned char *pOut = pOutImage->GetImageLine(y);
		for (x = 0; x < nWidth; x++)
		{
			long long sumR, sumG, sumB, dY;
			int W, sumW;
			Y0 = (pIn[2] * 29 + pIn[1] * 150 + pIn[0] * 77 + 128) >> 8;
			sumR = 0;
			sumG = 0;
			sumB = 0;
			sumW = 0;
			for (i = -2; i <= 2; i++)
			{
				if (y + i < 0 || y + i >= nHeight)continue;
				for (j = -2; j <= 2; j++)
				{
					if (x + j < 0 || x + j >= nWidth)continue;
					R = pIn[i*nWidth*nCh + j * nCh];
					G = pIn[i*nWidth*nCh + j * nCh+1];
					B = pIn[i*nWidth*nCh + j * nCh+2];
					Y = (B * 29 + G * 150 + R * 77 + 128) >> 8;
					dY = Y - Y0;
					dY *= dY;
					W = (int)(8 - ((dY*nInvNoise) >> 28));
					if (W >= 0)
					{
						W = nMask[i + 2][j + 2] << W;
						sumR += R * W;
						sumG += G * W;
						sumB += B * W;
						sumW += W;
					}
				}
			}
			*(pOut++) = (int)(sumR / sumW);
			*(pOut++) = (int)(sumG / sumW);
			*(pOut++) = (int)(sumB / sumW);
			pIn+=3;
		}
	}
	return true;
}
bool MultiUcharImage::Extend2Image(int nS)
{
	MultiUcharImage tmpOutImage;
	if (nS < 0)return false;
	int nMask = (1 << nS) - 1;
	int nInWidth = GetImageWidth();
	int nInHeight = GetImageHeight();
	int nCh = GetImageDim();
	int nOutWidth = nInWidth + (((1 << nS) - (nInWidth&nMask))&nMask);
	int nOutHeight = nInHeight + (((1 << nS) - (nInHeight&nMask))&nMask);
	if (tmpOutImage.GetImageWidth() != nOutWidth || tmpOutImage.GetImageHeight() != nOutHeight)
	{
		if (!tmpOutImage.CreateImage(nOutWidth, nOutHeight))return false;
	}
	FillUcharData(GetImageData(), tmpOutImage.GetImageData(), nInWidth, nInHeight, nOutWidth, nOutHeight, nCh);
	this->Clone(&tmpOutImage);
	return true;
}
bool MultiUcharImage::SubtractEdgeImage(MultiUcharImage *pInImage, MultiShortImage *pOutImage)
{
	int nWidth0 = GetImageWidth();
	int nHeight0 = GetImageHeight();
	int nCh = GetImageDim();
	int nWidth1 = pInImage->GetImageWidth();
	int nHeight1 = pInImage->GetImageHeight();
	if (nWidth1 < nWidth0 || nHeight1 < nHeight0)return false;
	if (!pOutImage->CreateImage(nWidth0, nHeight0, nCh))return false;
	SubtractUcharEdgeData(GetImageData(), pInImage->GetImageData(), pOutImage->GetImageData(), nWidth0, nHeight0, nWidth1, nHeight1, nCh);
	return true;
}
bool MultiUcharImage::DownScaleImagex2(MultiUcharImage *pOutImage, bool bDitheringEnable = false)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int nCh = GetImageDim();

	if ((nWidth & 1) == 1 || (nHeight & 1) == 1)
	{
		Extend2Image(1);
		nWidth = GetImageWidth();
		nHeight = GetImageHeight();
	}
	if (!pOutImage->CreateImage(nWidth >> 1, nHeight >> 1))return false;
	return DownScaleUcharDatax2(GetImageData(), pOutImage->GetImageData(), nWidth, nHeight, nCh, bDitheringEnable);
}
bool MultiUcharImage::UpScaleImagex2(MultiUcharImage *pOutImage, bool bDitheringEnable = false)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int nCh = GetImageDim();
	if (!pOutImage->CreateImage(nWidth * 2, nHeight * 2))return false;
	return UpScaleUcharDatax2(GetImageData(), pOutImage->GetImageData(), nWidth, nHeight, nCh, bDitheringEnable);
}
bool MultiUcharImage::AddBackEdgeImage(MultiUcharImage *pInputImage, MultiShortImage *pInputEdgeImage)
{
	int nWidth = pInputImage->GetImageWidth();
	int nHeight = pInputImage->GetImageHeight();
	int nCh = pInputImage->GetImageDim();
	int nEdgeWidth = pInputEdgeImage->GetImageWidth();
	int nEdgeHeight = pInputEdgeImage->GetImageHeight();
	if (nEdgeWidth > nWidth || nEdgeHeight > nHeight || pInputEdgeImage->GetImageDim() != 1)return false;
	if (!this->CreateImage(nEdgeWidth, nEdgeHeight))return false;
	AddBackUcharEdge(pInputImage->GetImageData(), pInputEdgeImage->GetImageData(), GetImageData(), nWidth, nHeight, nEdgeWidth, nEdgeHeight, nCh);
	return true;
}
bool MultiUcharImage::GaussPyramidImage(MultiUcharImage *pOutPyramid, MultiShortImage *pOutEdgePyramid, int &nPyramidLevel)
{
	int nWidth[12], nHeight[12];
	MultiUcharImage  TempImage;
	printf("input nPyramidLevel=%d\n", nPyramidLevel);
	pOutPyramid[0].Clone(this);
	for (int i = 0; i < nPyramidLevel; i++)
	{
		nWidth[i] = pOutPyramid[i].GetImageWidth();
		nHeight[i] = pOutPyramid[i].GetImageHeight();
		printf("Level=%d Size=[%d,%d]\n", i, nWidth[i], nHeight[i]);
		if (nWidth[i] <= 4 || nHeight[i] <= 4)
		{
			nPyramidLevel = i;
			printf("out PyramidLevel=%d\n", nPyramidLevel);
			break;
		}
		if (!pOutPyramid[i].Extend2Image(1))return false;
		if (!pOutPyramid[i].DownScaleImagex2(&pOutPyramid[i + 1], false))return false;
		if (!pOutPyramid[i + 1].UpScaleImagex2(&TempImage, false))return false;
		if (!pOutPyramid[i].SubtractEdgeImage(&TempImage, pOutEdgePyramid + i))return false;
	}
	return true;
}
bool MultiUcharImage::ResizeCubicImage(MultiUcharImage *pOutImage, int OutnWidth, int OutnHeight)
{
	int nInWidth = GetImageWidth();
	int nInHeight = GetImageHeight();
	int nChannel = GetImageDim();
	if (pOutImage->GetImageWidth() != OutnWidth || pOutImage->GetImageHeight() != OutnHeight || pOutImage->GetImageDim() != nChannel)
	{
		if (!pOutImage->SetImageSize(OutnWidth, OutnHeight, nChannel))return false;
	}
	return RescaleCubicData(GetImageData(), pOutImage->GetImageData(), nChannel, nInWidth, nInHeight, OutnWidth, OutnHeight);
}
bool MultiUcharImage::RepImageWithRect( MultiUcharImage *Rectimg, int nLeft, int nTop, int nRight, int nBottom)
{
	int rectnWidth = Rectimg->GetImageWidth();
	for (int y = nTop; y < nBottom; y++)
	{
		unsigned char *pOut = GetImageLine(y);
		unsigned char *pIn = Rectimg->GetImageLine(y - nTop);
		for (int x = nLeft; x < nRight; x++)
		{
			int tx = x - nLeft;
			if (tx < 0)
				tx = 0;
			else if (tx >= rectnWidth)
				tx = rectnWidth - 1;

			for (int i = 0; i < 3; i++)
			{
				pOut[x*3+i] = pIn[tx * 3 + i];
			}
		}
	}
	return true;
}
bool MultiUcharImage::ClearImageFollowMask(unsigned char *pInMaskData,int nx,int ny)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	if (nWidth!=nx|| nHeight!=ny)
	{
		return false;
	}
	for (int y = 0; y < nHeight; y++)
	{
		unsigned char *pMainline = GetImageLine(y);
		unsigned char *pInputMaskline = pInMaskData +y*nx;
		for (int x = 0; x < nWidth;x++)
		{
			if (pInputMaskline[0] ==0)
			{
				pMainline[0] = 0;
				pMainline[1] = 0;
				pMainline[2] = 0;
			}
			pMainline+=3;
			pInputMaskline++;
		}
	}
	return true;
}
bool MultiUcharImage::ClearImageFollowMaskThreshold(unsigned char *pInputMaskData, int nx, int ny,int thre)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	if (nWidth != nx || nHeight != ny)
	{
		return false;
	}
	for (int y = 0; y < nHeight; y++)
	{
		unsigned char *pMainline = GetImageLine(y);
		unsigned char *pInputMaskline = pInputMaskData + y * nx;
		for (int x = 0; x < nWidth; x++)
		{
			if (pInputMaskline[0] >= thre)
			{
				pMainline[0] = 0;
				pMainline[1] = 0;
				pMainline[2] = 0;
			}
			pMainline += 3;
			pInputMaskline++;
		}
	}
	return true;
}
bool MultiUcharImage::ClearImageFollowMaskUpDownThre(unsigned char *pInMask, int nx, int ny, int up, int down,int thre)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	if (nWidth != nx || nHeight != ny)
	{
		return false;
	}
	int newup, newdown;
	newup = up - thre; 
	newdown = down + thre;
	if (newup < 0) 
	{ 
		newup = 0;
	}
	if (newdown > nHeight) 
	{ 
		newdown = nHeight;
	}
	for (int y = 0; y < nHeight; y++)
	{
		unsigned char *pMainline = GetImageLine(y);
		if (y<newup || y>newdown)
		{
			for (int x = 0; x < nWidth; x++)
			{
				pMainline[0] = 0;
				pMainline[1] = 0;
				pMainline[2] = 0;
				pMainline += 3;
			}
		}
	}
	printf(" clearmask over! newup newdown thre %d  %d %d\n", newup, newdown, thre);
	return true;
}
bool MultiUcharImage::GetMeanYVal(float &value)
{
	MultiUcharImage tmpYUV;
	tmpYUV.Clone(this);
#ifdef USE_NEON
	tmpYUV.RGB2YCbCrOPT();
#else
	tmpYUV.RGB2YCbCr();
#endif
	Yuv420Image YUVImage;
	if (!YUVImage.YUV444ToYUV420(&tmpYUV))return false;
	SingleUcharImage tmpY;
	YUVImage.GetYImage(&tmpY);
	value=(float)(tmpY.GetMeanVal());
	return true;
}
bool MultiUcharImage::GetMeanRGBVal(float &Rvalue, float &Gvalue, float &Bvalue)
{
	SingleUcharImage ChImage[3];
	GetAllChannelImage(ChImage);
	Rvalue=ChImage[0].GetMeanVal();
	Gvalue = ChImage[1].GetMeanVal();
	Bvalue = ChImage[2].GetMeanVal();
	return true;
}
bool MultiUcharImage::DivideAWBGain(float Rgain, float Ggain, float Bgain)
{
	if (Rgain<=0.000001|| Ggain <= 0.000001|| Bgain <= 0.000001)
	{
		return false;
	}
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	#pragma omp parallel for
	for (int y = 0; y < nHeight; y++)
	{
		unsigned char *pInline = GetImageLine(y);
		for (int x = 0; x < nWidth; x++)
		{
			unsigned char RGB[3];
			RGB[0] = pInline[0];
			RGB[1] = pInline[1];
			RGB[2] = pInline[2];
			pInline[0] = (unsigned char)(float)RGB[0] / Rgain;
			pInline[1] = (unsigned char)(float)RGB[1] / Ggain;
			pInline[2] = (unsigned char)(float)RGB[2] / Bgain;
			pInline += 3;
		}
	}
	return true;
}
bool MultiUcharImage::ApplyAWBGain(float Rgain, float Ggain, float Bgain)
{
	if (Rgain <= 0.000001 || Ggain <= 0.000001 || Bgain <= 0.000001)
	{
		return false;
	}
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
#pragma omp parallel for
	for (int y = 0; y < nHeight; y++)
	{
		unsigned char *pInline = GetImageLine(y);
		for (int x = 0; x < nWidth; x++)
		{
			unsigned short tmpr = pInline[0] * Rgain;
			unsigned short tmpg = pInline[1] * Ggain;
			unsigned short tmpb = pInline[2] * Bgain;
			if (tmpr>255)
			{
				tmpr = 255;
			}
			if (tmpg > 255)
			{
				tmpg = 255;
			}
			if (tmpb > 255)
			{
				tmpb = 255;
			}
			pInline[0] = (unsigned char)tmpr;
			pInline[1] = (unsigned char)tmpg;
			pInline[2] = (unsigned char)tmpb;
			pInline += 3;
		}
	}
	return true;
}
bool MultiUcharImage::ClearImageFollowRectOutside(int nLeft, int nTop, int  nRight, int nBottom)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int dim = GetImageDim();
	if (nRight >= nWidth || nBottom>=nHeight)
	{
		return false;
	}
	if (dim==3)
	{
		for (int y = 0; y < nHeight; y++)
		{
			unsigned char *pMainline = GetImageLine(y);
			for (int x = 0; x < nWidth; x++)
			{
				if (x >= nLeft && x <= nRight && y >= nTop && y <= nBottom)
				{
				}
				else
				{
					pMainline[x * 3 + 0] = 0;
					pMainline[x * 3 + 1] = 0;
					pMainline[x * 3 + 2] = 0;
				}
			}
		}
	}
	else if(dim == 1)
	{
		for (int y = 0; y < nHeight; y++)
		{
			unsigned char *pMainline = GetImageLine(y);
			for (int x = 0; x < nWidth; x++)
			{
				if (x >= nLeft && x <= nRight && y >= nTop && y <= nBottom)
				{
				}
				else
				{
					pMainline[x ] = 0;
				}
			}
		}
	}
	return true;
}
void MultiUcharImage::YCbCr2BGROPT()
{
	int nThread = omp_get_num_procs();
	#pragma omp parallel for num_threads(8) schedule(dynamic,32)
	for (int y = 0; y < m_nHeight; y++)
	{
		int i, x, YUV[3], BGR[3];
		unsigned char *pLine = GetImageLine(y);
		x = 0;
#ifdef USE_NEON
		int tmpx = m_nWidth / 8 * 8;
		int16x8_t v128 = vdupq_n_s16(128);
		int16x8_t v255 = vdupq_n_s16(255);
		int32x4_t v0 = vdupq_n_s32(0);
		int32x4_t  vS, vT0, vT1, vT2;
		uint8x8x3_t vPixel;
		int16x8x3_t vYUV;
		int16x4x3_t vBGRL, vBGRH;
		for (; x < tmpx; x += 8)
		{
			vPixel = vld3_u8(pLine);
			vYUV.val[0] = vreinterpretq_s16_u16(vmovl_u8(vPixel.val[0]));
			vYUV.val[1] = vreinterpretq_s16_u16(vmovl_u8(vPixel.val[2]));
			vYUV.val[2] = vreinterpretq_s16_u16(vmovl_u8(vPixel.val[1]));

			vYUV.val[1] = vsubq_s16(vYUV.val[1], v128);
			vYUV.val[2] = vsubq_s16(vYUV.val[2], v128);

			vBGRL.val[0] = vget_low_s16(vYUV.val[0]);
			vBGRL.val[1] = vget_low_s16(vYUV.val[2]);
			vBGRL.val[2] = vget_low_s16(vYUV.val[1]);
			vS = vshll_n_s16(vBGRL.val[0], 11);
			vT0 = vmlal_n_s16(vS, vBGRL.val[1], 4096 - 467);
			vT2 = vmlal_n_s16(vS, vBGRL.val[2], 4096 - 1225);
			vT1 = vmlal_n_s16(vS, vBGRL.val[1], -705);
			vT1 = vmlal_n_s16(vT1, vBGRL.val[2], -1463);
			vT0 = vmaxq_s32(vT0, v0);
			vT1 = vmaxq_s32(vT1, v0);
			vT2 = vmaxq_s32(vT2, v0);
			vBGRL.val[0] = vshrn_n_s32(vT0, 11);
			vBGRL.val[1] = vshrn_n_s32(vT1, 11);
			vBGRL.val[2] = vshrn_n_s32(vT2, 11);

			vBGRH.val[0] = vget_high_s16(vYUV.val[0]);
			vBGRH.val[1] = vget_high_s16(vYUV.val[2]);
			vBGRH.val[2] = vget_high_s16(vYUV.val[1]);
			vS = vshll_n_s16(vBGRH.val[0], 11);
			vT0 = vmlal_n_s16(vS, vBGRH.val[1], 4096 - 467);
			vT2 = vmlal_n_s16(vS, vBGRH.val[2], 4096 - 1225);
			vT1 = vmlal_n_s16(vS, vBGRH.val[1], -705);
			vT1 = vmlal_n_s16(vT1, vBGRH.val[2], -1463);
			vT0 = vmaxq_s32(vT0, v0);
			vT1 = vmaxq_s32(vT1, v0);
			vT2 = vmaxq_s32(vT2, v0);
			vBGRH.val[0] = vshrn_n_s32(vT0, 11);
			vBGRH.val[1] = vshrn_n_s32(vT1, 11);
			vBGRH.val[2] = vshrn_n_s32(vT2, 11);
			vPixel.val[0] = vmovn_u16(vreinterpretq_u16_s16(vminq_s16(vcombine_s16(vBGRL.val[0], vBGRH.val[0]), v255)));
			vPixel.val[1] = vmovn_u16(vreinterpretq_u16_s16(vminq_s16(vcombine_s16(vBGRL.val[1], vBGRH.val[1]), v255)));
			vPixel.val[2] = vmovn_u16(vreinterpretq_u16_s16(vminq_s16(vcombine_s16(vBGRL.val[2], vBGRH.val[2]), v255)));
			vst3_u8(pLine, vPixel);
			pLine += 24;
		}
#endif
		for (; x < m_nWidth; x++)
		{
			YUV[0] = pLine[0];
			YUV[1] = pLine[1];
			YUV[2] = pLine[2];
			YUV[1] -= 128;
			YUV[2] -= 128;
			BGR[0] = YUV[0] * 2048 + YUV[1] * (4096 - 467);
			BGR[2] = YUV[0] * 2048 + YUV[2] * (4096 - 1225);
			BGR[1] = YUV[0] * 2048 - YUV[1] * 705 - YUV[2] * 1463;
			for (i = 0; i < 3; i++)
			{
				if (BGR[i] < 0)BGR[i] = 0;
				BGR[i] >>= 11;
				if (BGR[i] > 255)BGR[i] = 255;
				pLine[i] = (unsigned char)BGR[i];
			}
			pLine += 3;
		}
	}
}
void MultiUcharImage::YCbCr2RGB()
{
	int nThread = omp_get_num_procs();
	#pragma omp parallel for num_threads(8) schedule(dynamic,32)
	for (int y = 0; y < m_nHeight; y++)
	{
		int i, x, YUV[3], BGR[3];
		//unsigned char *pLine = datain + y * m_nWidth * 3;
		unsigned char *pLine = GetImageLine(y);
		x = 0;
		int tmpx = m_nWidth / 8 * 8;
		for (; x < m_nWidth; x++)
		{
			YUV[0] = pLine[0];
			YUV[1] = pLine[1];
			YUV[2] = pLine[2];
			YUV[1] -= 128;
			YUV[2] -= 128;
			BGR[2] = YUV[0] * 2048 + YUV[1] * (4096 - 467);
			BGR[0] = YUV[0] * 2048 + YUV[2] * (4096 - 1225);
			BGR[1] = YUV[0] * 2048 - YUV[1] * 705 - YUV[2] * 1463;
			for (i = 0; i < 3; i++)
			{
				if (BGR[i] < 0)BGR[i] = 0;
				BGR[i] >>= 11;
				if (BGR[i] > 255)BGR[i] = 255;
				pLine[i] = (unsigned char)BGR[i];
			}
	
			pLine += 3;
		}
	}
}
void MultiUcharImage::RGB2YCbCr()
{
	int nThread = omp_get_num_procs();
#pragma omp parallel for num_threads(nThread) schedule(dynamic,32)
	for (int y = 0; y < m_nHeight; y++)
	{
		unsigned char *pLine = GetImageLine(y);
		int x = 0;
		for (; x < m_nWidth; x++, pLine += 3)
		{
			int  yuv[3], bgr[3];
			for (int i = 0; i < 3; i++)
			{
				bgr[i] = pLine[i];
			}

			yuv[0] = (bgr[2] * 29 + bgr[1] * 150 + bgr[0] * 77 + 128) >> 8;
			yuv[1] = (bgr[2] * 128 - bgr[1] * 85 - bgr[0] * 43) / 256;	yuv[1] += 128;
			yuv[2] = (bgr[0] * 128 - bgr[1] * 107 - bgr[2] * 21) / 256;	yuv[2] += 128;

			for (int i = 0; i < 3; i++)
			{
				if (yuv[i] < 0)yuv[i] = 0;
				if (yuv[i] > 255)yuv[i] = 255;
				pLine[i] = (unsigned char)yuv[i];
			}
		}
	}
}
void MultiUcharImage::BGR2YCbCr()
{
	int i, x, y, yuv[3], bgr[3];
	for(y=0; y<m_nHeight; y++)
	{
		unsigned char *pLine=GetImageLine(y);
		for(x=0; x<m_nWidth; x++, pLine+=3)
		{
			for(i=0; i<3; i++)
			{
				bgr[i]=pLine[i];
			}	
			yuv[0]=(bgr[2]*4897+bgr[1]*9611+bgr[0]*1876+8192)>>14;
			yuv[1]=bgr[0]-yuv[0]+128;
			yuv[2]=bgr[2]-yuv[0]+128;
			for(i=0; i<3; i++)
			{
				if(yuv[i]<0)yuv[i]=0;	
				if(yuv[i]>255)yuv[i]=255;
				pLine[i]=(unsigned char) yuv[i];
			}
		}
	}
}
void MultiUcharImage::YCbCr2BGR()
{
	int i, x, y, yuv[3], bgr[3];
	for(y=0; y<m_nHeight; y++)
	{
		unsigned char *pLine=GetImageLine(y);
		for(x=0; x<m_nWidth; x++, pLine+=3)
		{
			yuv[0]=pLine[0];
			yuv[1]=pLine[1];	yuv[1]-=128;
			yuv[2]=pLine[2];	yuv[2]-=128;
			bgr[0]=yuv[1]+yuv[0];
			bgr[2]=yuv[2]+yuv[0];
			bgr[1]=yuv[0]-((3141*yuv[1]+8350*yuv[2])>>14);
			for(i=0; i<3; i++)
			{
				if(bgr[i]<0)bgr[i]=0;	if(bgr[i]>255)bgr[i]=255;
				pLine[i]=(unsigned char) bgr[i];
			}
		}
	}
}
void MultiUcharImage::BGR2YCbCrOPT()
{
#ifdef USE_NEON
	uint16x8_t vuConst128 = vdupq_n_u16(128);
	int16x8_t vsConst128 = vdupq_n_s16(128);
	int16x8_t vsConst0 = vdupq_n_s16(0);
	int16x8_t vsConst255 = vdupq_n_s16(255);
	uint16x8_t vConst1 = vdupq_n_u16(1);
#endif // USE_NEON_X
	int nThread = omp_get_num_procs();
#pragma omp parallel for num_threads(nThread) schedule(dynamic,32)
	for (int y = 0; y < m_nHeight; y++)
	{
		unsigned char *pLine = GetImageLine(y);
		int x = 0;
#ifdef USE_NEON

		for (; x < m_nWidth - 7; x += 8, pLine += 24)
		{
			uint8x8x3_t vbgr = vld3_u8(pLine);
			uint16x8_t vu16BGR0 = vmovl_u8(vbgr.val[0]);
			uint16x8_t vu16BGR1 = vmovl_u8(vbgr.val[1]);
			uint16x8_t vu16BGR2 = vmovl_u8(vbgr.val[2]);
			uint16x8_t vY0 = vmulq_n_u16(vu16BGR0, 29);

			vY0 = vmlaq_n_u16(vY0, vu16BGR1, 150);
			vY0 = vmlaq_n_u16(vY0, vu16BGR2, 77);
			//vmlal_u8(vY0, vbgr[0], vdupq_n_u8(29));
			vY0 = vaddq_u16(vY0, vuConst128);
			int16x8_t vYUV16[3];

			vYUV16[0] = vreinterpretq_s16_u16(vu16BGR0);
			vYUV16[1] = vreinterpretq_s16_u16(vu16BGR1);
			vYUV16[2] = vreinterpretq_s16_u16(vu16BGR2);

			int16x8_t vY1, vY2;

			//	//yuv[0] = (bgr[0] * 29 + bgr[1] * 150 + bgr[2] * 77 + 128) >> 8;
			vY1 = vmulq_s16(vYUV16[0], vsConst128);
			vY1 = vmlsq_n_s16(vY1, vYUV16[1], 85);
			vY1 = vmlsq_n_s16(vY1, vYUV16[2], 43);
			uint16x8_t cmp = vcltq_s16(vY1, vsConst0);
			int16x8_t vTMP = vmulq_s16(vsConst255, vreinterpretq_s16_u16(vandq_u16(vConst1, cmp)));
			vY1 = vaddq_s16(vY1, vTMP);
			vY1 = vshrq_n_s16(vY1, 8);
			vY1 = vaddq_s16(vY1, vsConst128);
			////	yuv[1] = (bgr[0] * 128 - bgr[1] * 85 - bgr[2] * 43) / 256;	yuv[1] += 128;
			//	
			vY2 = vmulq_s16(vYUV16[2], vsConst128);
			vY2 = vmlsq_n_s16(vY2, vYUV16[1], 107);
			vY2 = vmlsq_n_s16(vY2, vYUV16[0], 21);
			cmp = vcltq_s16(vY2, vsConst0);
			vTMP = vmulq_s16(vsConst255, vreinterpretq_s16_u16(vandq_u16(vConst1, cmp)));

			vY2 = vaddq_s16(vY2, vTMP);
			vY2 = vshrq_n_s16(vY2, 8);
			vY2 = vaddq_s16(vY2, vsConst128);
			//	yuv[2] = (bgr[2] * 128 - bgr[1] * 107 - bgr[0] * 21) / 256;	yuv[2] += 128;

			uint8x8x3_t vOUT;
			vOUT.val[0] = vshrn_n_u16(vY0, 8);

			vOUT.val[1] = vqmovun_s16(vY1);
			vOUT.val[2] = vqmovun_s16(vY2);
			vst3_u8(pLine, vOUT);
		}
#endif // USE_NEON

		for (; x < m_nWidth; x++, pLine += 3)
		{
			int  yuv[3], bgr[3];
			for (int i = 0; i < 3; i++)
			{
				bgr[i] = pLine[i];
			}
			yuv[0] = (bgr[0] * 29 + bgr[1] * 150 + bgr[2] * 77 + 128) >> 8;
			yuv[1] = (bgr[0] * 128 - bgr[1] * 85 - bgr[2] * 43) / 256;	yuv[1] += 128;
			yuv[2] = (bgr[2] * 128 - bgr[1] * 107 - bgr[0] * 21) / 256;	yuv[2] += 128;
			for (int i = 0; i < 3; i++)
			{
				if (yuv[i] < 0)yuv[i] = 0;
				if (yuv[i] > 255)yuv[i] = 255;
				pLine[i] = (unsigned char)yuv[i];
			}
		}
	}
}
void MultiUcharImage::RGB2YCbCrOPT()
{
#ifdef USE_NEON
	uint16x8_t vuConst128 = vdupq_n_u16(128);
	int16x8_t vsConst128 = vdupq_n_s16(128);
	int16x8_t vsConst0 = vdupq_n_s16(0);
	int16x8_t vsConst255 = vdupq_n_s16(255);
	uint16x8_t vConst1 = vdupq_n_u16(1);
#endif // USE_NEON_X
	int nThread = omp_get_num_procs();
#pragma omp parallel for num_threads(nThread) schedule(dynamic,32)
	for (int y = 0; y < m_nHeight; y++)
	{
		unsigned char *pLine = GetImageLine(y);
		int x = 0;
//#ifdef USE_NEON
//
//		for (; x < m_nWidth - 7; x += 8, pLine += 24)
//		{
//			uint8x8x3_t vbgr = vld3_u8(pLine);
//			uint16x8_t vu16BGR0 = vmovl_u8(vbgr.val[0]);
//			uint16x8_t vu16BGR1 = vmovl_u8(vbgr.val[1]);
//			uint16x8_t vu16BGR2 = vmovl_u8(vbgr.val[2]);
//			uint16x8_t vY0 = vmulq_n_u16(vu16BGR0, 29);
//
//			vY0 = vmlaq_n_u16(vY0, vu16BGR1, 150);
//			vY0 = vmlaq_n_u16(vY0, vu16BGR2, 77);
//			//vmlal_u8(vY0, vbgr[0], vdupq_n_u8(29));
//			vY0 = vaddq_u16(vY0, vuConst128);
//			int16x8_t vYUV16[3];
//
//			vYUV16[0] = vreinterpretq_s16_u16(vu16BGR0);
//			vYUV16[1] = vreinterpretq_s16_u16(vu16BGR1);
//			vYUV16[2] = vreinterpretq_s16_u16(vu16BGR2);
//
//			int16x8_t vY1, vY2;
//
//			//	//yuv[0] = (bgr[0] * 29 + bgr[1] * 150 + bgr[2] * 77 + 128) >> 8;
//			vY1 = vmulq_s16(vYUV16[0], vsConst128);
//			vY1 = vmlsq_n_s16(vY1, vYUV16[1], 85);
//			vY1 = vmlsq_n_s16(vY1, vYUV16[2], 43);
//			uint16x8_t cmp = vcltq_s16(vY1, vsConst0);
//			int16x8_t vTMP = vmulq_s16(vsConst255, vreinterpretq_s16_u16(vandq_u16(vConst1, cmp)));
//			vY1 = vaddq_s16(vY1, vTMP);
//			vY1 = vshrq_n_s16(vY1, 8);
//			vY1 = vaddq_s16(vY1, vsConst128);
//			////	yuv[1] = (bgr[0] * 128 - bgr[1] * 85 - bgr[2] * 43) / 256;	yuv[1] += 128;
//			//	
//			vY2 = vmulq_s16(vYUV16[2], vsConst128);
//			vY2 = vmlsq_n_s16(vY2, vYUV16[1], 107);
//			vY2 = vmlsq_n_s16(vY2, vYUV16[0], 21);
//			cmp = vcltq_s16(vY2, vsConst0);
//			vTMP = vmulq_s16(vsConst255, vreinterpretq_s16_u16(vandq_u16(vConst1, cmp)));
//
//			vY2 = vaddq_s16(vY2, vTMP);
//			vY2 = vshrq_n_s16(vY2, 8);
//			vY2 = vaddq_s16(vY2, vsConst128);
//			//	yuv[2] = (bgr[2] * 128 - bgr[1] * 107 - bgr[0] * 21) / 256;	yuv[2] += 128;
//
//			uint8x8x3_t vOUT;
//			vOUT.val[0] = vshrn_n_u16(vY0, 8);
//
//			vOUT.val[1] = vqmovun_s16(vY1);
//			vOUT.val[2] = vqmovun_s16(vY2);
//			vst3_u8(pLine, vOUT);
//		}
//#endif // USE_NEON

		for (; x < m_nWidth; x++, pLine += 3)
		{
			int  yuv[3], bgr[3];
			for (int i = 0; i < 3; i++)
			{
				bgr[i] = pLine[i];
			}

			yuv[0] = (bgr[2] * 29 + bgr[1] * 150 + bgr[0] * 77 + 128) >> 8;
			yuv[1] = (bgr[2] * 128 - bgr[1] * 85 - bgr[0] * 43) / 256;	yuv[1] += 128;
			yuv[2] = (bgr[0] * 128 - bgr[1] * 107 - bgr[2] * 21) / 256;	yuv[2] += 128;

			for (int i = 0; i < 3; i++)
			{
				if (yuv[i] < 0)yuv[i] = 0;
				if (yuv[i] > 255)yuv[i] = 255;
				pLine[i] = (unsigned char)yuv[i];
			}
		}
	}
}
bool MultiUcharImage::RGB2Y(SingleUcharImage *pOutImageYImage)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	if (pOutImageYImage->GetImageWidth() != nWidth || pOutImageYImage->GetImageHeight() != nHeight)
	{
		if (!pOutImageYImage->CreateImage(nWidth, nHeight))return false;
	}
	int nThread = omp_get_num_procs();
#pragma omp parallel for num_threads(nThread) schedule(dynamic,32)
	for (int y = 0; y < m_nHeight; y++)
	{
		unsigned char *pLine = GetImageLine(y);
		unsigned char *pOutLine = pOutImageYImage->GetImageLine(y);
		int x = 0;
		for (; x < m_nWidth; x++, pLine += 3)
		{
			int  yuv[3], bgr[3];
			for (int i = 0; i < 3; i++)
			{
				bgr[i] = pLine[i];
			}
			yuv[0] = (bgr[2] * 29 + bgr[1] * 150 + bgr[0] * 77 + 128) >> 8;
			if (yuv[0] < 0)yuv[0] = 0;
			if (yuv[0] > 255)yuv[0] = 255;
			pOutLine[0] = (unsigned char)yuv[0];
			pOutLine++;
		}
	}
	return true;
}
bool MultiUcharImage::RGBToHue(MultiShortImage *pOutHueImage)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	if (pOutHueImage->GetImageWidth() != nWidth || pOutHueImage->GetImageHeight() != nHeight)
	{
		if (!pOutHueImage->CreateImage(nWidth, nHeight,1))return false;
	}
	int nThread = omp_get_num_procs();
	#pragma omp parallel for num_threads(nThread) schedule(dynamic,32)
	for (int y = 0; y < m_nHeight; y++)
	{
		unsigned char *pRGBLine = GetImageLine(y);
		short *pOutLine = pOutHueImage->GetImageLine(y);
		int x = 0;
		for (; x < m_nWidth; x++, pRGBLine += 3)
		{
			/*double temprgb[3] = { 0.0,0.0,0.0 };
			temprgb[0] = 0.0 + (0.299 * (double)(int)pRGBLine[0] + 0.587 * (double)(int)pRGBLine[1] + 0.114 * (double)(int)pRGBLine[2]);
			temprgb[1] = 128.0 + (-0.169 * (double)(int)pRGBLine[0] - 0.331 * (double)(int)pRGBLine[1] + 0.499 * (double)(int)pRGBLine[2]);
			temprgb[2] = 128.0 + (0.499 * (double)(int)pRGBLine[0] - 0.418 * (double)(int)pRGBLine[1] - 0.0813 * (double)(int)pRGBLine[2]);
			double tmp = (int )((atan2(temprgb[2] - 128.0, temprgb[1] - 128.0)* 180.0 / PI) + 0.5 + 360.0);
			pOutLine[0] = (int)tmp%360;*/
			float HSV[3]{ 0,0,0 };
			RGBToHSVf(pRGBLine[0], pRGBLine[1], pRGBLine[2], HSV[0], HSV[1], HSV[2]);
			pOutLine[0] = (short)HSV[0];
			pOutLine++;
		}
	}
	return true;
}
bool MultiUcharImage::CopyExtendRect(MultiUcharImage *pInputImage, int nLeft, int nTop, int nRight, int nBottom, int nOutWidth, int nOutHeight)
{
	int y;
	if (!CreateImage(nOutWidth, nOutHeight))return false;
	if (nLeft == 0 && nRight == 0)
	{
		int nWidth = pInputImage->GetImageWidth();
		int nHeight = pInputImage->GetImageHeight();
		int nStart = (nOutHeight - nHeight) / 2;
		int nEnd = nOutHeight - nStart;
		if (nHeight != nOutHeight && nWidth == nOutWidth)
		{
			for (y = 0; y < nStart; y++)
			{
				unsigned char *pOut = GetImageLine(y);
				for (int x = 0; x < nOutWidth; x++)
				{
					pOut[0] = 0;
					pOut[1] = 128;
					pOut[2] = 128;
					pOut += 3;
				}
			}
#pragma omp parallel for
			for (y = nStart; y < nEnd; y++)
			{
				unsigned char *pIn = pInputImage->GetImageLine(y - nStart);
				unsigned char *pOut = GetImageLine(y);
				memcpy(pOut, pIn, nOutWidth * 3);
			}
			for (y = nEnd; y < nOutHeight; y++)
			{
				unsigned char *pOut = GetImageLine(y);
				for (int x = 0; x < nOutWidth; x++)
				{
					pOut[0] = 0;
					pOut[1] = 128;
					pOut[2] = 128;
					pOut += 3;
				}
			}
		}
		else if (nHeight == nOutHeight && nWidth != nOutWidth)
		{
			int nWidth = pInputImage->GetImageWidth();
			int nStart = (nOutWidth - nWidth) / 2;
			int nEnd = nOutWidth - nStart;
			for (y = 0; y < nOutHeight; y++)
			{
				memcpy(GetImageLine(y) + nStart * 3, pInputImage->GetImageLine(y), pInputImage->GetImageWidth() * 3);
			}
			for (y = 0; y < nOutHeight; y++)
			{
				unsigned char *pOut = GetImageLine(y);
				for (int x = 0; x < nOutWidth; x++)
				{
					if (x < nStart || x >= nOutWidth - nStart)
					{
						pOut[0] = 0;
						pOut[1] = 128;
						pOut[2] = 128;
					}
					pOut += 3;
				}
			}
		}
		else
		{
			Clone(pInputImage);
		}
	}
	else
	{
		int nWidth = pInputImage->GetImageWidth();
		int nStart = MAX2(0, nLeft);
		int nEnd = MIN2(nRight, nWidth);
		int SizeM = sizeof(unsigned char) * 3 * (nEnd - nStart);
#pragma omp parallel for
		for (y = nTop; y < nBottom; y++)
		{
			unsigned char *pIn = pInputImage->GetImageLine(y);
			unsigned char *pOut = GetImageLine(y - nTop);
			for (int x = nLeft; x < 0; x++)
			{
				pOut[0] = 0;
				pOut[1] = 128;
				pOut[2] = 128;
				pOut += 3;

			}
			memcpy(pOut, pIn + nStart * 3, SizeM);
			pOut += SizeM;
			for (int x = nWidth; x < nRight; x++)
			{
				pOut[0] = 0;
				pOut[1] = 128;
				pOut[2] = 128;
				pOut += 3;
			}
		}
	}
	return true;
}
bool MultiUcharImage::SaveBGRToBitmapFile(char *pFileName, bool bVFilp)
{
	int i;
	BITMAPFILEHEADER BmpFileHdr;
	BITMAPINFO *pInfo;
	long nBitsSize, nBISize;
	FILE* fp = fopen(pFileName, "wb");
	if (fp == NULL)return false;
	nBISize = sizeof(BITMAPINFOHEADER);
	pInfo = (BITMAPINFO *)malloc(nBISize);
	pInfo->bmiHeader.biBitCount = 24;
	pInfo->bmiHeader.biWidth = m_nWidth;
	pInfo->bmiHeader.biHeight = m_nHeight;
	pInfo->bmiHeader.biCompression = BI_RGB;
	pInfo->bmiHeader.biSizeImage = 0;
	pInfo->bmiHeader.biPlanes = 1;
	pInfo->bmiHeader.biClrImportant = 0;
	pInfo->bmiHeader.biClrUsed = 0;
	pInfo->bmiHeader.biXPelsPerMeter = 75;
	pInfo->bmiHeader.biYPelsPerMeter = 75;
	pInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	int nPitch = (m_nWidth * 24 + 31) / 32 * 4;
	nBitsSize = nPitch * m_nHeight;
	// Write the file header.
	BmpFileHdr.bfType = 0x4D42;		// 'BM'
	BmpFileHdr.bfSize = sizeof(BITMAPFILEHEADER) + nBISize + nBitsSize;
	BmpFileHdr.bfReserved1 = 0;
	BmpFileHdr.bfReserved2 = 0;
	BmpFileHdr.bfOffBits = sizeof(BITMAPFILEHEADER) + nBISize;
	if (fwrite(&BmpFileHdr, 1, sizeof(BITMAPFILEHEADER), fp) != sizeof(BITMAPFILEHEADER))
	{
		fclose(fp);
		free(pInfo);
		return false;
	}
	if (fwrite(pInfo, 1, nBISize, fp) != (unsigned long)nBISize)
	{
		fclose(fp);
		free(pInfo);
		return false;
	}
	unsigned char *pBuffer = new unsigned char[nPitch];
	for (i = 0; i < m_nHeight; i++)
	{
		unsigned char *pScanLine;
		if (bVFilp)
			pScanLine = GetImageLine(m_nHeight - 1 - i);
		else
			pScanLine = GetImageLine(i);
		memcpy(pBuffer, pScanLine, m_nWidth * 3 * sizeof(unsigned char));
		if (fwrite(pBuffer, 1, nPitch, fp) != (unsigned long)nPitch)
		{
			fclose(fp);
			free(pInfo);
			delete[] pBuffer;
			return false;
		}
	}
	delete[] pBuffer;
	fclose(fp);
	free(pInfo);
	return true;
}
bool MultiUcharImage::SaveRGBToBitmapFile(char *pFileName, bool bVFilp)
{
	BITMAPFILEHEADER BmpFileHdr;
	BITMAPINFO *pInfo;
	long nBitsSize, nBISize;
	FILE* fp = fopen(pFileName, "wb");
	if (fp == NULL)return false;
	nBISize = sizeof(BITMAPINFOHEADER);
	pInfo = (BITMAPINFO *)malloc(nBISize);
	pInfo->bmiHeader.biBitCount = 24;
	pInfo->bmiHeader.biWidth = m_nWidth;
	pInfo->bmiHeader.biHeight = m_nHeight;
	pInfo->bmiHeader.biCompression = BI_RGB;
	pInfo->bmiHeader.biSizeImage = 0;
	pInfo->bmiHeader.biPlanes = 1;
	pInfo->bmiHeader.biClrImportant = 0;
	pInfo->bmiHeader.biClrUsed = 0;
	pInfo->bmiHeader.biXPelsPerMeter = 75;
	pInfo->bmiHeader.biYPelsPerMeter = 75;
	pInfo->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	int nPitch = (m_nWidth * 24 + 31) / 32 * 4;
	nBitsSize = nPitch * m_nHeight;
	BmpFileHdr.bfType = 0x4D42;		// 'BM'
	BmpFileHdr.bfSize = sizeof(BITMAPFILEHEADER) + nBISize + nBitsSize;
	BmpFileHdr.bfReserved1 = 0;
	BmpFileHdr.bfReserved2 = 0;
	BmpFileHdr.bfOffBits = sizeof(BITMAPFILEHEADER) + nBISize;
	if (fwrite(&BmpFileHdr, 1, sizeof(BITMAPFILEHEADER), fp) != sizeof(BITMAPFILEHEADER))
	{
		fclose(fp);
		free(pInfo);
		return false;
	}
	if (fwrite(pInfo, 1, nBISize, fp) != (unsigned long)nBISize)
	{
		fclose(fp);
		free(pInfo);
		return false;
	}
	unsigned char *pBuffer = new unsigned char[nPitch];
	for (int y = 0; y < m_nHeight; y++)
	{
		unsigned char *pScanLine;
		if (bVFilp)
			pScanLine = GetImageLine(m_nHeight - 1 - y);
		else
			pScanLine = GetImageLine(y);
		memcpy(pBuffer, pScanLine, m_nWidth * 3 * sizeof(unsigned char));
		for (int x = 0; x < m_nWidth; x++)
		{
			unsigned char R = pBuffer[x * 3 + 0];
			unsigned char B = pBuffer[x * 3 + 2];
			pBuffer[x * 3 + 0] = B;
			pBuffer[x * 3 + 2] = R;
		}
		if (fwrite(pBuffer, 1, nPitch, fp) != (unsigned long)nPitch)
		{
			fclose(fp);
			free(pInfo);
			delete[] pBuffer;
			return false;
		}
	}
	delete[] pBuffer;
	fclose(fp);
	free(pInfo);
	return true;
}
bool MultiUcharImage::LoadBitmapFileToBGR(char *pFileName)
{
	int i;
	long nBitsSize, nBISize;
	BITMAPFILEHEADER BmpFileHdr;
	long nBytes;
	FILE *fp = fopen(pFileName, "rb");
	if (fp == NULL)return false;
	nBytes = fread(&BmpFileHdr, 1, sizeof(BITMAPFILEHEADER), fp);
	if (nBytes != sizeof(BITMAPFILEHEADER))
	{
		fclose(fp);
		return false;
	}
	if (BmpFileHdr.bfType != 0x4D42)
	{
		fclose(fp);
		return false;
	}
	BITMAPINFOHEADER BmpInfoHdr;
	nBytes = fread(&BmpInfoHdr, 1, sizeof(BITMAPINFOHEADER), fp);
	if (nBytes != sizeof(BITMAPINFOHEADER))
	{
		fclose(fp);
		return false;
	}
	if ((BmpInfoHdr.biBitCount != 24) || (BmpInfoHdr.biSize != sizeof(BITMAPINFOHEADER)))
	{
		fclose(fp);
		return false;
	}
	int nPitch = (BmpInfoHdr.biWidth * BmpInfoHdr.biBitCount + 31) / 32 * 4;
	if (BmpInfoHdr.biSizeImage == 0)
		BmpInfoHdr.biSizeImage = nPitch * BmpInfoHdr.biHeight;
	nBISize = sizeof(BITMAPINFOHEADER);
	nBitsSize = BmpFileHdr.bfSize - BmpFileHdr.bfOffBits;
	if (nBitsSize < nPitch * BmpInfoHdr.biHeight)
		nBitsSize = nPitch * BmpInfoHdr.biHeight;
	if (!CreateImage(BmpInfoHdr.biWidth, BmpInfoHdr.biHeight))
	{
		fclose(fp);
		return false;
	}
	fseek(fp, BmpFileHdr.bfOffBits, SEEK_SET);
	unsigned char *pBuffer = new unsigned char[nPitch];
	for (i = 0; i < m_nHeight; i++)
	{
		unsigned char *pLine = GetImageLine(m_nHeight - 1 - i);
		if ((nBytes = fread(pBuffer, 1, nPitch, fp)) != nPitch)
		{
			fclose(fp);
			delete[] pBuffer;
			return false;
		}
		memcpy(pLine, pBuffer, m_nWidth * 3);
	}
	fclose(fp);
	delete[] pBuffer;
	return true;
}
bool MultiUcharImage::LoadBitmapFileToRGB(char *pFileName)
{
	int i;
	long nBitsSize, nBISize;
	BITMAPFILEHEADER BmpFileHdr;
	long nBytes;
	FILE *fp = fopen(pFileName, "rb");
	if (fp == NULL)return false;
	nBytes = fread(&BmpFileHdr, 1, sizeof(BITMAPFILEHEADER), fp);
	if (nBytes != sizeof(BITMAPFILEHEADER))
	{
		fclose(fp);
		return false;
	}
	if (BmpFileHdr.bfType != 0x4D42)
	{
		fclose(fp);
		return false;
	}
	BITMAPINFOHEADER BmpInfoHdr;
	nBytes = fread(&BmpInfoHdr, 1, sizeof(BITMAPINFOHEADER), fp);
	if (nBytes != sizeof(BITMAPINFOHEADER))
	{
		fclose(fp);
		return false;
	}
	if ((BmpInfoHdr.biBitCount != 24) || (BmpInfoHdr.biSize != sizeof(BITMAPINFOHEADER)))
	{
		fclose(fp);
		return false;
	}
	int nPitch = (BmpInfoHdr.biWidth * BmpInfoHdr.biBitCount + 31) / 32 * 4;
	if (BmpInfoHdr.biSizeImage == 0)
		BmpInfoHdr.biSizeImage = nPitch * BmpInfoHdr.biHeight;
	nBISize = sizeof(BITMAPINFOHEADER);
	nBitsSize = BmpFileHdr.bfSize - BmpFileHdr.bfOffBits;
	if (nBitsSize < nPitch * BmpInfoHdr.biHeight)
		nBitsSize = nPitch * BmpInfoHdr.biHeight;
	if (!CreateImage(BmpInfoHdr.biWidth, BmpInfoHdr.biHeight))
	{
		fclose(fp);
		return false;
	}
	fseek(fp, BmpFileHdr.bfOffBits, SEEK_SET);
	unsigned char *pBuffer = new unsigned char[nPitch];
	for (i = 0; i < m_nHeight; i++)
	{
		unsigned char *pLine = GetImageLine(m_nHeight - 1 - i);
		if ((nBytes = fread(pBuffer, 1, nPitch, fp)) != nPitch)
		{
			fclose(fp);
			delete[] pBuffer;
			return false;
		}
		for (int k = 0; k < m_nWidth; k++)
		{
			unsigned char R = pBuffer[k * 3 + 0];
			unsigned char B = pBuffer[k * 3 + 2];
			pBuffer[k * 3 + 0] = B;
			pBuffer[k * 3 + 2] = R;
		}
		memcpy(pLine, pBuffer, m_nWidth * 3);
	}
	fclose(fp);
	delete[] pBuffer;
	return true;
}
bool MultiUcharImage::SaveRGBToJpegFile(char * pFileName,int Quality)
{
	SingleUcharImage Ydata;
	SingleUcharImage udata;
	SingleUcharImage vdata;
	SingleUcharImage jpgdata;
	Ydata.CreateImage(m_nWidth, m_nHeight);
	udata.CreateImage(m_nWidth, m_nHeight);
	vdata.CreateImage(m_nWidth, m_nHeight);
	jpgdata.CreateImage(m_nWidth, m_nHeight);
	Yuv420Image YUVImage;
	MultiUcharImage RGBImage;
	RGBImage.Clone(this);
	RGBImage.RGB2YCbCrOPT();
	YUVImage.YUV444ToYUV420(&RGBImage);
	unsigned long jpegSize = 0;
	YUVImage.GetYImage(&Ydata);
	YUVImage.GetUImage(&udata);
	YUVImage.GetVImage(&vdata);
	YUV2Jpg(Ydata.GetImageData(), udata.GetImageData(), vdata.GetImageData(), m_nWidth, m_nHeight, Quality, jpgdata.GetImageData(), &jpegSize);
	FILE *fp = fopen(pFileName, "wb");
	fwrite(jpgdata.GetImageData(), jpegSize, 1, fp);
	fclose(fp);
	return true;
}
bool MultiUcharImage::SaveBGRToJpegFile(char * pFileName, int Quality)
{
	SingleUcharImage Ydata;
	SingleUcharImage udata;
	SingleUcharImage vdata;
	SingleUcharImage jpgdata;
	Ydata.CreateImage(m_nWidth, m_nHeight);
	udata.CreateImage(m_nWidth, m_nHeight);
	vdata.CreateImage(m_nWidth, m_nHeight);
	jpgdata.CreateImage(m_nWidth, m_nHeight);
	Yuv420Image YUVImage;
	MultiUcharImage RGBImage;
	RGBImage.Clone(this);
	RGBImage.BGR2YCbCrOPT();
	YUVImage.YUV444ToYUV420(&RGBImage);
	unsigned long jpegSize = 0;
	YUVImage.GetYImage(&Ydata);
	YUVImage.GetUImage(&udata);
	YUVImage.GetVImage(&vdata);
	YUV2Jpg(Ydata.GetImageData(), udata.GetImageData(), vdata.GetImageData(), m_nWidth, m_nHeight, Quality, jpgdata.GetImageData(), &jpegSize);
	FILE *fp = fopen(pFileName, "wb");
	fwrite(jpgdata.GetImageData(), jpegSize, 1, fp);
	fclose(fp);
	return true;
}
