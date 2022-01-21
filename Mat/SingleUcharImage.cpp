#include "SingleUcharImage.h"
bool SingleUcharImage::CreateImage(int nWidth, int nHeight)
{
	return SetImageSize(nWidth, nHeight, 1);
}
bool SingleUcharImage::CreateImageFillValue(int nWidth, int nHeight, int nValue)
{
	if (!SetImageSize(nWidth, nHeight, 1))return false;
	FillValue(nValue);
	return true;
}
bool SingleUcharImage::CreateImageWithData(int nWidth, int nHeight, unsigned char *pInputData)
{
	if (!SetImageSize(nWidth, nHeight, 1))return false;
	memcpy(GetImageData(), pInputData, nWidth*nHeight);
	return true;
}
bool SingleUcharImage::Clone(SingleUcharImage *pInputImage)
{
	int nWidth = pInputImage->GetImageWidth();
	int nHeight = pInputImage->GetImageHeight();
	if (!CreateImage(nWidth, nHeight))return false;
	memcpy(m_pImgData, pInputImage->GetImageData(), nWidth*nHeight);
	return true;
}
bool SingleUcharImage::GetEachBlockAverageValue(SingleUcharImage *pOutImage,int nRadius)
{
	int Width = GetImageWidth();
	int Height = GetImageHeight();
	int Stride = GetImageWidth();
	if (!pOutImage->CreateImage(Width, Height))return false;
	unsigned int *Integral = MergerBufnew<unsigned int>((Width + 1) * (Height + 1));
	GetImageIntegralData(GetImageData(), Integral, Width, Height, Stride);
	int nProcs = omp_get_num_procs();
	#pragma omp parallel for  num_threads(nProcs) schedule(dynamic,16)  
	for (int Y = 0; Y < Height; Y++)
	{
		int Y1 = max(Y - nRadius, 0);
		int Y2 = min(Y + nRadius + 1, Height - 1);
		unsigned int *LineP1 = Integral + Y1 * (Width + 1);
		unsigned int *LineP2 = Integral + Y2 * (Width + 1);
		unsigned char *LinePD = pOutImage->GetImageLine(Y);
		for (int X = 0; X < Width; X++)
		{
			int X1 = max(X - nRadius, 0);
			int X2 = min(X + nRadius + 1, Width);
			unsigned int Sum = LineP2[X2] - LineP1[X2] - LineP2[X1] + LineP1[X1];
			unsigned int PixelCount = (X2 - X1) * (Y2 - Y1);
			LinePD[X] = (Sum + (PixelCount >> 1)) / PixelCount;
		}
	}
	MergerBufdelete<unsigned int>(Integral);
	return true;
}
bool SingleUcharImage::GetHistogram(int nHist[])
{
	int nProcs = omp_get_num_procs();
	int *pBuffers = new int[256 * nProcs];
	memset(pBuffers, 0, 256 * nProcs * sizeof(int));
	for (int g = 0; g < 256; g++)
	{
		nHist[g] = 0;
	}
#pragma omp parallel for num_threads(nProcs)
	for (int y = 0; y < m_nHeight; y++)
	{
		int nThreadId = omp_get_thread_num();
		unsigned char *pY0 = m_pImgData + y * m_nWidth;
		int *pB = pBuffers + 256 * nThreadId;
		for (int x = 0; x < m_nWidth; x++)
		{
			int g = *(pY0 + x);
			pB[g]++;
		}
	}
	for (int k = 0; k < nProcs; k++)
	{
		int *pB = pBuffers + 256 * k;
		for (int i = 0; i < 256; i++)
		{
			nHist[i] += pB[i];
		}
	}
	delete[]pBuffers;
	return true;
}
bool SingleUcharImage::GetRectHistogram(unsigned int nHist[],unsigned char maxvalue, int nLeft, int nTop, int nRight, int nBottom)
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
		unsigned char *pY = GetImageLine(y) + nLeft;
		for (int x = nLeft; x < nRight; x++)
		{
			unsigned char g = *(pY++);
			nHist[g]++;
		}
	}
	return true;
}
void SingleUcharImage::HistMapping(int map[256], SingleUcharImage *pOutImage)
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
				dstLine[x] = (unsigned char)map[srcLine[x]];
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
				dstLine[x] = (unsigned char)map[srcLine[x]];
			}
		}
	}
}
bool SingleUcharImage::GetIntegralHistogram(CImageData_UINT32  *pOutIntegralHist)
{
	int nHeight = GetImageHeight();
	int nWidth = GetImageWidth();
	if (pOutIntegralHist->GetImageWidth() != nWidth || pOutIntegralHist->GetImageHeight() != nHeight)
	{
		if (!pOutIntegralHist->SetImageSize(nWidth, nHeight, 256))return false;
	}
	CImageData_UINT16 IntegralImage;
	IntegralImage.SetImageSize(nWidth, nHeight,1);
	unsigned short *pIntegralliney0 = IntegralImage.GetImageLine(0);
	pIntegralliney0[0] =(unsigned short)this[0][0];
	for (int y = 1; y < nHeight; ++y)
	{
		unsigned short *pIntegralline0 = IntegralImage.GetImageLine(y - 1);
		unsigned short *pIntegralline1 = IntegralImage.GetImageLine(y);
		pIntegralline1[0] = pIntegralline0[0];
	}
	for (int x = 1; x < nWidth; ++x)
	{
		pIntegralliney0[x] = pIntegralliney0[x - 1];
	}
	for (int y = 1; y < nHeight; ++y)
	{
		unsigned char *pInline = GetImageLine(y);
		unsigned short *pIntegralline0 = IntegralImage.GetImageLine(y-1);
		unsigned short *pIntegralline1 = IntegralImage.GetImageLine(y);
		unsigned int *IntegralHistline0 = pOutIntegralHist->GetImageLine(y - 1);
		unsigned int *IntegralHistline1 = pOutIntegralHist->GetImageLine(y);
		for (int x = 1; x < nWidth; ++x)
		{
			pIntegralline1[x] = pIntegralline0[x] + pIntegralline1[x - 1] - pIntegralline0[x - 1] + pInline[x];
			int index0 = (x - 1) * 256;
			int index1 = index0+256;
			for (int k = 0; k < 256; ++k)
			{
				IntegralHistline1[index1 + k] = IntegralHistline0[index1 + k] + IntegralHistline1[index0 + k] - IntegralHistline0[index0 + k];
			}
			++IntegralHistline1[index1 + pIntegralline1[x]];
		}
	}
	return true;
}
bool SingleUcharImage::ClearImageFollowRectOutside(int nLeft, int nTop, int  nRight, int nBottom)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int dim = GetImageDim();
	if (nRight >= nWidth || nBottom >= nHeight)
	{
		return false;
	}
	#pragma omp parallel for 
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
				pMainline[0] = 0;
			}
			pMainline++;
		}
	}
	return true;
}
float SingleUcharImage::GetMaskValueRatio(unsigned char cutlabelvalue[],int len)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int labelnum = 0;
	float ratio = 0;
	#pragma omp parallel for reduction(+:labelnum)
	for (int y = 0; y < nHeight; y++)
	{
		unsigned char *pMainline = GetImageLine(y);
		for (int x = 0; x < nWidth; x++)
		{
			for (int k=0;k<len;k++)
			{
				if (pMainline[0] == cutlabelvalue[k])
				{
					labelnum++;
				}
			}
			pMainline++;
		}
	}
	ratio = (float)labelnum / (float)(nHeight*nWidth);
	return ratio;
}
bool SingleUcharImage::FillImageRect(int nLeft, int nTop, int nRight, int nBottom,unsigned char nValue)
{
	int linelen = nRight - nLeft;
	for (int y = nTop; y < nBottom; y++)
	{
		unsigned char *pOut = GetImageLine(y);
		memset(pOut+ nLeft, nValue, linelen);
	}
	return true;
}
bool SingleUcharImage::SobelImage(SingleUcharImage *pOutImage)
{
	/*
gx=	-1 0 +1   gy= +1 +2 +1
	-2 0 +2	       0  0  0
	-1 0 +1		  -1 -2 -1
	*/
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	if (pOutImage->GetImageWidth() != nWidth || pOutImage->GetImageHeight() != nHeight)
	{
		if (!pOutImage->CreateImageFillValue(nWidth, nHeight,0))return false;
	}
	#pragma omp parallel for
	for (int y = 1; y < nHeight - 1; y++)
	{
		unsigned char *pSline0 = GetImageLine(y - 1);
		unsigned char *pSline1 = GetImageLine(y);
		unsigned char *pSline2 = GetImageLine(y + 1);
		unsigned char *pDline = pOutImage->GetImageLine(y);
		for (int x = 1; x < nWidth - 1; x++)
		{
			int gx = pSline0[2]- pSline0[0] + (pSline1[2]-pSline1[0]) * 2 +pSline2[2] - pSline2[0];
			int gy = pSline0[2]+ pSline0[0] + (pSline0[1]-pSline2[1]) * 2 - pSline2[2] - pSline2[0];
			unsigned short sum = sqrt(gx*gx + gy * gy);
			if (sum > 255)sum = 255;
			pDline[1] = sum;
			pDline++;
			pSline0++;
			pSline1++;
			pSline2++;
		}
	}
	return true;
}
bool SingleUcharImage::PrewittImage(SingleUcharImage *pOutImage)
{
	/*
gx=	-1 0 +1   gy= +1 +1 +1
	-1 0 +1	       0  0  0
	-1 0 +1		  -1 -1 -1
	*/
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	if (pOutImage->GetImageWidth() != nWidth || pOutImage->GetImageHeight() != nHeight)
	{
		if (!pOutImage->CreateImageFillValue(nWidth, nHeight, 0))return false;
	}
#pragma omp parallel for
	for (int y = 1; y < nHeight - 1; y++)
	{
		unsigned char *pSline0 = GetImageLine(y - 1);
		unsigned char *pSline1 = GetImageLine(y);
		unsigned char *pSline2 = GetImageLine(y + 1);
		unsigned char *pDline = pOutImage->GetImageLine(y);
		for (int x = 1; x < nWidth - 1; x++)
		{
			short gx = pSline0[2] - pSline0[0] + (pSline1[2] - pSline1[0]) + pSline2[2] - pSline2[0];
			short gy = pSline0[2] + pSline0[0] + (pSline0[1] - pSline2[1])  - pSline2[2] - pSline2[0];
			unsigned short sum = sqrt(gx*gx + gy * gy);
			if (sum > 255)sum = 255;
			pDline[1] = sum;
			pDline++;
			pSline0++;
			pSline1++;
			pSline2++;
		}
	}
	return true;
}
bool SingleUcharImage::LaplacianImage(SingleUcharImage *pOutImage)
{
	/*
kernel1=	0  1  0   kernel2	=2  0  2
			1 -4  1				 0 -8  0
			0  1  0				 2  0  2
	*/
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	if (pOutImage->GetImageWidth() != nWidth || pOutImage->GetImageHeight() != nHeight)
	{
		if (!pOutImage->CreateImageFillValue(nWidth, nHeight, 0))return false;
	}
	#pragma omp parallel for
	for (int y = 1; y < nHeight - 1; y++)
	{
		unsigned char *pSline0 = GetImageLine(y - 1);
		unsigned char *pSline1 = GetImageLine(y);
		unsigned char *pSline2 = GetImageLine(y + 1);
		unsigned char *pDline = pOutImage->GetImageLine(y);
		for (int x = 1; x < nWidth - 1; x++)
		{
			short sum = pSline0[1] + pSline1[0] + pSline1[2] + pSline2[1] - pSline1[1]*4 ;
			pDline[1] = (unsigned char)CLIP(sum, 0, 255);
			pDline++;
			pSline0++;
			pSline1++;
			pSline2++;
		}
	}
	return true;
}
bool SingleUcharImage::ThresholdImage(SingleUcharImage * pOutImage, int thresh, int maxval, int type)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	if (pOutImage->GetImageWidth() != nWidth || pOutImage->GetImageHeight() != nHeight)
	{
		if (!pOutImage->Clone(this))return false;
	}
	#pragma omp parallel for
	for (int y = 0; y < nHeight; y++)
	{
		unsigned char *pSline = GetImageLine(y);
		unsigned char *pDline = pOutImage->GetImageLine(y);
		for (int x = 0; x < nWidth; x++)
		{
			switch (type)
			{
			case 0:
				if (pSline[0] > thresh)
				{
					pDline[0] = maxval;
				}
				else
				{
					pDline[0] = 0;
				}
				break;
			case 1:
				if (pSline[0] > thresh)
				{
					pDline[0] = 0;
				}
				else
				{
					pDline[0] = maxval;
				}
				break;
			case 2:
				if (pSline[0] > thresh)
				{
					pDline[0] = thresh;
				}
				break;
			case 3:
				if (pSline[0] < thresh)
				{
					pDline[0] = thresh;
				}
				break;
			case 4:
				if (pSline[0] > thresh)
				{
					pDline[0] = 0;
				}
				break;
			default:
				break;
			}
			pDline++;
			pSline++;
		}
	}
	return true;
}
bool SingleUcharImage::Dilation3x3SquareImage(SingleUcharImage *pOutImage)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	unsigned char *pLines[3];
	if (pOutImage->GetImageWidth() != nWidth || pOutImage->GetImageHeight() != nHeight)
	{
		if (!pOutImage->CreateImage(nWidth, nHeight))return false;
	}
	int nThread = omp_get_num_procs();
	unsigned char *pBuffer = new unsigned char[nWidth * 3 * nThread];
	if (pBuffer == NULL)return false;
	int loop = 0;
	#pragma omp parallel for num_threads(nThread) firstprivate(loop) private(pLines) 
	for (int y = 0; y < nHeight; y++)
	{
		int nThreadId = omp_get_thread_num();
		if (loop == 0)
		{
			pLines[0] = pBuffer + nWidth * 3 * nThreadId;
			pLines[1] = pLines[0] + 1;
			pLines[2] = pLines[1] + 1;
			FillLine(pLines[0], (unsigned char)0, nWidth, 3);
			HDilation3Line(GetImageLine(y), pLines[1], nWidth);
			loop++;
		}
		HDilation3Line(GetImageLine(y + 1), pLines[2], nWidth);
		VDilation3Line(pBuffer + nWidth * 3 * nThreadId, pOutImage->GetImageLine(y), nWidth);
		unsigned char *pTemp = pLines[0];
		pLines[0] = pLines[1];
		pLines[1] = pLines[2];
		pLines[2] = pTemp;
		if (y == nHeight - 1)
		{
			FillLine(pLines[2], (unsigned char)0, nWidth, 3);
			VDilation3Line(pBuffer + nWidth * 3 * nThreadId, pOutImage->GetImageLine(nHeight - 1), nWidth);
		}
	}
	delete[] pBuffer;
	return true;
}
bool SingleUcharImage::Dilation3x3DiamondImage(SingleUcharImage *pOutImage)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	unsigned char *pLines[3];
	if (pOutImage->GetImageWidth() != nWidth || pOutImage->GetImageHeight() != nHeight)
	{
		if (!pOutImage->CreateImage(nWidth, nHeight))return false;
	}
	int nThread = omp_get_num_procs();
	unsigned char *pBuffer = new unsigned char[nWidth * 3 * nThread];
	if (pBuffer == NULL)return false;
	int loop = 0;
#pragma omp parallel for num_threads(nThread) firstprivate(loop) private(pLines) 
	for (int y = 0; y < nHeight; y++)
	{
		int nThreadId = omp_get_thread_num();
		if (loop == 0)
		{
			pLines[0] = pBuffer + nWidth * 3 * nThreadId;
			pLines[1] = pLines[0] + 1;
			pLines[2] = pLines[1] + 1;
			FillLine(pLines[0], (unsigned char)0, nWidth, 3);
			RotateLine(GetImageLine(y), pLines[1], nWidth, 3);
			loop++;
		}
		RotateLine(GetImageLine(y + 1), pLines[2], nWidth, 3);
		Dilation3x3Line(pLines, pOutImage->GetImageLine(y), nWidth);
		unsigned char *pTemp = pLines[0];
		pLines[0] = pLines[1];
		pLines[1] = pLines[2];
		pLines[2] = pTemp;
		if (y == nHeight - 1)
		{
			FillLine(pLines[2], (unsigned char)0, nWidth, 3);
			Dilation3x3Line(pLines, pOutImage->GetImageLine(nHeight - 1), nWidth);
		}
	}
	delete[] pBuffer;
	return true;
}
bool SingleUcharImage::Dilation5x5Image(SingleUcharImage *pOutImage)
{
	/*
	Mask
	0 1 1 1 0
	1 1 1 1 1
	1 1 1 1 1
	1 1 1 1 1
	0 1 1 1 0
	*/
	SingleUcharImage TmpOutImage;
	if (!Dilation3x3SquareImage(&TmpOutImage))return false;
	if (!TmpOutImage.Dilation3x3DiamondImage(pOutImage))return false;
	return true;
}
bool SingleUcharImage::Dilation7x7Image(SingleUcharImage *pOutImage)
{
	/*
	Mask
	0 0 1 1 1 0 0
	0 1 1 1 1 1 0
	1 1 1 1 1 1 1
	1 1 1 1 1 1 1
	1 1 1 1 1 1 1
	0 1 1 1 1 1 0
	0 0 1 1 1 0 0
	*/
	SingleUcharImage TmpOutImage;
	SingleUcharImage TmpOutImage1;
	if (!Dilation3x3SquareImage(&TmpOutImage))return false;
	if (!TmpOutImage.Dilation3x3DiamondImage(&TmpOutImage1))return false;
	if (!TmpOutImage1.Dilation3x3DiamondImage(pOutImage))return false;
	return true;
}
bool SingleUcharImage::Dilation9x9Image(SingleUcharImage *pOutImage)
{
	/*
	Mask
	0 0 1 1 1 1 1 0 0
	0 1 1 1 1 1 1 1 0
	1 1 1 1 1 1 1 1 1
	1 1 1 1 1 1 1 1 1
	1 1 1 1 1 1 1 1 1
	1 1 1 1 1 1 1 1 1
	1 1 1 1 1 1 1 1 1
	0 1 1 1 1 1 1 1 0
	0 0 1 1 1 1 1 0 0
	*/
	SingleUcharImage TmpOutImage;
	SingleUcharImage TmpOutImage1;
	if (!Dilation3x3SquareImage(&TmpOutImage))return false;
	if (!TmpOutImage.Dilation3x3SquareImage(&TmpOutImage1))return false;
	if (!TmpOutImage1.Dilation3x3DiamondImage(&TmpOutImage))return false;
	if (!TmpOutImage.Dilation3x3DiamondImage(pOutImage))return false;
	return true;
}
bool SingleUcharImage::Erosion3x3Square(SingleUcharImage *pOutImage)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	if (pOutImage->GetImageWidth() != nWidth || pOutImage->GetImageHeight() != nHeight)
	{
		if (!pOutImage->CreateImage(nWidth, nHeight))return false;
	}
	unsigned char *pVErosionLine[2];
	int nThread = omp_get_num_procs();
	unsigned char *pBuffer = new unsigned char[((nWidth + 2) * 2)*nThread];
	unsigned char *pLine255= new unsigned char[(nWidth + 2)];
	memset(pLine255, 255, nWidth);
	int loop = 0;
#pragma omp parallel for num_threads(nThread) firstprivate(loop) private(pVErosionLine)
	for (int y = 0; y < nHeight; y++)
	{
		unsigned char *pVLines[3];
		if (loop == 0)
		{
			int nThreadId = omp_get_thread_num();
			pVErosionLine[0] = pBuffer + 1+ (nWidth + 2) * 2* nThreadId;
			pVErosionLine[1] = pVErosionLine[0] + (nWidth + 2);
			pVLines[0] = (y == 0 ? pLine255 : GetImageLine(y - 1));
			pVLines[1] = GetImageLine(y);
			pVLines[2] = GetImageLine(y+1);
			VErosion3Line(pVLines, pVErosionLine[0], nWidth);
			pVErosionLine[0][-1] = pVErosionLine[0][nWidth] = 255;
			loop++;
		}
		pVLines[0] = (y == 0 ? pLine255 : GetImageLine(y - 1));
		pVLines[1] = GetImageLine(y);
		pVLines[2] = (y == nHeight - 1 ? pLine255 : GetImageLine(y + 1));
		VErosion3Line(pVLines, pVErosionLine[1], nWidth);
		pVErosionLine[1][-1] = pVErosionLine[1][nWidth] = 255;
		HErosion3Line(pVErosionLine[0], pOutImage->GetImageLine(y-1), nWidth);
		unsigned char *pTemp = pVErosionLine[0];
		pVErosionLine[0] = pVErosionLine[1];
		pVErosionLine[1] = pTemp;
	}
	delete[] pLine255;
	delete[] pBuffer;
	return true;
}
bool SingleUcharImage::Erosion3x3Diamond(SingleUcharImage *pOutImage)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	if (pOutImage->GetImageWidth() != nWidth || pOutImage->GetImageHeight() != nHeight)
	{
		if (!pOutImage->CreateImage(nWidth, nHeight))return false;
	}
	unsigned char *pLines[3];
	int nThread = omp_get_num_procs();
	unsigned char *pBuffer = new unsigned char[(nWidth + 2) * 3 * nThread];
	int loop = 0;
	#pragma omp parallel for num_threads(nThread) firstprivate(loop) private(pLines) 
	for (int y = 0; y < nHeight; y++)
	{
		int nThreadId = omp_get_thread_num();
		if (loop == 0)
		{
			for (int i = 0; i < 3; i++)
			{
				pLines[i] = pBuffer + 1 + (nWidth + 2)*(i+3* nThreadId);
			}
			memset(pLines[0] - 1, 255, nWidth + 2);
			memcpy(pLines[1], GetImageLine(y), nWidth);
			pLines[1][-1] = pLines[1][nWidth] = 255;
			loop++;
		}
		if (y == nHeight - 1)
		{
			memset(pLines[2], 255, nWidth);
		}
		else
		{
			memcpy(pLines[2], GetImageLine(y + 1), nWidth);
		}
		pLines[2][-1] = pLines[2][nWidth] = 255;
		Erosion3x3Line(pLines, pOutImage->GetImageLine(y), nWidth);
		unsigned char *pTemp = pLines[0];
		pLines[0] = pLines[1];
		pLines[1] = pLines[2];
		pLines[2] = pTemp;
	}
	delete[] pBuffer;
	return true;
}
bool SingleUcharImage::Erosion5x5Image(SingleUcharImage *pOutImage)
{
	/*
	Mask
	0 1 1 1 0
	1 1 1 1 1
	1 1 1 1 1
	1 1 1 1 1
	0 1 1 1 0
	*/
	SingleUcharImage TmpOutImage;
	if (!Erosion3x3Square(&TmpOutImage))return false;
	if (!TmpOutImage.Erosion3x3Diamond(pOutImage))return false;
	return true;
}
bool SingleUcharImage::Erosion7x7Image(SingleUcharImage *pOutImage)
{
	/*
	Mask
	0 0 1 1 1 0 0
	0 1 1 1 1 1 0
	1 1 1 1 1 1 1
	1 1 1 1 1 1 1
	1 1 1 1 1 1 1
	0 1 1 1 1 1 0
	0 0 1 1 1 0 0
	*/
	SingleUcharImage TmpOutImage;
	SingleUcharImage TmpOutImage1;
	if (!Erosion3x3Square(&TmpOutImage))return false;
	if (!TmpOutImage.Erosion3x3Diamond(&TmpOutImage1))return false;
	if (!TmpOutImage1.Erosion3x3Diamond(pOutImage))return false;
	return true;
}
bool SingleUcharImage::Erosion9x9Image(SingleUcharImage *pOutImage)
{
	/*
	Mask
	0 0 1 1 1 1 1 0 0
	0 1 1 1 1 1 1 1 0
	1 1 1 1 1 1 1 1 1
	1 1 1 1 1 1 1 1 1
	1 1 1 1 1 1 1 1 1
	1 1 1 1 1 1 1 1 1
	1 1 1 1 1 1 1 1 1
	0 1 1 1 1 1 1 1 0
	0 0 1 1 1 1 1 0 0
	*/
	SingleUcharImage TmpOutImage;
	SingleUcharImage TmpOutImage1;
	if (!Erosion3x3Square(&TmpOutImage))return false;
	if (!TmpOutImage.Erosion3x3Square(&TmpOutImage1))return false;
	if (!TmpOutImage1.Erosion3x3Diamond(&TmpOutImage))return false;
	if (!TmpOutImage.Erosion3x3Diamond(pOutImage))return false;
	return true;
}
bool SingleUcharImage::BoxFilterImage(SingleUcharImage *pOutImage, int nRadius)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	int Stride = GetImageWidth();
	if (pOutImage->GetImageWidth() != nWidth || pOutImage->GetImageHeight() != nHeight)
	{
		if (!pOutImage->CreateImage(nWidth, nHeight))return false;
	}
	unsigned int *Integral = MergerBufnew<unsigned int>((nWidth + 1) * (nHeight + 1));
	GetImageIntegralData(GetImageData(), Integral, nWidth, nHeight, Stride);
	int nProcs = omp_get_num_procs();
	#pragma omp parallel for  num_threads(nProcs) 
	for (int Y = 0; Y < nHeight; Y++)
	{
		int Y1 = max(Y - nRadius, 0);
		int Y2 = min(Y + nRadius + 1, nHeight - 1);
		unsigned int *LineP1 = Integral + Y1 * (nWidth + 1);
		unsigned int *LineP2 = Integral + Y2 * (nWidth + 1);
		unsigned char *LinePD = pOutImage->GetImageLine(Y);
		for (int X = 0; X < nWidth; X++)
		{
			int X1 = max(X - nRadius, 0);
			int X2 = min(X + nRadius + 1, nWidth);
			unsigned int Sum = LineP2[X2] - LineP1[X2] - LineP2[X1] + LineP1[X1];
			unsigned int PixelCount = (X2 - X1) * (Y2 - Y1);
			LinePD[X] = (Sum + (PixelCount >> 1)) / PixelCount;
		}
	}
	MergerBufdelete<unsigned int>(Integral);
	return true;
}
bool SingleUcharImage::GaussianBlur3x3Image(SingleUcharImage * pOutImage)
{
	/*
	kernel
	1 2 1
	2 4 2
	1 2 1
	*/
	bool bDitheringEnable;
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	if (pOutImage->GetImageWidth() != nWidth || pOutImage->GetImageHeight() != nHeight)
	{
		if (!pOutImage->CreateImage(nWidth, nHeight))return false;
	}
	int nProcs = omp_get_num_procs();
	unsigned short *pBuffer = new unsigned short[(nWidth * 3)*nProcs];
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
				pLines[i] = pBuffer + (nThreadId * 3 + i)*nWidth;
			}
			for (i = -1; i < 1; i++)
			{
				HGaussianLine3(GetImageLine(y + i), pLines[i + 1], nWidth, 1);
			}
			loop++;
		}
		unsigned char *pInLine = GetImageLine(y + 1);
		unsigned char  *pOutLine = pOutImage->GetImageLine(y);
		HGaussianLine3(pInLine, pLines[2], nWidth, 1);
		VGaussianLine3(pLines, pOutLine, nWidth, 1);
		unsigned short *p = pLines[0];
		for (i = 0; i < 2; i++)
		{
			pLines[i] = pLines[i + 1];
		}
		pLines[2] = p;
	}
	delete[]pBuffer;
	return true;
}
bool SingleUcharImage::GaussianBlur5x5Image(SingleUcharImage * pOutImage)
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
	if (pOutImage->GetImageWidth() != nWidth || pOutImage->GetImageHeight() != nHeight)
	{
		if (!pOutImage->CreateImage(nWidth, nHeight))return false;
	}
	int nProcs = omp_get_num_procs();
	unsigned short *pBuffer = new unsigned short[(nWidth * 5)*nProcs];
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
				pLines[i] = pBuffer + (nThreadId * 5 + i)*nWidth;
			}
			for (i = -2; i < 2; i++)
			{
				HGaussianLine5(GetImageLine(y + i), pLines[i + 2], nWidth, 1);
			}
			loop++;
		}
		unsigned char *pInLine = GetImageLine(y + 2);
		unsigned char  *pOutLine = pOutImage->GetImageLine(y);
		HGaussianLine5(pInLine, pLines[4], nWidth, 1);
		VGaussianLine5(pLines, pOutLine, nWidth, 1);
		unsigned short *p = pLines[0];
		for (i = 0; i < 4; i++)
		{
			pLines[i] = pLines[i + 1];
		}
		pLines[4] = p;
	}
	delete[]pBuffer;
	return true;
}
bool SingleUcharImage::Bilateral5x5Image(SingleUcharImage *pOutImage,int nThre)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
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
		int i, j, x, Y0, Y;
		unsigned char *pIn = GetImageLine(y);
		unsigned char *pOut = pOutImage->GetImageLine(y);
		for (x = 0; x < nWidth; x++)
		{
			long long sumY, dY;
			int W, sumW;
			Y0 = *pIn;
			sumY = 0;
			sumW = 0;
			for (i = -2; i <= 2; i++)
			{
				if (y + i < 0 || y + i >= nHeight)continue;
				for (j = -2; j <= 2; j++)
				{
					if (x + j < 0 || x + j >= nWidth)continue;
					Y = pIn[i*nWidth + j];
					dY = Y - Y0;
					dY *= dY;
					W = (int)(8 - ((dY*nInvNoise) >> 28));
					if (W >= 0)
					{
						W = nMask[i + 2][j + 2] << W;
						sumY += Y * W;
						sumW += W;
					}
				}
			}
			*(pOut++) = (int)(sumY / sumW);
			pIn++;
		}
	}
	return true;
}
bool SingleUcharImage::Extend2Image(int nS)//2µÄx´Î·½
{
	SingleUcharImage tmpOutImage;
	if (nS < 0)return false;
	int nMask = (1 << nS) - 1;
	int nInWidth = GetImageWidth();
	int nInHeight = GetImageHeight();
	int nOutWidth = nInWidth + (((1 << nS) - (nInWidth&nMask))&nMask);
	int nOutHeight = nInHeight + (((1 << nS) - (nInHeight&nMask))&nMask);
	if (tmpOutImage.GetImageWidth() != nOutWidth || tmpOutImage.GetImageHeight() != nOutHeight)
	{
		if (!tmpOutImage.CreateImage(nOutWidth, nOutHeight))return false;
	}
	FillUcharData(GetImageData(), tmpOutImage.GetImageData(), nInWidth, nInHeight, nOutWidth, nOutHeight, 1);
	this->Clone(&tmpOutImage);
	return true;
}
bool SingleUcharImage::ExtendRightAndBottomImage(int nOutWidth,int nOutHeight)
{
	SingleUcharImage tmpOutImage;
	int nInWidth = GetImageWidth();
	int nInHeight = GetImageHeight();
	if (tmpOutImage.GetImageWidth() != nOutWidth || tmpOutImage.GetImageHeight() != nOutHeight)
	{
		if (!tmpOutImage.CreateImage(nOutWidth, nOutHeight))return false;
	}
	FillUcharData(GetImageData(), tmpOutImage.GetImageData(), nInWidth, nInHeight, nOutWidth, nOutHeight, 1);
	this->Clone(&tmpOutImage);
	return true;
}

bool SingleUcharImage::SubtractEdgeImage(SingleUcharImage *pInImage, MultiShortImage *pOutImage)
{
	int nWidth0 = GetImageWidth();
	int nHeight0 = GetImageHeight();
	int nWidth1 = pInImage->GetImageWidth();
	int nHeight1 = pInImage->GetImageHeight();
	if (nWidth1 < nWidth0 || nHeight1 < nHeight0)return false;
	if (!pOutImage->CreateImage(nWidth0, nHeight0, 1))return false;
	SubtractUcharEdgeData(GetImageData(), pInImage->GetImageData(), pOutImage->GetImageData(), nWidth0, nHeight0, nWidth1, nHeight1, 1);
	return true;
}
bool SingleUcharImage::DownScaleImagex2(SingleUcharImage *pOutImage, bool bDitheringEnable = false)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	SingleUcharImage TempImage;
	if ((nWidth & 1) == 1 || (nHeight & 1) == 1)
	{
		Extend2Image(1);
		nWidth = GetImageWidth();
		nHeight = GetImageHeight();
	}
	if (!pOutImage->CreateImage(nWidth >> 1, nHeight >> 1))return false;
	return DownScaleUcharDatax2(GetImageData(), pOutImage->GetImageData(), nWidth, nHeight, 1, bDitheringEnable);	
}
bool SingleUcharImage::DownScaleImageSamplex2(SingleUcharImage *pOutImage)
{
	int nInWidth = GetImageWidth();
	int nInHeight = GetImageHeight();
	int nOutWidth = (nInWidth + (nInWidth & 1)) >> 1;
	int nOutHeight = (nInHeight + (nInHeight & 1)) >> 1;
	if (pOutImage->GetImageWidth() != nOutWidth || pOutImage->GetImageHeight() != nOutHeight)
	{
		if (!pOutImage->CreateImage(nOutWidth, nOutHeight))return false;
	}
	#pragma omp parallel for schedule(dynamic,16)
	for (int y = 0; y < nInHeight / 2; y++)
	{
		unsigned char *pIn[2];
		unsigned char *pOut = pOutImage->GetImageLine(y);
		pIn[0] = GetImageLine(y * 2);
		pIn[1] = GetImageLine(y * 2 + 1);
		int x;
		unsigned int Y;
		x = 0;
		for (; x < (nInWidth / 2) - 7; x += 8)
		{
			const uint8x16_t vIn0 = vld1q_u8(pIn[0]);
			const uint8x16_t vIn1 = vld1q_u8(pIn[1]);
			pIn[0] += 2 * 8;
			pIn[1] += 2 * 8;
			uint16x8_t vY = vpaddlq_u8(vIn0);
			vY = vaddq_u16(vY, vpaddlq_u8(vIn1));
			const uint8x8_t vOut = vshrn_n_u16(vY, 2);
			vst1_u8(pOut, vOut);
			pOut += 8;
		}
		for (; x < nInWidth / 2; x++)
		{
			Y = 0;
			for (int i = 0; i < 2; i++)
			{
				for (int j = 0; j < 2; j++)
				{
					Y += pIn[i][j];
				}
				pIn[i] += 2;
			}
			Y >>= 2;
			*(pOut++) = (unsigned char)Y;
		}
		for (; x < nOutWidth; x++)
		{
			Y = 0;
			for (int i = 0; i < 2; i++)
			{
				Y += pIn[i][0];
			}
			Y >>= 1;
			*(pOut++) = (unsigned char)Y;
		}
	}
	if ((nInHeight / 2) < nOutHeight)
	{
		int x;
		unsigned int Y;
		int y = nOutHeight - 1;
		unsigned char *pIn[2];
		unsigned char *pOut = pOutImage->GetImageLine(y);
		pIn[0] = GetImageLine(y * 2);
		for (x = 0; x < nInWidth / 2; x++)
		{
			Y = (pIn[0][0] + pIn[0][1]);
			pIn[0] += 2;

			Y /= 2;
			*(pOut++) = (unsigned char)Y;
		}
		for (; x < nOutWidth; x++)
		{
			Y = pIn[0][0];
			*(pOut++) = (unsigned char)Y;
		}
	}
	return true;
}
bool SingleUcharImage::UpScaleImagex2(SingleUcharImage *pOutImage, bool bDitheringEnable = false)
{
	int nWidth =GetImageWidth();
	int nHeight = GetImageHeight();
	if (!pOutImage->CreateImage(nWidth * 2, nHeight * 2))return false;
	return UpScaleUcharDatax2(GetImageData(), pOutImage->GetImageData(), nWidth, nHeight, 1, bDitheringEnable);
}
bool SingleUcharImage::AddBackEdgeImage(SingleUcharImage *pInputImage, MultiShortImage *pInputEdgeImage)
{
	int nWidth = pInputImage->GetImageWidth();
	int nHeight = pInputImage->GetImageHeight();
	int nEdgeWidth = pInputEdgeImage->GetImageWidth();
	int nEdgeHeight = pInputEdgeImage->GetImageHeight();
	if (nEdgeWidth > nWidth || nEdgeHeight > nHeight || pInputEdgeImage->GetImageDim() != 1)return false;
	if (!this->CreateImage(nEdgeWidth, nEdgeHeight))return false;
	AddBackUcharEdge(pInputImage->GetImageData(), pInputEdgeImage->GetImageData(), GetImageData(), nWidth, nHeight, nEdgeWidth, nEdgeHeight, 1);
	return true;
}
bool SingleUcharImage::GaussPyramidImage(SingleUcharImage *pOutPyramid,MultiShortImage *pOutEdgePyramid,int &nPyramidLevel)
{
	int nWidth[12], nHeight[12];
	SingleUcharImage  TempImage;
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
		if (!pOutPyramid[i].DownScaleImagex2(&pOutPyramid[i+1], false))return false;
		if (!pOutPyramid[i + 1].UpScaleImagex2(&TempImage, false))return false;
		if (!pOutPyramid[i].SubtractEdgeImage(&TempImage, pOutEdgePyramid + i))return false;
	}
	return true;
}
bool SingleUcharImage::GetSamplePyramid(SingleUcharImage *pOutPyramid, int nPyramidLevel)
{
	pOutPyramid[0].Clone(this);
	for (int i = 0; i < nPyramidLevel; i++)
	{
		if (!pOutPyramid[i].DownScaleImageSamplex2(&pOutPyramid[i + 1]))return false;
	}
	return true;
}
bool SingleUcharImage::CombineGaussPyramidAndEdgeImage(MultiShortImage *pInputEdgePyramid, int nPyramidLevel)
{
	//SingleUcharImage TempImage;
	//int nWidth[12], nHeight[12];
	//for (int i = nPyramidLevel - 1; i >= 0; i--)
	//{
	//	if (!UpScaleImagex2(&TempImage, false))return false;
	//	if (!AddBackEdgeImage(&TempImage, &pInputEdgePyramid[i]))return false;//this

	//	if (!CopyImageRect(&TempImage, 0, 0, nWidth[i], nHeight[i]))return false;
	//}
	return true;
}
bool SingleUcharImage::NonMaximumImage(CImage_FLOAT *pImage, TFeaturePoint *pPtList, int &nPtNum, int nMaxNum)
{
	int i, y;
	int nWidth = pImage->GetImageWidth();
	int nHeight = pImage->GetImageHeight();
	float *pInLines[7];
	float *pBuffer = new float[nWidth * 7];
	if (pBuffer == NULL)return false;
	pInLines[0] = pBuffer;
	for (i = 1; i < 7; i++)
	{
		pInLines[i] = pInLines[i - 1] + 1;
	}
	float *pInLine = pImage->GetImageData();
	for (i = 0; i < 6; i++)
	{
		RotateLine(pInLine, pInLines[i], nWidth, 7);
		pInLine += nWidth;
	}
	nPtNum = 0;
	for (y = 3; y < nHeight - 3; y++)
	{
		RotateLine(pInLine, pInLines[6], nWidth, 7);
		pInLine += nWidth;
		NonMaximumLine(pInLines, y, nWidth, 7, pPtList, nPtNum, nMaxNum);
		float *pTemp = pInLines[0];
		for (i = 0; i < 6; i++)
		{
			pInLines[i] = pInLines[i + 1];
		}
		pInLines[6] = pTemp;
	}
	delete[] pBuffer;
	return true;
}
bool SingleUcharImage::ComputeCornernessImage(CImage_FLOAT *pOutImage, float fHarris_K, int nHarrisThre, int nRatio, int nPtThre1, int nPtThre2, int nThre)
{
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	unsigned char *pInLines[7];
	if (!pOutImage->SetImageSize(nWidth, nHeight, 1))return false;
	int nProcs = omp_get_num_procs();
	unsigned char *pBuffer = new unsigned char[nWidth * 8 * nProcs];
	if (pBuffer == NULL)return false;
	int loop = 0;
	#pragma omp parallel for  num_threads(nProcs) firstprivate(loop) private(pInLines)
	for (int y = 0; y < nHeight - 3; y++)
	{
		if (loop == 0)
		{
			int nThreadId = omp_get_thread_num();
			pInLines[0] = pBuffer + nWidth * 7 * nThreadId + 0;
			pInLines[1] = pBuffer + nWidth * 7 * nThreadId + 1;
			pInLines[2] = pBuffer + nWidth * 7 * nThreadId + 2;
			pInLines[3] = pBuffer + nWidth * 7 * nThreadId + 3;
			pInLines[4] = pBuffer + nWidth * 7 * nThreadId + 4;
			pInLines[5] = pBuffer + nWidth * 7 * nThreadId + 5;
			pInLines[6] = pBuffer + nWidth * 7 * nThreadId + 6;
			RotateLine(GetImageLine(y - 3), pInLines[0], nWidth, 7);
			RotateLine(GetImageLine(y - 2), pInLines[1], nWidth, 7);
			RotateLine(GetImageLine(y - 1), pInLines[2], nWidth, 7);
			RotateLine(GetImageLine(y + 0), pInLines[3], nWidth, 7);
			RotateLine(GetImageLine(y + 1), pInLines[4], nWidth, 7);
			RotateLine(GetImageLine(y + 2), pInLines[5], nWidth, 7);
			loop++;
		}
		if (y >= nHeight - 3)
		{
			pInLines[6] = pInLines[5];
			ComputeCornernessLine(pInLines, pOutImage->GetImageLine(y), nWidth, 7,  fHarris_K,  nHarrisThre,  nRatio,  nPtThre1,  nPtThre2,  nThre);
			for (int i = 0; i < 6; i++)
			{
				pInLines[i] = pInLines[i + 1];
			}
		}
		else
		{
			RotateLine(GetImageLine(3 + y), pInLines[6], nWidth, 7);
			ComputeCornernessLine(pInLines, pOutImage->GetImageLine(y), nWidth, 7, fHarris_K, nHarrisThre, nRatio, nPtThre1, nPtThre2, nThre);
			unsigned char *pTemp = pInLines[0];
			for (int i = 0; i < 6; i++)
			{
				pInLines[i] = pInLines[i + 1];
			}
			pInLines[6] = pTemp;
		}
	}
	delete[] pBuffer;
	return true;
}
bool SingleUcharImage::DetectFeaturePoint(TFeaturePoint *pPtList, int &nPtNum, int nMaxNum, float fHarris_K, int nHarrisThre, int nRatio, int nPtThre1, int nPtThre2, int nThre)
{
	CImage_FLOAT CornerImage;
	if (!ComputeCornernessImage(&CornerImage, fHarris_K, nHarrisThre, nRatio, nPtThre1, nPtThre2, nThre))return false;
	if (!NonMaximumImage(&CornerImage, pPtList, nPtNum, nMaxNum))return false;
	return true;
}
bool SingleUcharImage::UpdateFeaturePoint(TFeaturePoint *pPtList, int nPtNum,float fHarris_K)
{
	int nWidth = GetImageWidth();
	int nProcs = omp_get_num_procs();
	#pragma omp parallel for  
	for (int i = 0; i < nPtNum; i++)
	{
		int j, k, m, n, x, y, x1, y1, x2, y2, C;
		float R[5][5], max;
		unsigned char nYWin[12][11];// 1 more line for neon 
		pPtList[i].fX *= 2.0;
		pPtList[i].fY *= 2.0;
		x = (int)(pPtList[i].fX + 0.5);
		y = (int)(pPtList[i].fY + 0.5);
		GetImagePixelBlock(x - 5, x + 5, y - 5, y + 5, &nYWin[0][0]);
		max = 0;
		for (j = 0; j < 5; j++)
		{
			for (k = 0; k < 5; k++)
			{
				R[j][k] = ComputeHarrisCorner(nYWin, k, j, fHarris_K);
				if (max < R[j][k])max = R[j][k];
			}
		}
		if (max > 0)
		{
			C = 0;
			pPtList[i].fX = pPtList[i].fY = 0.0;
			for (y1 = y - 2, j = 0; y1 <= y + 2; y1++, j++)
			{
				for (x1 = x - 2, k = 0; x1 <= x + 2; x1++, k++)
				{
					if (R[j][k] == max)
					{
						pPtList[i].fX += x1;
						pPtList[i].fY += y1;
						C++;
					}
				}
			}
			if (C > 1)
			{
				pPtList[i].fX /= C;
				pPtList[i].fY /= C;
			}
		}
		pPtList[i].fScore = max;
	}
	return true;
}
bool SingleUcharImage::MarkFeaturePoint(char *pFileName, TFeaturePoint pPtList[], int nNum, int nS, int nC)
{
	int i, j, x, y, Y;
	int nWidth = GetImageWidth();
	int nHeight = GetImageHeight();
	MultiUcharImage RGBImage;
	if (!RGBImage.CreateImage(nWidth, nHeight))return false;
	for (y = 0; y < nHeight; y++)
	{
		unsigned char *pIn = GetImageLine(y);
		unsigned char *pOut = RGBImage.GetImageLine(y);
		for (x = 0; x < nWidth; x++)
		{
			Y = *(pIn++);
			for (i = 0; i < 3; i++)
			{
				*(pOut++) = (unsigned char)Y;
			}
		}
	}
	for (i = 0; i < nNum; i++)
	{
		x = (int)(pPtList[i].fX + 0.5);
		y = (int)(pPtList[i].fY + 0.5);
		for (j = -nS; j <= nS; j++)
		{
			unsigned char *pPixel1 = RGBImage.GetImagePixel(x + j, y);
			pPixel1[0] = (unsigned char)((i*nC) & 255);
			pPixel1[1] = (unsigned char)((i*nC * 2) & 255);
			pPixel1[2] = (unsigned char)((255 - i * nC * 3) & 255);
			unsigned char *pPixel2 = RGBImage.GetImagePixel(x, y + j);
			pPixel2[0] = (unsigned char)((i*nC) & 255);
			pPixel2[1] = (unsigned char)((i*nC * 2) & 255);
			pPixel2[2] = (unsigned char)((255 - i * nC * 3) & 255);
		}
	}
	return RGBImage.SaveBGRToBitmapFile(pFileName);
}
bool SingleUcharImage::ResizeCubicImage(SingleUcharImage *pOutImage, int OutnWidth, int OutnHeight)
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
bool SingleUcharImage::ResizeNearestNeighborImage(SingleUcharImage *pOutImage, int OutnWidth, int OutnHeight)
{
	int nInWidth = GetImageWidth();
	int nInHeight = GetImageHeight();
	float rx = (float)OutnWidth / (float)nInWidth;
	float ry = (float)OutnHeight / (float)nInHeight;
	int nThread = omp_get_num_procs();
	#pragma omp parallel for num_threads(nThread) schedule(dynamic,nThread)
	for (int y = 0; y < OutnHeight; y++)
	{
		unsigned char *pOutline = pOutImage->GetImageLine(y);
		for (int x = 0; x < OutnWidth; x++)
		{
			int xx = CLIP ((int)((float)x / rx), 0, nInWidth - 1);
			int yy = CLIP((int)((float)y / ry), 0, nInHeight - 1);
			unsigned char *pInline = GetImageLine(yy);
			*pOutline++ = pInline[xx];
		}
	}
	return true;
}
bool SingleUcharImage::SaveGrayToBitmapFile(char *pFileName)
{
	int i, j;
	BITMAPFILEHEADER BmpFileHdr;
	BITMAPINFO *pInfo;
	long nBitsSize, nBISize;
	FILE* fp = fopen(pFileName, "wb");
	if(fp==NULL)return false;
	nBISize = sizeof(BITMAPINFOHEADER);
	pInfo=(BITMAPINFO *) malloc(nBISize);
	pInfo->bmiHeader.biBitCount=24;
	pInfo->bmiHeader.biWidth=m_nWidth;
	pInfo->bmiHeader.biHeight=m_nHeight;
	pInfo->bmiHeader.biCompression=BI_RGB;  
	pInfo->bmiHeader.biSizeImage=0;
	pInfo->bmiHeader.biPlanes=1;
	pInfo->bmiHeader.biClrImportant=0;
	pInfo->bmiHeader.biClrUsed=0;
	pInfo->bmiHeader.biXPelsPerMeter=75;
	pInfo->bmiHeader.biYPelsPerMeter=75;
	pInfo->bmiHeader.biSize=sizeof(BITMAPINFOHEADER);  		
	int nPitch = (m_nWidth * 24 + 31) / 32 * 4;
	nBitsSize = nPitch * m_nHeight;
	BmpFileHdr.bfType = 0x4D42;		// 'BM'
	BmpFileHdr.bfSize = sizeof(BITMAPFILEHEADER) + nBISize + nBitsSize;
	BmpFileHdr.bfReserved1 = 0;
	BmpFileHdr.bfReserved2 = 0;
	BmpFileHdr.bfOffBits = sizeof(BITMAPFILEHEADER) + nBISize;
	if ( fwrite(&BmpFileHdr, 1, sizeof(BITMAPFILEHEADER), fp) != sizeof(BITMAPFILEHEADER) )
	{
		fclose(fp);
		free(pInfo);
		return false;
	}
	if ( fwrite(pInfo, 1, nBISize, fp) != (unsigned long) nBISize )
	{
		fclose(fp);
		free(pInfo);
		return false;
	}
	unsigned char *pBuffer=new unsigned char[nPitch];
	for(i=0; i<m_nHeight; i++)
	{
		unsigned char *pScanLine=GetImageLine(m_nHeight-1-i);
		for(j=0; j<m_nWidth; j++)
		{
			unsigned char g=pScanLine[j];
			pBuffer[j*3]=pBuffer[j*3+1]=pBuffer[j*3+2]=g;
		}
		if(fwrite(pBuffer, 1, nPitch, fp)!=(unsigned long) nPitch)
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
bool SingleUcharImage::LoadBitmapFileToGray(char *pFileName)
{
	int i, j;
	long nBitsSize, nBISize;
	BITMAPFILEHEADER BmpFileHdr;
	long nBytes;
	FILE *fp=fopen(pFileName,"rb");
	if(fp==NULL)return false;
	nBytes = fread(&BmpFileHdr, 1, sizeof(BITMAPFILEHEADER), fp);
	if ( nBytes != sizeof(BITMAPFILEHEADER) )
	{
		fclose(fp);
		return false;
	}
	if ( BmpFileHdr.bfType != 0x4D42 )
	{
		fclose(fp);
		return false;
	}
	BITMAPINFOHEADER BmpInfoHdr;
	nBytes = fread(&BmpInfoHdr, 1, sizeof(BITMAPINFOHEADER), fp);
	if ( nBytes != sizeof(BITMAPINFOHEADER) )
	{
		fclose(fp);
		return false;
	}
	if((BmpInfoHdr.biBitCount!=24)||(BmpInfoHdr.biSize != sizeof(BITMAPINFOHEADER)))
	{
		fclose(fp);
		return false;
	}
	int nPitch = (BmpInfoHdr.biWidth * BmpInfoHdr.biBitCount + 31) / 32 * 4;
	if ( BmpInfoHdr.biSizeImage == 0 )
		BmpInfoHdr.biSizeImage = nPitch * BmpInfoHdr.biHeight;

	nBISize = sizeof(BITMAPINFOHEADER);
	nBitsSize = BmpFileHdr.bfSize - BmpFileHdr.bfOffBits;
	if ( nBitsSize < nPitch * BmpInfoHdr.biHeight )
		nBitsSize = nPitch * BmpInfoHdr.biHeight;
	if(!CreateImage(BmpInfoHdr.biWidth, BmpInfoHdr.biHeight))
	{
		fclose(fp);
		return false;
	}
	fseek(fp, BmpFileHdr.bfOffBits, SEEK_SET);
	unsigned char *pBuffer=new unsigned char[nPitch];
	for(i=0; i<m_nHeight; i++)
	{
		unsigned char *pLine=GetImageLine(m_nHeight-1-i);
		if((nBytes =fread(pBuffer, 1, nPitch, fp))!=nPitch)
		{
			fclose(fp);
			delete[] pBuffer;
			return false;
		}
		for(j=0; j<m_nWidth; j++)
		{
			int b, g, r, Y;
			b=pBuffer[j*3];
			g=pBuffer[j*3+1];
			r=pBuffer[j*3+2];
			Y=(b*2+g*9+r*5)>>4;
			pLine[j]=(unsigned char) Y;
		}
	}
	fclose(fp);
	delete[] pBuffer;
	return true;
}
