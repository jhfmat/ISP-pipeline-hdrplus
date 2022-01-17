#include "Common.h"
#include "Basicdef.h"
#include "SubFunction.h"
extern bool AnalysisHistIsSingleColor(int nHistHue[],int nMaxValue, int nHistRangeNumThre,int nHistRange,unsigned int nRealHistRangeNum)
{
	bool flag = false;
	int Max = 0;
	for (int j = 0; j < nMaxValue; j++)
	{
		int sumhistHue = 0;
		int end = j + nHistRange;
		if (end > nMaxValue)
		{
			end = nMaxValue;
		}
		for (int n = j; n < end; n++)
		{
			sumhistHue += nHistHue[n];
		}
		if (sumhistHue > Max)
		{
			Max = sumhistHue;
		}
		if (sumhistHue > nHistRangeNumThre)
		{
			flag = true;
		}
	}
	nRealHistRangeNum = Max;
	return flag;
}
extern bool AnalysisRGBHistIsSingleColor(unsigned int nHistR[], unsigned int nHistG[], unsigned int nHistB[], int nMaxValue, int nHistRangeNumThre, int nHistRange, unsigned int nRealHistRangeNum[])
{
	bool flagR = false;
	bool flagG = false;
	bool flagB = false;
	int MaxR = 0;
	int MaxG = 0;
	int MaxB = 0;
	for (int j = 0; j < nMaxValue; j++)
	{
		unsigned long long sumhistRGB[3] = {0,0,0};
		int end = j + nHistRange;
		if (end > nMaxValue)
		{
			end = nMaxValue;
		}
		for (int n = j; n < end; n++)
		{
			sumhistRGB[0] += nHistR[n];
			sumhistRGB[1] += nHistG[n];
			sumhistRGB[2] += nHistB[n];
		}
		if (sumhistRGB[0] > MaxR)
		{
			MaxR = sumhistRGB[0];
		}
		if (sumhistRGB[1] > MaxG)
		{
			MaxG = sumhistRGB[1];
		}
		if (sumhistRGB[2] > MaxB)
		{
			MaxB = sumhistRGB[2];
		}
	}
	if (MaxR > nHistRangeNumThre)
	{
		flagR = true;
	}
	if (MaxG > nHistRangeNumThre)
	{
		flagG = true;
	}
	if (MaxB > nHistRangeNumThre)
	{
		flagB = true;
	}
	nRealHistRangeNum[0] = MaxR;
	nRealHistRangeNum[1] = MaxG;
	nRealHistRangeNum[2] = MaxB;
	if (flagR == true && flagG == true && flagB == true)
	{
		return true;
	}
	else
	{
		return false;
	}
}
extern void AnalysisRGBHistToMeanY(unsigned int nHistR[], unsigned int nHistG[], unsigned int nHistB[],int nWidth,int nHeight, int nMaxValue,unsigned char &MeanYValue,unsigned char MeanRGBValue[])
{
	unsigned long long sumhistRGB[3] = { 0,0,0 };
	unsigned int MeanY = 0;
	for (int j = 0; j < nMaxValue; j++)
	{
		sumhistRGB[0] += nHistR[j]*j;
		sumhistRGB[1] += nHistG[j]*j;
		sumhistRGB[2] += nHistB[j]*j;
	}
	unsigned int size = nWidth * nHeight;
	MeanRGBValue[0] = (unsigned char)(sumhistRGB[0]/size);
	MeanRGBValue[1] = (unsigned char)(sumhistRGB[1]/size);
	MeanRGBValue[2] = (unsigned char)(sumhistRGB[2]/size);
	MeanYValue = (unsigned char)((sumhistRGB[2] * 29 + sumhistRGB[1] * 150 + sumhistRGB[0] * 77 + 128) >> 8);
}

extern void HistIntToHistfloat(int pHist[256], float pfHist[256])
{
	for (int i = 1; i < 256; i++)
	{
		pHist[i] += pHist[i - 1];//累计pHist代表这个亮度之前所有的总和
	}
	//pHist[255]代表总数量=nx*ny;
	for (int i = 0; i < 256; i++)
		pfHist[i] = (float)pHist[i] / (float)pHist[255];
}
extern void FindHistSpecification(float SrcHistNum[256], float DstHistNum[256], int map[256])
{
	float minValue = 0;
	float srcMin[256][256];
	short lastStartY = 0, lastEndY = 0, startY = 0, endY = 0;
	//图像每个等级数量与两一个图像等级数量
	for (int y = 0; y < 256; y++)
	{
		for (int x = 0; x < 256; x++)
		{
			srcMin[x][y] = fabs(SrcHistNum[y] - DstHistNum[x]);//srcMin[x][y]记录原直方图第y个数据与
		}
	}
	for (int x = 0; x < 256; x++)//一维找到最小
	{
		minValue = srcMin[x][0];
		for (int y = 0; y < 256; y++)//从二维找到最小的
		{
			if (minValue > srcMin[x][y])
			{
				endY = y;//记录最小的位子
				minValue = srcMin[x][y];
			}
		}
		if (startY != lastStartY || endY != lastEndY)
		{
			for (int i = startY; i <= endY; i++)
			{
				map[i] = x;
			}
			lastStartY = startY;
			lastEndY = endY;
			startY = lastEndY + 1;
		}
	}
}
extern float HistogramMeanValue(int pHist[256])
{
	int num = 0;
	for (int i = 0; i < 256; i++)
	{
		num += pHist[i];
	}
	float sum = 0;
	for (int i = 0; i < 256; i++)
	{
		sum += (float)(i*pHist[i]) / (float)num;
	}
	return sum;
}
extern void HGaussianLine3(unsigned char *pInLine, unsigned short *pOutLine, int nWidth, int nChannel)
{
	int i, x, Y;
	unsigned char *pIn[3];
	pIn[0] = pIn[1] = pInLine;
	pIn[2] = pIn[1] + nChannel;
	if (nChannel == 1)
	{
		x = 0;
#ifdef USE_NEON
		for (; x < nWidth - 8; x += 8)
		{
			uint8x8_t  vIn[3];
			vIn[0] = vld1_u8(pIn[0]);
			vIn[1] = vld1_u8(pIn[1]);
			vIn[2] = vld1_u8(pIn[2]);
			uint16x8_t vY = vaddl_u8(vIn[0], vIn[2]);
			vY = vmlaq_n_u16(vY, vmovl_u8(vIn[1]), 2);
			vst1q_u16(pOutLine, vY);
			pOutLine += 8;
			pIn[0] += 7;
			pIn[1] += 7;
			pIn[2] += 7;
			pIn[0] = pIn[1];
			pIn[1] = pIn[2];
			pIn[2] ++;
		}
#endif
		for (; x < nWidth - 1; x++)
		{
			pOutLine[0] = (unsigned short)((pIn[0][0] + pIn[2][0] + pIn[1][0] * 2));
			pOutLine += 1;
			pIn[0] = pIn[1];
			pIn[1] = pIn[2];
			pIn[2] ++;
		}
	}
	else if (nChannel == 2)
	{
		for (x = 0; x < nWidth - 1; x++)
		{
			pOutLine[0] = (unsigned short)((pIn[0][0] + pIn[2][0] + pIn[1][0] * 2));
			pOutLine[1] = (unsigned short)((pIn[0][1] + pIn[2][1] + pIn[1][1] * 2));
			pOutLine += 2;
			pIn[0] = pIn[1];
			pIn[1] = pIn[2];
			pIn[2] += 2;
		}
	}
	else
	{
		for (x = 0; x < nWidth - 1; x++)
		{
			for (i = 0; i < nChannel; i++)
			{
				Y = pIn[0][i] + pIn[2][i];
				Y += pIn[1][i] * 2;
				pOutLine[i] = (unsigned short)Y;
			}
			pOutLine += nChannel;
			pIn[0] = pIn[1];
			pIn[1] = pIn[2];
			pIn[2] += nChannel;
		}
	}
	pIn[2] = pIn[1];
	for (i = 0; i < nChannel; i++)
	{
		Y = pIn[0][i] + pIn[2][i];
		Y += pIn[1][i] * 2;
		pOutLine[i] = (unsigned short)Y;
	}
	pOutLine += nChannel;
}
extern void VGaussianLine3(unsigned short *pInLines[3], unsigned char *pOutLine, int nWidth,int nChannel)//交替存储
{
	int  x;
	x = 0;
#ifdef USE_NEON
	uint16x8_t neonvalue8= vdupq_n_u16(8);
	for (; x < nWidth*nChannel - 7; x += 8)
	{
		uint16x8_t vIn[3];
		vIn[0] = vld1q_u16(pInLines[0] + x);
		vIn[1] = vld1q_u16(pInLines[1] + x);
		vIn[2] = vld1q_u16(pInLines[2] + x);
		uint16x8_t vY = vaddq_u16(vIn[0], vIn[2]);
		vY = vmlaq_n_u16(vY, vIn[1], 2);
		vY = vaddq_u16(vY, neonvalue8);
		vst1_u8(pOutLine, vshrn_n_u16(vY, 4));
		pOutLine += 8;
	}
#endif
	for (; x < nWidth*nChannel; x++)
	{
		unsigned short Y = pInLines[0][x] + pInLines[2][x]+ pInLines[1][x] * 2+8;
		Y = Y >> 4;
		*(pOutLine++) = Y;
	}
}
extern void HGaussianLine5(unsigned char *pInLine, unsigned short *pOutLine, int nWidth, int nChannel)
{
	int x, out, i;
	x = 0;
	for (i = 0; i < nChannel; i++)
	{
		out = (pInLine[0 * nChannel + i] + pInLine[(x + 2)*nChannel + i]);
		out += (pInLine[0 * nChannel + i] + pInLine[(x + 1)*nChannel + i]) * 4;
		out += pInLine[x*nChannel + i] * 6;
		pOutLine[x*nChannel + i] = (unsigned short)out;
	}
	x = 1;
	for (i = 0; i < nChannel; i++)
	{
		out = (pInLine[0 * nChannel + i] + pInLine[(x + 2)*nChannel + i]);
		out += (pInLine[(x - 1)*nChannel + i] + pInLine[(x + 1)*nChannel + i]) * 4;
		out += pInLine[x*nChannel + i] * 6;
		pOutLine[x*nChannel + i] = (unsigned short)out;
	}
	x = 2;
#ifdef USE_NEON
	if (nChannel == 1)
	{
		uint16x8_t vTemp, vOut;
		uint8x8_t vIn0, vIn1;
		vIn0 = vld1_u8(pInLine + x - 2);
		for (; x < (nWidth - 12) / 8 * 8; x += 8)
		{
			vIn1 = vld1_u8(pInLine + x + 6);
			vOut = vaddl_u8(vIn0, vext_u8(vIn0, vIn1, 4));
			vTemp = vaddl_u8(vext_u8(vIn0, vIn1, 1), vext_u8(vIn0, vIn1, 3));
			vIn0 = vext_u8(vIn0, vIn1, 2);
			vTemp = vaddw_u8(vTemp, vIn0);
			vTemp = vshlq_n_u16(vTemp, 2);
			vOut = vaddq_u16(vOut, vTemp);
			vTemp = vshll_n_u8(vIn0, 1);
			vOut = vaddq_u16(vOut, vTemp);
			vst1q_u16(pOutLine + x, vOut);
			vIn0 = vIn1;
		}
	}
#endif
	for (; x < nWidth - 2; x++)
	{
		for (i = 0; i < nChannel; i++)
		{
			out = (pInLine[(x - 2)*nChannel + i] + pInLine[(x + 2)*nChannel + i]);
			out += (pInLine[(x - 1)*nChannel + i] + pInLine[(x + 1)*nChannel + i]) * 4;
			out += pInLine[x*nChannel + i] * 6;
			pOutLine[x*nChannel + i] = (unsigned short)out;
		}
	}
	x = nWidth - 2;
	for (i = 0; i < nChannel; i++)
	{
		out = (pInLine[(x - 2)*nChannel + i] + pInLine[(nWidth - 1)*nChannel + i]);
		out += (pInLine[(x - 1)*nChannel + i] + pInLine[(x + 1)*nChannel + i]) * 4;
		out += pInLine[x*nChannel + i] * 6;
		pOutLine[x*nChannel + i] = (unsigned short)out;
	}
	x = nWidth - 1;
	for (i = 0; i < nChannel; i++)
	{
		out = (pInLine[(x - 2)*nChannel + i] + pInLine[(nWidth - 1)*nChannel + i]);
		out += (pInLine[(x - 1)*nChannel + i] + pInLine[(nWidth - 1)*nChannel + i]) * 4;
		out += pInLine[x*nChannel + i] * 6;
		pOutLine[x*nChannel + i] = (unsigned short)out;
	}
}
extern void VGaussianLine5(unsigned short *pInLines[5], unsigned char *pOutLine, int nWidth, int nChannel)
{
	int x, out, i;
	x = 0;
#ifdef USE_NEON
	uint16x8_t neonvalue128 = vdupq_n_u16(128);
	if (nChannel == 1)
	{
		for (; x < nWidth / 8 * 8; x += 8)
		{
			uint16x8_t vTemp;
			uint16x8_t vIn0 = vld1q_u16(pInLines[0] + x);
			uint16x8_t vIn1 = vld1q_u16(pInLines[4] + x);
			uint16x8_t vOut = vaddq_u16(vIn0, vIn1);
			vIn0 = vld1q_u16(pInLines[1] + x);
			vIn1 = vld1q_u16(pInLines[3] + x);
			vTemp = vaddq_u16(vIn0, vIn1);
			vIn0 = vld1q_u16(pInLines[2] + x);
			vTemp = vaddq_u16(vTemp, vIn0);
			vTemp = vshlq_n_u16(vTemp, 2);
			vOut = vaddq_u16(vOut, vTemp);
			vTemp = vshlq_n_u16(vIn0, 1);
			vOut = vaddq_u16(vOut, vTemp);
			vOut = vaddq_u16(vOut, neonvalue128);
			vst1_u8(pOutLine + x, vrshrn_n_u16(vOut, 8));
		}
	}
	else if (nChannel == 3)//MISMATCH
	{
		for (; x < nWidth - 7; x += 8)
		{
			uint16x8x3_t vTemp, vOut;
			uint16x8x3_t vIn0 = vld3q_u16(pInLines[0] + 3 * x);
			uint16x8x3_t vIn1 = vld3q_u16(pInLines[4] + 3 * x);

			vOut.val[0] = vaddq_u16(vIn0.val[0], vIn1.val[0]);
			vOut.val[1] = vaddq_u16(vIn0.val[1], vIn1.val[1]);
			vOut.val[2] = vaddq_u16(vIn0.val[2], vIn1.val[2]);

			vIn0 = vld3q_u16(pInLines[1] + 3 * x);
			vIn1 = vld3q_u16(pInLines[3] + 3 * x);
			vTemp.val[0] = vaddq_u16(vIn0.val[0], vIn1.val[0]);
			vTemp.val[1] = vaddq_u16(vIn0.val[1], vIn1.val[1]);
			vTemp.val[2] = vaddq_u16(vIn0.val[2], vIn1.val[2]);

			vIn0 = vld3q_u16(pInLines[2] + 3 * x);
			vTemp.val[0] = vaddq_u16(vTemp.val[0], vIn0.val[0]);
			vTemp.val[1] = vaddq_u16(vTemp.val[1], vIn0.val[1]);
			vTemp.val[2] = vaddq_u16(vTemp.val[2], vIn0.val[2]);

			vTemp.val[0] = vshlq_n_u16(vTemp.val[0], 2);
			vTemp.val[1] = vshlq_n_u16(vTemp.val[1], 2);
			vTemp.val[2] = vshlq_n_u16(vTemp.val[2], 2);

			vOut.val[0] = vaddq_u16(vOut.val[0], vTemp.val[0]);
			vOut.val[1] = vaddq_u16(vOut.val[1], vTemp.val[1]);
			vOut.val[2] = vaddq_u16(vOut.val[2], vTemp.val[2]);


			vTemp.val[0] = vshlq_n_u16(vIn0.val[0], 1);
			vTemp.val[1] = vshlq_n_u16(vIn0.val[1], 1);
			vTemp.val[2] = vshlq_n_u16(vIn0.val[2], 1);

			vOut.val[0] = vaddq_u16(vOut.val[0], vTemp.val[0]);
			vOut.val[1] = vaddq_u16(vOut.val[1], vTemp.val[1]);
			vOut.val[2] = vaddq_u16(vOut.val[2], vTemp.val[2]);
			uint8x8x3_t vu8Out;

			vOut.val[0] = vaddq_u16(vOut.val[0], neonvalue128);
			vOut.val[1] = vaddq_u16(vOut.val[1], neonvalue128);
			vOut.val[2] = vaddq_u16(vOut.val[2], neonvalue128);
			vu8Out.val[0] = vrshrn_n_u16(vOut.val[0], 8);
			vu8Out.val[1] = vrshrn_n_u16(vOut.val[1], 8);
			vu8Out.val[2] = vrshrn_n_u16(vOut.val[2], 8);
			vst3_u8(pOutLine + 3 * x, vu8Out);
		}
	}
#endif
	for (; x < nWidth; x++)
	{
		for (i = 0; i < nChannel; i++)
		{
			out = pInLines[0][x*nChannel + i] + pInLines[4][x*nChannel + i];
			out += (pInLines[1][x*nChannel + i] + pInLines[3][x*nChannel + i]) * 4;
			out += pInLines[2][x*nChannel + i] * 6;
			out += 128;
			out >>= 8;
			if (out > 255) out = 255;
			pOutLine[x*nChannel + i] = (unsigned char)out;
		}
	}
}
extern void HDilation3Line(unsigned char *pInLine, unsigned char *pOutLine, int nWidth)
{
	int x;
	unsigned char In[3];
	In[0] = 0;
	In[1] = *(pInLine++);
	for (x = 0; x < nWidth - 1; x++)
	{
		In[2] = *(pInLine++);
		*pOutLine = (In[0] | In[1] | In[2]);
		pOutLine += 3;
		In[0] = In[1];
		In[1] = In[2];
	}
	*pOutLine = (In[0] | In[1]);
	pOutLine += 3;
}
extern void VDilation3Line(unsigned char *pInLine, unsigned char *pOutLine, int nWidth)
{
	int x;
	for (x = 0; x < nWidth; x++)
	{
		*(pOutLine++) = (pInLine[0] | pInLine[1] | pInLine[2]);
		pInLine += 3;
	}
}
extern void Dilation3x3Line(unsigned char *pInLines[], unsigned char *pOutLine, int nWidth)
{
	int i, x;
	unsigned char Buffer[9];
	unsigned char *pIn[3];
	unsigned char *pWin[3];
	for (i = 0; i < 3; i++)
	{
		pIn[i] = pInLines[i];
		pWin[i] = Buffer + i * 3;
	}
	for (i = 0; i < 3; i++)
	{
		pWin[0][i] = 0;
		pWin[1][i] = pIn[i][0];
		pIn[i] += 3;
	}
	for (x = 0; x < nWidth - 1; x++)
	{
		for (i = 0; i < 3; i++)
		{
			pWin[2][i] = pIn[i][0];
			pIn[i] += 3;
		}
		*(pOutLine++) = (pWin[0][1] | pWin[1][0] | pWin[1][1] | pWin[1][2] | pWin[2][1]);
		unsigned char *pTemp = pWin[0];
		pWin[0] = pWin[1];
		pWin[1] = pWin[2];
		pWin[2] = pTemp;
	}
	{
		*(pOutLine++) = (pWin[0][1] | pWin[1][0] | pWin[1][1] | pWin[1][2]);
	}
}
extern void HErosion3Line(unsigned char *pInLine, unsigned char *pOutLine, int nWidth)
{
	int x;
	x = 0;
#ifdef USE_NEON
	uint8x16_t vIn0, vIn1;
	vIn0 = vld1q_u8(pInLine + x - 1);
	for (; x < nWidth - 15; x += 16)
	{
		vIn1 = vld1q_u8(pInLine + x - 1 + 16);
		uint8x16_t vOut = vIn0;
		vOut = vandq_u8(vOut, vextq_u8(vIn0, vIn1, 1));
		vOut = vandq_u8(vOut, vextq_u8(vIn0, vIn1, 2));
		vst1q_u8(pOutLine + x, vOut);
		vIn0 = vIn1;
	}
#endif
	for (; x < nWidth; x++)
	{
		pOutLine[x] = (pInLine[x - 1] & pInLine[x] & pInLine[x + 1]);
	}
}
extern void VErosion3Line(unsigned char *pInLine[3], unsigned char *pOutLine, int nWidth)
{
	int x;
	x = 0;
#ifdef USE_NEON
	for (; x < nWidth / 16 * 16; x += 16)
	{
		uint8x16_t vOut = vld1q_u8(pInLine[0] + x);
		vOut = vandq_u8(vOut, vld1q_u8(pInLine[1] + x));
		vOut = vandq_u8(vOut, vld1q_u8(pInLine[2] + x));
		vst1q_u8(pOutLine + x, vOut);
	}
#endif
	for (; x < nWidth; x++)
	{
		pOutLine[x] = (pInLine[0][x] & pInLine[1][x] & pInLine[2][x]);
	}
}
extern void Erosion3x3Line(unsigned char *pInLines[], unsigned char *pOutLine, int nWidth)
{
	int x;
	x = 0;
#ifdef USE_NEON
	for (; x < nWidth / 16 * 16; x += 16)
	{
		uint8x16_t vOut = vld1q_u8(pInLines[0] + x);
		vOut = vandq_u8(vOut, vld1q_u8(pInLines[1] + x - 1));
		vOut = vandq_u8(vOut, vld1q_u8(pInLines[1] + x));
		vOut = vandq_u8(vOut, vld1q_u8(pInLines[1] + x + 1));
		vOut = vandq_u8(vOut, vld1q_u8(pInLines[2] + x));
		vst1q_u8(pOutLine + x, vOut);
	}
#endif
	for (; x < nWidth; x++)
	{
		pOutLine[x] = (pInLines[0][x] & pInLines[1][x - 1] & pInLines[1][x] & pInLines[1][x + 1] & pInLines[2][x]);
	}
}
extern void GetImageIntegralData(unsigned char *pInData, unsigned int *Integral, int Width, int Height, int Stride)
{
	unsigned int *ColSum = (unsigned int *)calloc(Width, sizeof(unsigned int));        //    用的calloc函数哦，自动内存清0
	memset(Integral, 0, (Width + 1) * sizeof(unsigned int));
	for (int Y = 0; Y < Height; Y++)
	{
		unsigned char *LinePS = pInData + Y * Stride;
		unsigned int *LinePD = Integral + (Y + 1) * (Width + 1) + 1;//积分图1 1开始一行起始
		LinePD[-1] = 0;
		for (int X = 0; X < Width; X++)//行方向的积分累加
		{
			ColSum[X] += LinePS[X];//ColSum统计的是的累加LinePS统计行的累加
			LinePD[X] = LinePD[X - 1] + ColSum[X];
		}
	}
	free(ColSum);
}
extern void GetImageIntegralSData(short *pInData, unsigned int *Integral, int Width, int Height, int Stride)
{
	unsigned int *ColSum = (unsigned int *)calloc(Width, sizeof(unsigned int));        //    用的calloc函数哦，自动内存清0
	memset(Integral, 0, (Width + 1) * sizeof(unsigned int));
	for (int Y = 0; Y < Height; Y++)
	{
		short *LinePS = pInData + Y * Stride;
		unsigned int *LinePD = Integral + (Y + 1) * (Width + 1) + 1;//积分图1 1开始一行起始
		LinePD[-1] = 0;
		for (int X = 0; X < Width; X++)//行方向的积分累加
		{
			ColSum[X] += LinePS[X];//ColSum统计的是的累加LinePS统计行的累加
			LinePD[X] = LinePD[X - 1] + ColSum[X];
		}
	}
	free(ColSum);
}
extern void GetImageIntegralUSData(unsigned short *pInData, unsigned int *Integral, int Width, int Height, int Stride)
{
	unsigned int *ColSum = (unsigned int *)calloc(Width, sizeof(unsigned int));        //    用的calloc函数哦，自动内存清0
	memset(Integral, 0, (Width + 1) * sizeof(unsigned int));
	for (int Y = 0; Y < Height; Y++)
	{
		unsigned short *LinePS = pInData + Y * Stride;
		unsigned int *LinePD = Integral + (Y + 1) * (Width + 1) + 1;//积分图1 1开始一行起始
		LinePD[-1] = 0;
		for (int X = 0; X < Width; X++)//行方向的积分累加
		{
			ColSum[X] += LinePS[X];//ColSum统计的是的累加LinePS统计行的累加
			LinePD[X] = LinePD[X - 1] + ColSum[X];
		}
	}
	free(ColSum);
}
extern void FillUcharData(unsigned char *pInData, unsigned char *pOutData, int nInWidth, int nInHeight, int nOutWidth, int nOutHeight, int nChannel)
{
	#pragma omp parallel for
	for (int y = 0; y < nInHeight; y++)
	{
		int i, x;
		unsigned char *pInLine = pInData + y * nInWidth*nChannel;
		unsigned char *pOutLine = pOutData + y * nOutWidth*nChannel;
		memcpy(pOutLine, pInLine, nInWidth*nChannel);
		pOutLine += nInWidth * nChannel;
		x = nInWidth;
		for (; x < nOutWidth; x++)
		{
			for (i = 0; i < nChannel; i++)
			{
				pOutLine[i] = pOutLine[i - nChannel];
			}
			pOutLine += nChannel;
		}
	}
	int y = nInHeight;
	for (; y < nOutHeight; y++)
	{
		unsigned char *pInLine = pOutData + (nInHeight - 1)*nOutWidth*nChannel;
		unsigned char *pOutLine = pOutData + y * nOutWidth*nChannel;
		memcpy(pOutLine, pInLine, nOutWidth*nChannel);
	}
}
extern void FillWordData(unsigned short *pInData, unsigned short *pOutData, int nInWidth, int nInHeight, int nOutWidth, int nOutHeight, int nChannel)
{
#pragma omp parallel for
	for (int y = 0; y < nInHeight; y++)
	{
		int i, x;
		unsigned short *pInLine = pInData + y * nInWidth*nChannel;
		unsigned short *pOutLine = pOutData + y * nOutWidth*nChannel;
		memcpy(pOutLine, pInLine, nInWidth*nChannel*sizeof(unsigned short));
		pOutLine += nInWidth * nChannel;
		x = nInWidth;
		for (; x < nOutWidth; x++)
		{
			for (i = 0; i < nChannel; i++)
			{
				pOutLine[i] = pOutLine[i - nChannel];
			}
			pOutLine += nChannel;
		}
	}
	int y = nInHeight;
	for (; y < nOutHeight; y++)
	{
		unsigned short *pInLine = pOutData + (nInHeight - 1)*nOutWidth*nChannel;
		unsigned short *pOutLine = pOutData + y * nOutWidth*nChannel;
		memcpy(pOutLine, pInLine, nOutWidth*nChannel * sizeof(unsigned short));
	}
}
extern void SubtractUcharEdgeData(unsigned char *pInData0, unsigned char *pInData1, short *pOutImage, int nWidth0, int nHeight0, int nWidth1, int nHeight1, int nChannel)
{
	#pragma omp parallel for
	for (int y = 0; y < nHeight0; y++)
	{
		unsigned char *pIn0 = pInData0 + y * nWidth0 * nChannel;
		unsigned char *pIn1 = pInData1 + y * nWidth1 * nChannel;
		short *pOut = pOutImage + y * nWidth0 * nChannel;
		int x = 0;
#ifdef USE_NEON
		for (; x < nWidth0 * nChannel - 15; x += 16)
		{
			uint8x8_t vIn0[2];
			uint8x8_t vIn1[2];
			vIn0[0] = vld1_u8(pIn0); pIn0 += 8;
			vIn0[1] = vld1_u8(pIn0); pIn0 += 8;
			vIn1[0] = vld1_u8(pIn1); pIn1 += 8;
			vIn1[1] = vld1_u8(pIn1); pIn1 += 8;
			uint16x8_t vout = vsubl_u8(vIn0[0], vIn1[0]);
			vst1q_s16((short *)pOut, vout);
			pOut += 8;
			vout = vsubl_u8(vIn0[1], vIn1[1]);
			vst1q_s16((short *)pOut, vout);
			pOut += 8;
		}
#endif
		for (; x < nWidth0 * nChannel; x++)
		{
			int Y = *(pIn0++);
			Y -= *(pIn1++);
			*(pOut++) = (short)Y;
		}
	}
}
extern void SubtractWordEdgeData(unsigned short *pInImage0, unsigned short *pInImage1, short *pOutImage, int nWidth0, int nHeight0, int nWidth1, int nHeight1, int nChannel)
{
	#pragma omp parallel for
	for (int y = 0; y < nHeight0; y++)
	{
		int x = 0;
		unsigned short *pIn0 = pInImage0 + y * nWidth0 * nChannel;
		unsigned short *pIn1 = pInImage1 + y * nWidth1 * nChannel;
		short *pOut = pOutImage + y * nWidth0 * nChannel;
//#ifdef USE_NEON
//		for (; x < nWidth0*nChannel - 7; x += 8)
//		{
//			int16x8_t vOut = vsubq_u16(vld1q_u16(pIn0), vld1q_u16(pIn1));//if (Y < -32768)Y = -32768;	if (Y > 32767)Y = 32767;
//			vst1q_s16(pOut, vOut);
//			pIn0 += 8;
//			pIn1 += 8;
//			pOut += 8;
//		}
//#endif
		for (; x < nWidth0*nChannel; x++)
		{
			int Y = *(pIn0++);
			Y -= *(pIn1++);
			if (Y < -32768)Y = -32768;	if (Y > 32767)Y = 32767;
			*(pOut++) = (short)Y;
		}
		pIn1 += (nWidth1 - nWidth0)*nChannel;
	}
}
static void HDownScaleUcharLinex2Ch3(unsigned char *pInLine, unsigned short *pOutLine, int nWidth)
{
	int i, x, Y;
	unsigned char *pIn[6];
	x = 0;
#ifdef USE_NEON
	uint8x8x3_t vInD0;
	uint8x16x3_t vInD12;
	uint8x8x3_t vInD123;
	uint16x8x3_t vOut123;
	vInD0 = vld3_u8(pInLine);
	for (int i = 0; i < 3; i++)
	{
		//vInD0.val[i][1] = vInD0.val[i][0] = vInD0.val[i][2];
		vInD0.val[i] = vtbl1_u8(vInD0.val[i], vcreate_u8(0x0504030201000000L));
	}
	for (; x < nWidth - 3 - 7 * 2; x += 2 * 8)
	{
		for (int channel = 0; channel < 3; channel++)
		{
			vInD12 = vld3q_u8(pInLine + 6 * 3);
			uint8x8_t vTbl = vcreate_u8(0x0e0c0a0806040200L);
			uint8x8_t vT11 = vcreate_u8(0x0101010101010101L);

			vInD123.val[0] = vInD0.val[channel];
			vInD123.val[1] = vget_low_u8(vInD12.val[channel]);
			vInD123.val[2] = vget_high_u8(vInD12.val[channel]);

			uint16x8_t vOut;
			uint8x8_t vInTmp = vtbl3_u8(vInD123, vTbl);     //0
			vOut = vmovl_u8(vInTmp);

			vTbl = vadd_u8(vTbl, vT11);
			vInTmp = vtbl3_u8(vInD123, vTbl); //1
			vOut = vmlaq_n_u16(vOut, vmovl_u8(vInTmp), 5);

			vTbl = vadd_u8(vTbl, vT11);
			vInTmp = vtbl3_u8(vInD123, vTbl); //2
			vOut = vmlaq_n_u16(vOut, vmovl_u8(vInTmp), 10);

			vTbl = vadd_u8(vTbl, vT11);
			vInTmp = vtbl3_u8(vInD123, vTbl); //3
			vOut = vmlaq_n_u16(vOut, vmovl_u8(vInTmp), 10);
			vTbl = vadd_u8(vTbl, vT11);
			vInTmp = vtbl3_u8(vInD123, vTbl); //4
			vOut = vmlaq_n_u16(vOut, vmovl_u8(vInTmp), 5);
			vTbl = vadd_u8(vTbl, vT11);
			vInTmp = vtbl3_u8(vInD123, vTbl); //5
			vOut = vaddw_u8(vOut, vInTmp);
			// print_V8(vInTmp);
			// print_V8(vOut);
			vOut123.val[channel] = vOut;
			vInD0.val[channel] = vget_high_u8(vInD12.val[channel]);
		}

		pInLine += 2 * 3 * 8;
		vst3q_u16(pOutLine, vOut123);
		pOutLine += 3 * 8;
	}
#endif
	pIn[0] = x - 2 < 0 ? pInLine : pInLine - 2 * 3;
	pIn[1] = x - 1 < 0 ? pInLine : pInLine - 1 * 3;
	pIn[2] = pInLine;
	pIn[3] = pIn[2] + 3;
	pIn[4] = pIn[3] + 3;
	pIn[5] = pIn[4] + 3;
	for (; x < nWidth - 3; x += 2)
	{
		i = 0;
		for (; i < 3; i++)
		{
			Y = (int)pIn[0][i];
			Y += (int)pIn[1][i] * 5;
			Y += (int)pIn[2][i] * 10;
			Y += (int)pIn[3][i] * 10;
			Y += (int)pIn[4][i] * 5;
			Y += (int)pIn[5][i];

			pOutLine[i] = (unsigned short)Y;
		}
		pIn[0] = pIn[2];
		pIn[1] = pIn[3];
		pIn[2] = pIn[4];
		pIn[3] = pIn[5];
		if (x < nWidth - 3)
		{
			pIn[4] += 3 * 2;
			pIn[5] += 3 * 2;
		}
		pOutLine += 3;
	}
	pIn[4] = pIn[2];
	pIn[5] = pIn[3];
	for (; x < nWidth; x += 2)
	{
		for (i = 0; i < 3; i++)
		{
			Y = (int)pIn[0][i];
			Y += (int)pIn[1][i] * 5;
			Y += (int)pIn[2][i] * 10;
			Y += (int)pIn[3][i] * 10;
			Y += (int)pIn[4][i] * 5;
			Y += (int)pIn[5][i];

			pOutLine[i] = (unsigned short)Y;
		}
		pIn[0] = pIn[2];
		pIn[1] = pIn[3];
		pIn[2] = pIn[4];
		pIn[3] = pIn[5];

		pOutLine += 3;
	}
}
extern void HDownScaleUcharLinex2(unsigned char *pInLine, unsigned short *pOutLine, int nWidth, int nChannel)
{
	if (nChannel == 3)
	{
		HDownScaleUcharLinex2Ch3(pInLine, pOutLine, nWidth);
		return;
	}
	unsigned short i, x, Y;
	unsigned char *pIn[6];
	pIn[0] = pIn[1] = pIn[2] = pInLine;
	pIn[3] = pIn[2] + nChannel;
	pIn[4] = pIn[3] + nChannel;
	pIn[5] = pIn[4] + nChannel;
	for (x = 0; x < nWidth - 3; x += 2)
	{
		i = 0;
#ifdef USE_NEON
		uint16x8_t vY;
		for (; i < (nChannel + 7) / 8 * 8; i += 8)
		{
			vY = vaddl_u8(vld1_u8(pIn[0] + i), vld1_u8(pIn[5] + i));
			vY = vmlaq_n_u16(vY, vaddl_u8(vld1_u8(pIn[1] + i), vld1_u8(pIn[4] + i)), 5);
			vY = vmlaq_n_u16(vY, vaddl_u8(vld1_u8(pIn[2] + i), vld1_u8(pIn[3] + i)), 10);
			vst1q_u16(pOutLine + i, vY);
			//pOutLine[i] = (unsigned short)Y;
		}
#endif
		for (; i < nChannel; i++)
		{
			Y = (unsigned short)pIn[0][i];
			Y += (unsigned short)pIn[1][i] * 5;
			Y += (unsigned short)pIn[2][i] * 10;
			Y += (unsigned short)pIn[3][i] * 10;
			Y += (unsigned short)pIn[4][i] * 5;
			Y += (unsigned short)pIn[5][i];

			pOutLine[i] = (unsigned short)Y;
		}
		pIn[0] = pIn[2];
		pIn[1] = pIn[3];
		pIn[2] = pIn[4];
		pIn[3] = pIn[5];
		pIn[4] += nChannel * 2;
		pIn[5] += nChannel * 2;

		pOutLine += nChannel;
	}
	pIn[4] = pIn[2];
	pIn[5] = pIn[3];
	for (; x < nWidth; x += 2)
	{
		for (i = 0; i < nChannel; i++)
		{
			Y = (unsigned short)pIn[0][i];
			Y += (unsigned short)pIn[1][i] * 5;
			Y += (unsigned short)pIn[2][i] * 10;
			Y += (unsigned short)pIn[3][i] * 10;
			Y += (unsigned short)pIn[4][i] * 5;
			Y += (unsigned short)pIn[5][i];

			pOutLine[i] = (unsigned short)Y;
		}
		pIn[0] = pIn[2];
		pIn[1] = pIn[3];
		pIn[2] = pIn[4];
		pIn[3] = pIn[5];
		pOutLine += nChannel;
	}
}
extern void VDownScaleUcharLinex2(unsigned short *pInLines[], unsigned char *pOutLine, int nWidth, int nChannel)
{
	int i, x, Y;
	unsigned short *pIn[6];
	for (i = 0; i < 6; i++)
	{
		pIn[i] = pInLines[i];
	}
	x = 0;
#ifdef USE_NEON
	uint32x4_t vY;
	uint16x4_t vOutL, vOutH;
	for (; x < (nWidth >> 1)*nChannel / 8 * 8; x += 8)
	{
		vY = vaddl_u16(vld1_u16(pIn[0] + x), vld1_u16(pIn[5] + x));
		vY = vmlaq_n_u32(vY, vaddl_u16(vld1_u16(pIn[1] + x), vld1_u16(pIn[4] + x)), 5);
		vY = vmlaq_n_u32(vY, vaddl_u16(vld1_u16(pIn[2] + x), vld1_u16(pIn[3] + x)), 10);
		vOutL = vshrn_n_u32(vY, 10);
		vY = vaddl_u16(vld1_u16(pIn[0] + x + 4), vld1_u16(pIn[5] + x + 4));
		vY = vmlaq_n_u32(vY, vaddl_u16(vld1_u16(pIn[1] + x + 4), vld1_u16(pIn[4] + x + 4)), 5);
		vY = vmlaq_n_u32(vY, vaddl_u16(vld1_u16(pIn[2] + x + 4), vld1_u16(pIn[3] + x + 4)), 10);
		vOutH = vshrn_n_u32(vY, 10);
		vst1_u8(pOutLine, vmovn_u16(vcombine_u16(vOutL, vOutH)));
		pOutLine += 8;
	}
#endif
	for (; x < (nWidth >> 1)*nChannel; x++)
	{
		Y = (int)pIn[0][x];
		Y += (int)pIn[1][x] * 5;
		Y += (int)pIn[2][x] * 10;
		Y += (int)pIn[3][x] * 10;
		Y += (int)pIn[4][x] * 5;
		Y += (int)pIn[5][x];
		Y >>= 10;
		*(pOutLine++) = (unsigned char)Y;
	}
}
extern bool DownScaleUcharDatax2(unsigned char *pInData, unsigned char *pOutData, int nWidth, int nHeight, int nChannel, bool bDitheringEnable)
{
	int nPitch = (nWidth >> 1)*nChannel + 8;
	int nProcs = omp_get_num_procs();
	unsigned short *pBuffer = new unsigned short[nPitch * 6 * nProcs];
	if (pBuffer == NULL)
	{
		return false;
	}
	unsigned char *pInLine = pInData;
	unsigned short *pVLines[6];
	int   loop = 0;
#pragma omp parallel for firstprivate(loop) private(pVLines)
	for (int y = 0; y < nHeight; y += 2)
	{
		unsigned char *pOutLine = pOutData + (y >> 1)*(nWidth >> 1)*nChannel;;
		int nThreadId = omp_get_thread_num();
		int yy, i;
		if (loop == 0)
		{
			pVLines[0] = pBuffer + 6 * nPitch*nThreadId;
			pVLines[1] = pVLines[0] + nPitch;
			pVLines[2] = pVLines[1] + nPitch;
			pVLines[3] = pVLines[2] + nPitch;
			pVLines[4] = pVLines[3] + nPitch;
			pVLines[5] = pVLines[4] + nPitch;
			if (y == 0)
			{
				HDownScaleUcharLinex2(pInLine, pVLines[0], nWidth, nChannel);
				HDownScaleUcharLinex2(pInLine, pVLines[1], nWidth, nChannel);
			}
			else
			{
				HDownScaleUcharLinex2(pInLine + (y - 2)*nWidth * nChannel, pVLines[0], nWidth, nChannel);
				HDownScaleUcharLinex2(pInLine + (y - 1)*nWidth * nChannel, pVLines[1], nWidth, nChannel);
			}
			HDownScaleUcharLinex2(pInLine + (y + 0)*nWidth * nChannel, pVLines[2], nWidth, nChannel);
			HDownScaleUcharLinex2(pInLine + (y + 1)*nWidth * nChannel, pVLines[3], nWidth, nChannel);
			loop++;
		}
		yy = y + 2;
		if (yy >= nHeight) yy -= 2;
		HDownScaleUcharLinex2(pInLine + yy * nWidth*nChannel, pVLines[4], nWidth, nChannel);
		HDownScaleUcharLinex2(pInLine + (yy + 1)*nWidth*nChannel, pVLines[5], nWidth, nChannel);
		VDownScaleUcharLinex2(pVLines, pOutLine, nWidth, nChannel);
		pOutLine += (nWidth >> 1)*nChannel;
		unsigned short *pTemp0 = pVLines[0];
		unsigned short *pTemp1 = pVLines[1];
		pVLines[0] = pVLines[2];
		pVLines[1] = pVLines[3];
		pVLines[2] = pVLines[4];
		pVLines[3] = pVLines[5];
		pVLines[4] = pTemp0;
		pVLines[5] = pTemp1;
	}
	delete[] pBuffer;
	return true;
}

extern void HDownScaleWordLinex2(unsigned short *pInLine, unsigned int *pOutLine, int nWidth, int nChannel)
{
	unsigned int i, x, Y;
	unsigned short *pIn[6];
	pIn[0] = pIn[1] = pIn[2] = pInLine;
	pIn[3] = pIn[2] + nChannel;
	pIn[4] = pIn[3] + nChannel;
	pIn[5] = pIn[4] + nChannel;
	for (x = 0; x < nWidth - 3; x += 2)
	{
		i = 0;
#ifdef USE_NEON
		uint32x4_t vY;
		for (; i < (nChannel + 3) / 4 * 4; i += 4)
		{
			vY = vaddl_u16(vld1_u16(pIn[0] + i), vld1_u16(pIn[5] + i));
			vY = vmlaq_n_u32(vY, vaddl_u16(vld1_u16(pIn[1] + i), vld1_u16(pIn[4] + i)), 5);
			vY = vmlaq_n_u32(vY, vaddl_u16(vld1_u16(pIn[2] + i), vld1_u16(pIn[3] + i)), 10);
			vst1q_u16(pOutLine + i, vY);
		}
#endif
		for (; i < nChannel; i++)
		{
			Y = (unsigned int)pIn[0][i];
			Y += (unsigned int)pIn[1][i] * 5;
			Y += (unsigned int)pIn[2][i] * 10;
			Y += (unsigned int)pIn[3][i] * 10;
			Y += (unsigned int)pIn[4][i] * 5;
			Y += (unsigned int)pIn[5][i];
			pOutLine[i] = Y;
		}
		pIn[0] = pIn[2];
		pIn[1] = pIn[3];
		pIn[2] = pIn[4];
		pIn[3] = pIn[5];
		pIn[4] += nChannel * 2;
		pIn[5] += nChannel * 2;

		pOutLine += nChannel;
	}
	pIn[4] = pIn[2];
	pIn[5] = pIn[3];
	for (; x < nWidth; x += 2)
	{
		for (i = 0; i < nChannel; i++)
		{
			Y = (unsigned int)pIn[0][i];
			Y += (unsigned int)pIn[1][i] * 5;
			Y += (unsigned int)pIn[2][i] * 10;
			Y += (unsigned int)pIn[3][i] * 10;
			Y += (unsigned int)pIn[4][i] * 5;
			Y += (unsigned int)pIn[5][i];
			pOutLine[i] = Y;
		}
		pIn[0] = pIn[2];
		pIn[1] = pIn[3];
		pIn[2] = pIn[4];
		pIn[3] = pIn[5];
		pOutLine += nChannel;
	}
}
extern void VDownScaleWordLinex2(unsigned int *pInLines[], unsigned short *pOutLine, int nWidth, int nChannel)
{
		unsigned int i, x, Y;
		unsigned int *pIn[6];
		for (i = 0; i < 6; i++)
		{
			pIn[i] = pInLines[i];
		}
		x = 0;
#ifdef USE_NEON
		uint32x4_t vY;
		uint16x4_t vOutL, vOutH;
		for (; x < (nWidth >> 1)*nChannel - 3; x += 4)
		{
			vY = vaddq_u32(vld1q_u32(pIn[0] + x), vld1q_u32(pIn[5] + x));
			vY = vmlaq_n_u32(vY, vaddq_u32(vld1q_u32(pIn[1] + x), vld1q_u32(pIn[4] + x)), 5);
			vY = vmlaq_n_u32(vY, vaddq_u32(vld1q_u32(pIn[2] + x), vld1q_u32(pIn[3] + x)), 10);
			vOutL = vshrn_n_u32(vY, 10);
			vst1_u16(pOutLine, vOutL);
			pOutLine += 4;
		}
#endif
		for (; x < (nWidth >> 1)*nChannel; x++)
		{
			Y = (int)pIn[0][x];
			Y += (int)pIn[1][x] * 5;
			Y += (int)pIn[2][x] * 10;
			Y += (int)pIn[3][x] * 10;
			Y += (int)pIn[4][x] * 5;
			Y += (int)pIn[5][x];
			Y >>= 10;
			*(pOutLine++) = (short)Y;
		}
}
extern bool DownScaleWordDatax2(unsigned short *pInData, unsigned short *pOutData, int nWidth, int nHeight, int nChannel, bool bDitheringEnable)
{
	int nPitch = (nWidth >> 1)*nChannel + 8;
	int nProcs = omp_get_num_procs();
	unsigned int *pBuffer = new unsigned int[nPitch * 6 * nProcs];
	if (pBuffer == NULL)
	{
		return false;
	}
	unsigned short *pInLine = pInData;
	unsigned int *pVLines[6];
	int   loop = 0;
#pragma omp parallel for firstprivate(loop) private(pVLines)
	for (int y = 0; y < nHeight; y += 2)
	{
		unsigned short *pOutLine = pOutData + (y >> 1)*(nWidth >> 1)*nChannel;;
		int nThreadId = omp_get_thread_num();
		int yy, i;
		if (loop == 0)
		{
			pVLines[0] = pBuffer + 6 * nPitch*nThreadId;
			pVLines[1] = pVLines[0] + nPitch;
			pVLines[2] = pVLines[1] + nPitch;
			pVLines[3] = pVLines[2] + nPitch;
			pVLines[4] = pVLines[3] + nPitch;
			pVLines[5] = pVLines[4] + nPitch;
			if (y == 0)
			{
				HDownScaleWordLinex2(pInLine, pVLines[0], nWidth, nChannel);
				HDownScaleWordLinex2(pInLine, pVLines[1], nWidth, nChannel);
			}
			else
			{
				HDownScaleWordLinex2(pInLine + (y - 2)*nWidth * nChannel, pVLines[0], nWidth, nChannel);
				HDownScaleWordLinex2(pInLine + (y - 1)*nWidth * nChannel, pVLines[1], nWidth, nChannel);
			}
			HDownScaleWordLinex2(pInLine + (y + 0)*nWidth * nChannel, pVLines[2], nWidth, nChannel);
			HDownScaleWordLinex2(pInLine + (y + 1)*nWidth * nChannel, pVLines[3], nWidth, nChannel);
			loop++;
		}

		yy = y + 2;
		if (yy >= nHeight) yy -= 2;
		HDownScaleWordLinex2(pInLine + yy * nWidth*nChannel, pVLines[4], nWidth, nChannel);
		HDownScaleWordLinex2(pInLine + (yy + 1)*nWidth*nChannel, pVLines[5], nWidth, nChannel);
		VDownScaleWordLinex2(pVLines, pOutLine, nWidth, nChannel);
		pOutLine += (nWidth >> 1)*nChannel;
		unsigned int *pTemp0 = pVLines[0];
		unsigned int *pTemp1 = pVLines[1];
		pVLines[0] = pVLines[2];
		pVLines[1] = pVLines[3];
		pVLines[2] = pVLines[4];
		pVLines[3] = pVLines[5];
		pVLines[4] = pTemp0;
		pVLines[5] = pTemp1;
	}
	delete[] pBuffer;
	return true;
}
extern void HUpScaleUcharLinex2(unsigned char *pInLine, unsigned short *pOutLine, int nWidth, int nChannel)
{
	if (nChannel == 1)
	{
		int i, x;
		unsigned short Y;
		unsigned char *pIn;
		pIn = pInLine;
		Y = (unsigned short)pIn[0] << 2;
		pOutLine[0] = (unsigned short)Y;
		pOutLine += 1;
		x = 0;
#ifdef USE_NEON
		uint8x8_t vData0 = vld1_u8(&pIn[0]);
		uint8x8_t vData1 = vld1_u8(&pIn[8]);
		for (; x < nWidth - 1 - 7; x += 8)
		{
			uint8x8_t vIn0 = vData0;
			uint8x8_t vIn1 = vext_u8(vData0, vData1, 1);
			pIn += 8;
			vData0 = vData1;
			vData1 = vld1_u8(&pIn[8]);
			uint16x8x2_t vOut;
			uint16x8_t vTmp = vaddl_u8(vIn0, vIn1);
			vOut.val[0] = vaddw_u8(vaddw_u8(vTmp, vIn0), vIn0);
			vOut.val[1] = vaddw_u8(vaddw_u8(vTmp, vIn1), vIn1);
			vst2q_u16(pOutLine, vOut);
			pOutLine += 2 * 8;
		}
#endif
		for (; x < nWidth - 1; x++)
		{
			Y = (unsigned short)pIn[0] * 3;
			Y += (unsigned short)pIn[1];
			pOutLine[0] = Y;

			Y = (unsigned short)pIn[0];
			Y += (unsigned short)pIn[1] * 3;
			pOutLine[1] = Y;

			pOutLine += 2;
			pIn++;
		}
		Y = (unsigned short)pIn[0] << 2;
		pOutLine[0] = (unsigned short)Y;
		return;
	}
	int i, x;
	unsigned short Y;
	unsigned char *pIn[2];
	pIn[0] = pInLine;
	pIn[1] = pInLine + nChannel;
	for (i = 0; i < nChannel; i++)
	{
		Y = (unsigned short)pIn[0][i] << 2;
		pOutLine[i] = (unsigned short)Y;
	}
	pOutLine += nChannel;
	for (x = 0; x < nWidth - 1; x++)
	{
		for (i = 0; i < nChannel; i++)
		{
			Y = (unsigned short)pIn[0][i] * 3;
			Y += (unsigned short)pIn[1][i];
			pOutLine[i] = Y;

			Y = (unsigned short)pIn[0][i];
			Y += (unsigned short)pIn[1][i] * 3;
			pOutLine[i + nChannel] = Y;
		}
		pOutLine += nChannel * 2;
		pIn[0] = pIn[1];
		pIn[1] += nChannel;
	}
	for (i = 0; i < nChannel; i++)
	{
		Y = (unsigned short)pIn[0][i] << 2;
		pOutLine[i] = (unsigned short)Y;
	}
}
extern void VUpScaleUcharLinex2(unsigned short *pInLines[], unsigned char *pOutLine, int nLen)
{
	int i, x;
	unsigned short *pIn[2];
	unsigned char *pOutLine0 = pOutLine;
	unsigned char *pOutLine1 = pOutLine + nLen;
	for (i = 0; i < 2; i++)
	{
		pIn[i] = pInLines[i];
	}
	x = 0;
#ifdef USE_NEON
	for (; x < nLen - 7; x += 8)
	{
		uint16x8_t vIn0 = vld1q_u16(&pIn[0][x]);
		uint16x8_t vIn1 = vld1q_u16(&pIn[1][x]);
		uint16x8_t vY0 = vmlaq_n_u16(vIn1, vIn0, 3);
		vst1_u8(pOutLine0, vshrn_n_u16(vY0, 4));
		uint16x8_t vY1 = vmlaq_n_u16(vIn0, vIn1, 3);
		vst1_u8(pOutLine1, vshrn_n_u16(vY1, 4));
		pOutLine0 += 8;
		pOutLine1 += 8;
	}
#endif
	for (; x < nLen; x++)
	{
		unsigned short Y0 = (pIn[0][x] * 3 + pIn[1][x]) >> 4;
		unsigned short Y1 = (pIn[0][x] + pIn[1][x] * 3) >> 4;
		*(pOutLine0++) = (unsigned char)Y0;
		*(pOutLine1++) = (unsigned char)Y1;
	}
}
extern bool UpScaleUcharDatax2(unsigned char *pInData, unsigned char *pOutData, int nWidth, int nHeight, int nChannel, bool bDitheringEnable)
{
	int nPitch = nWidth * 2 * nChannel;
	int nProcs = omp_get_num_procs();
	unsigned short *pBuffers = new unsigned short[nPitch * 2 * nProcs];
	if (pBuffers == NULL)
	{
		return false;
	}
	unsigned short *pVLines[2];
	int loop = 0;
	#pragma omp parallel for firstprivate(loop) private(pVLines)
	for (int y = 0; y < nHeight - 1; y++)
	{
		unsigned char *pInLine = pInData + (y + 1)*nWidth*nChannel;
		unsigned char *pOutLine = pOutData + nPitch + y * 2 * nPitch;
		if (loop == 0)
		{
			int nThreadId = omp_get_thread_num();
			pVLines[0] = pBuffers + nThreadId * (nPitch * 2);
			pVLines[1] = pBuffers + nThreadId * (nPitch * 2) + nPitch;
			HUpScaleUcharLinex2(pInLine - nWidth * nChannel, pVLines[0], nWidth, nChannel);
			if (y == 0)
			{
				unsigned char *tmppOutLine = pOutData;
				for (int x = 0; x < (nWidth * 2 * nChannel); x++)
				{
					int Y = pVLines[0][x] >> 2;
					*(tmppOutLine++) = (unsigned char)Y;
				}
			}
			loop++;
		}
		HUpScaleUcharLinex2(pInLine, pVLines[1], nWidth, nChannel);
		VUpScaleUcharLinex2(pVLines, pOutLine, nPitch);
		unsigned short *pTemp = pVLines[0];
		pVLines[0] = pVLines[1];
		pVLines[1] = pTemp;
		if (y == (nHeight - 2))
		{
			unsigned char *ptmpOut = pOutLine + 2 * nPitch;
			for (int x = 0; x < (nWidth * 2 * nChannel); x++)
			{
				int Y = pVLines[0][x] >> 2;
				*(ptmpOut++) = (unsigned char)Y;
			}
		}
	}
	delete[] pBuffers;
	return true;
}
extern void HUpScaleWordLinex2(unsigned short *pInLine, unsigned int *pOutLine, int nWidth, int nChannel)
{
	unsigned int i, x;
	unsigned int Y;
	unsigned short *pIn[2];
	if (nWidth == 1)
	{
		for (i = 0; i < nChannel; i++)
		{
			Y = pInLine[i];
			Y <<= 2;
			pOutLine[i] = Y;
		}
		pOutLine += nChannel;
		for (i = 0; i < nChannel; i++)
		{
			Y = pInLine[i];
			Y <<= 2;
			pOutLine[i] = Y;
		}
		pOutLine += nChannel;
	}
	else
	{
		pIn[0] = pIn[1] = pInLine;
		if (nChannel == 1)
		{
			x = 0;
			for (; x < nWidth - 1; x++)
			{
				*pOutLine++ = pIn[0][0] + (pIn[1][0]<<1)+ pIn[1][0];
				pIn[0] = pIn[1];
				pIn[1] += 1;
				*pOutLine++ = (pIn[0][0]<<1)+ pIn[0][0] + pIn[1][0];
			}
		}
		else
		{
			for (x = 0; x < nWidth - 1; x++)
			{
				for (i = 0; i < nChannel; i++)
				{
					Y = pIn[0][i];
					Y += pIn[1][i] * 3;
					pOutLine[i] = Y;
				}
				pOutLine += nChannel;
				pIn[0] = pIn[1];
				pIn[1] += nChannel;

				for (i = 0; i < nChannel; i++)
				{
					Y = pIn[0][i] * 3;
					Y += pIn[1][i];
					pOutLine[i] = Y;
				}
				pOutLine += nChannel;
			}
		}
		for (; x < nWidth; x++)
		{
			for (i = 0; i < nChannel; i++)
			{
				Y = pIn[0][i]+ (pIn[1][i]<<1)+ pIn[1][i];
				pOutLine[i] = Y;
			}
			pOutLine += nChannel;
			pIn[0] = pIn[1];
			for (i = 0; i < nChannel; i++)
			{
				Y = (pIn[0][i]<<1)+ pIn[0][i]+ pIn[1][i];
				pOutLine[i] = Y;
			}
			pOutLine += nChannel;
		}
	}
}
extern void VUpScaleWordLinex2(unsigned int *pInLines[], unsigned short *pOutLine, int bYFlag, int nWidth, int nChannel)
{
		int  x;
		int nLen = nWidth * 2 * nChannel;
		unsigned int *pIn[2];
		pIn[0] = pInLines[0];
		pIn[1] = pInLines[1];
		if (bYFlag == 0)
		{
			x = 0;
			for (; x < nLen - 4; x += 4)
			{
				*(pOutLine++) = (unsigned int)((pIn[0][0] + (pIn[1][0]<<1)+ pIn[1][0])>>4);
				*(pOutLine++) = (unsigned int)((pIn[0][1] + (pIn[1][1] << 1)+ pIn[1][1]) >> 4);
				*(pOutLine++) = (unsigned int)((pIn[0][2] + (pIn[1][2] << 1)+ pIn[1][2]) >> 4);
				*(pOutLine++) = (unsigned int)((pIn[0][3] + (pIn[1][3] << 1)+ pIn[1][3]) >> 4);
				pIn[0] += 4;
				pIn[1] += 4;
			}
			for (; x < nLen; x++)
			{
				*(pOutLine++) = (unsigned int)((pIn[0][0] + (pIn[1][0] << 1) + pIn[1][0]) >> 4);
				pIn[0]++;
				pIn[1]++;
			}
		}
		else
		{
			x = 0;
			for (; x < nLen - 4; x += 4)
			{
				*(pOutLine++) = (unsigned int)((pIn[1][0] + (pIn[0][0] << 1) + pIn[0][0]) >> 4);
				*(pOutLine++) = (unsigned int)((pIn[1][1] + (pIn[0][1] << 1) + pIn[0][1]) >> 4);
				*(pOutLine++) = (unsigned int)((pIn[1][2] + (pIn[0][2] << 1) + pIn[0][2]) >> 4);
				*(pOutLine++) = (unsigned int)((pIn[1][3] + (pIn[0][3] << 1) + pIn[0][3]) >> 4);
				pIn[0] += 4;
				pIn[1] += 4;
			}
			for (; x < nLen; x++)
			{
				*(pOutLine++) = (unsigned int)((pIn[1][0] + (pIn[0][0] << 1) + pIn[0][0]) >> 4);
				pIn[0]++;
				pIn[1]++;
			}
		}
}
extern bool UpScaleWordDatax2(unsigned short *pInData, unsigned short *pOutData, int nWidth, int nHeight, int nChannel)
{
	int nPitch = nWidth * 2 * nChannel;
	int nProcs = omp_get_num_procs();
	unsigned int *pBuffer = new unsigned int[nPitch * 2 * nProcs];
	if (pBuffer == NULL)
	{
		return false;
	}
	unsigned int *pVLines[2];
	unsigned short *pInLine = pInData;
	int   loop = 0;
#pragma omp parallel for firstprivate(loop) private(pVLines)
	for (int y = 0; y < nHeight; y++)
	{
		unsigned short *pOutLine = pOutData + (y << 1)*nPitch;
		int nThreadId = omp_get_thread_num();
		int yy, i;
		if (loop == 0)
		{
			pVLines[0] = pBuffer + 2 * nPitch*nThreadId;
			pVLines[1] = pVLines[0] + nPitch;
			if (y == 0)
			{
				HUpScaleWordLinex2(pInLine, pVLines[0], nWidth, nChannel);
			}
			else
			{
				HUpScaleWordLinex2(pInLine + (y - 1)*nWidth * nChannel, pVLines[0], nWidth, nChannel);
			}
			loop++;
		}
		HUpScaleWordLinex2(pInLine + y * nWidth*nChannel, pVLines[1], nWidth, nChannel);
		VUpScaleWordLinex2(pVLines, pOutLine, 0, nWidth, nChannel);
		yy = y + 1;
		if (yy >= nHeight) yy -= 1;
		unsigned int *pTemp0 = pVLines[0];
		pVLines[0] = pVLines[1];
		pVLines[1] = pTemp0;
		HUpScaleWordLinex2(pInLine + yy * nWidth*nChannel, pVLines[1], nWidth, nChannel);
		pOutLine += nPitch;
		VUpScaleWordLinex2(pVLines, pOutLine, 1, nWidth, nChannel);
	}
	delete[] pBuffer;
	return true;
}
extern void AddBackUcharEdge(unsigned char *pInData, short *pEdgeData, unsigned char *pOutData, int nWidth, int nHeight, int nEdgeWidth, int nEdgeHeight, int nChannel)
{
	int i, x, y, Y;
	unsigned char *pIn = pInData;
	short *pE = pEdgeData;
	unsigned char *pOut = pOutData;
	for (y = 0; y < nEdgeHeight; y++)
	{
		for (x = 0; x < nEdgeWidth; x++)
		{
			for (i = 0; i < nChannel; i++)
			{
				Y = *(pIn++);
				Y += *(pE++);
				if (Y < 0)Y = 0;	if (Y > 255)Y = 255;
				*(pOut++) = (unsigned char)Y;
			}
		}
		pIn += (nWidth - nEdgeWidth)*nChannel;
	}
}
extern void AddBackWordEdge(unsigned short *pInImage, short *pEdgeImage, unsigned short *pOutImage, int nWidth, int nHeight, int nEdgeWidth, int nEdgeHeight, int nChannel)
{
	int i, x, y, Y;

	unsigned short *pIn = pInImage;
	short *pE = pEdgeImage;
	unsigned short *pOut = pOutImage;
	for (y = 0; y < nEdgeHeight; y++)
	{
		for (x = 0; x < nEdgeWidth; x++)
		{
			for (i = 0; i < nChannel; i++)
			{
				Y = *(pIn++);
				Y += *(pE++);
				if (Y < 0)Y = 0;	if (Y > 65535)Y = 65535;
				*(pOut++) = (unsigned short)Y;
			}
		}
		pIn += (nWidth - nEdgeWidth)*nChannel;
	}
}
#define SHIFT_NUM 14
extern inline void CalCubicCoef(double f, short *pCoefs)
{
	double f2 = f * f;
	double f3 = f2 * f;
	int scale = 1 << SHIFT_NUM;
	pCoefs[0] = (short)((-f3 / 6 + f2 / 2 - f / 3) * scale);
	pCoefs[1] = (short)((f3 / 2 - f2 - f / 2 + 1) * scale);
	pCoefs[2] = (short)((-f3 / 2 + f2 / 2 + f) * scale);
	pCoefs[3] = (short)((f3 / 6 - f / 6) * scale);
}
#if defined(USE_NEON) && defined( __DITHERING_)
#define V_FILTER(vOut0, vOut1, vPix0, vPix1, vPix2, vPix3, vE) \
    temp =  vreinterpretq_s16_u16(vmovl_u8(vPix0)); \
	vOut0 = vmull_s16(vget_low_s16(temp), vdup_n_s16(f[0])); \
    vOut1 = vmull_s16(vget_high_s16(temp), vdup_n_s16(f[0])); \
	temp = vreinterpretq_s16_u16(vmovl_u8(vPix1)); \
	vOut0 = vmlal_s16(vOut0, vget_low_s16(temp), vdup_n_s16(f[1])); \
	vOut1 = vmlal_s16(vOut1, vget_high_s16(temp), vdup_n_s16(f[1])); \
	temp = vreinterpretq_s16_u16(vmovl_u8(vPix2)); \
	vOut0 = vmlal_s16(vOut0, vget_low_s16(temp), vdup_n_s16(f[2])); \
	vOut1 = vmlal_s16(vOut1, vget_high_s16(temp), vdup_n_s16(f[2])); \
	temp = vreinterpretq_s16_u16(vmovl_u8(vPix3)); \
	vOut0 = vmlal_s16(vOut0, vget_low_s16(temp), vdup_n_s16(f[3])); \
	vOut1 = vmlal_s16(vOut1, vget_high_s16(temp), vdup_n_s16(f[3])); \
	vOut0 = vmaxq_s32(vOut0, vdupq_n_s32(0)); \
	vOut1 = vmaxq_s32(vOut1, vdupq_n_s32(0)); \
	vOut0 = vaddq_s32(vOut0, vE); \
	vE = vandq_s32(vOut0, vdupq_n_s32((1 << SHIFT_NUM) - 1)); \
	vOut1 = vaddq_s32(vOut1, vE); \
	vE = vandq_s32(vOut1, vdupq_n_s32((1 << SHIFT_NUM) - 1)); \
	vOut0 = vshrq_n_s32(vOut0, SHIFT_NUM); \
	vOut1 = vshrq_n_s32(vOut1, SHIFT_NUM); \
	vOut0 = vminq_s32(vOut0, vdupq_n_s32(255)); \
	vOut1 = vminq_s32(vOut1, vdupq_n_s32(255)); \

#define H_FILTER(v0, v1, vE, vPix0, vPix1, vPix2, vPix3, f) \
    v16Pix0 = vreinterpret_s16_u16(vget_low_u16(vmovl_u8(vPix0))); \
	v16Pix1 = vreinterpret_s16_u16(vget_low_u16(vmovl_u8(vPix2))); \
	vTemp = vzipq_s32(vmull_s16(v16Pix0, vld1_s16(f)), vmull_s16(v16Pix1, vld1_s16(f + 8))); \
	v0 = vaddq_s32(vTemp.val[0], vTemp.val[1]); \
	v16Pix0 = vreinterpret_s16_u16(vget_low_u16(vmovl_u8(vPix1))); \
	v16Pix1 = vreinterpret_s16_u16(vget_low_u16(vmovl_u8(vPix3))); \
	vTemp = vzipq_s32(vmull_s16(v16Pix0, vld1_s16(f + 4)), vmull_s16(v16Pix1, vld1_s16(f + 12))); \
	v1 = vaddq_s32(vTemp.val[0], vTemp.val[1]); \
	vTemp = vzipq_s32(v0, v1); \
	v0 = vaddq_s32(vTemp.val[0], vTemp.val[1]); \
	v0 = vmaxq_s32(v0, vdupq_n_s32(0)); \
	v0 = vaddq_s32(v0, vE); \
	vE = vandq_s32(v0, vdupq_n_s32((1 << SHIFT_NUM) - 1)); \
	v0 = vshrq_n_s32(v0, SHIFT_NUM); \
	v0 = vminq_s32(v0, vdupq_n_s32(255)); \

#endif

#if defined(USE_NEON) && !defined( __DITHERING_)
#define V_FILTER(vOut0, vOut1, vPix0, vPix1, vPix2, vPix3, vE) \
    temp =  vreinterpretq_s16_u16(vmovl_u8(vPix0)); \
	vOut0 = vmull_s16(vget_low_s16(temp), vdup_n_s16(f[0])); \
    vOut1 = vmull_s16(vget_high_s16(temp), vdup_n_s16(f[0])); \
	temp = vreinterpretq_s16_u16(vmovl_u8(vPix1)); \
	vOut0 = vmlal_s16(vOut0, vget_low_s16(temp), vdup_n_s16(f[1])); \
	vOut1 = vmlal_s16(vOut1, vget_high_s16(temp), vdup_n_s16(f[1])); \
	temp = vreinterpretq_s16_u16(vmovl_u8(vPix2)); \
	vOut0 = vmlal_s16(vOut0, vget_low_s16(temp), vdup_n_s16(f[2])); \
	vOut1 = vmlal_s16(vOut1, vget_high_s16(temp), vdup_n_s16(f[2])); \
	temp = vreinterpretq_s16_u16(vmovl_u8(vPix3)); \
	vOut0 = vmlal_s16(vOut0, vget_low_s16(temp), vdup_n_s16(f[3])); \
	vOut1 = vmlal_s16(vOut1, vget_high_s16(temp), vdup_n_s16(f[3])); \
	vOut0 = vmaxq_s32(vOut0, vdupq_n_s32(0)); \
	vOut1 = vmaxq_s32(vOut1, vdupq_n_s32(0)); \
	vOut0 = vaddq_s32(vOut0, vdupq_n_s32( 1 << (SHIFT_NUM-1) )); \
	vOut1 = vaddq_s32(vOut1, vdupq_n_s32(1 << (SHIFT_NUM - 1))); \
	vOut0 = vshrq_n_s32(vOut0, SHIFT_NUM); \
	vOut1 = vshrq_n_s32(vOut1, SHIFT_NUM); \
	vOut0 = vminq_s32(vOut0, vdupq_n_s32(255)); \
	vOut1 = vminq_s32(vOut1, vdupq_n_s32(255)); \

#define H_FILTER(v0, v1, vE, vPix0, vPix1, vPix2, vPix3, f) \
    v16Pix0 = vreinterpret_s16_u16(vget_low_u16(vmovl_u8(vPix0))); \
	v16Pix1 = vreinterpret_s16_u16(vget_low_u16(vmovl_u8(vPix2))); \
	vTemp = vzipq_s32(vmull_s16(v16Pix0, vld1_s16(f)), vmull_s16(v16Pix1, vld1_s16(f + 8))); \
	v0 = vaddq_s32(vTemp.val[0], vTemp.val[1]); \
	v16Pix0 = vreinterpret_s16_u16(vget_low_u16(vmovl_u8(vPix1))); \
	v16Pix1 = vreinterpret_s16_u16(vget_low_u16(vmovl_u8(vPix3))); \
	vTemp = vzipq_s32(vmull_s16(v16Pix0, vld1_s16(f + 4)), vmull_s16(v16Pix1, vld1_s16(f + 12))); \
	v1 = vaddq_s32(vTemp.val[0], vTemp.val[1]); \
	vTemp = vzipq_s32(v0, v1); \
	v0 = vaddq_s32(vTemp.val[0], vTemp.val[1]); \
	v0 = vmaxq_s32(v0, vdupq_n_s32(0)); \
	v0 = vaddq_s32(v0,  vdupq_n_s32( 1 << (SHIFT_NUM-1) )); \
	v0 = vshrq_n_s32(v0, SHIFT_NUM); \
	v0 = vminq_s32(v0, vdupq_n_s32(255)); \

#endif
extern void VReScaleLine(unsigned char *pInLines[], unsigned char *pOutLine, int nChannel, int nWidth, short *f)
{
	int i, j, x, Out[3], E[3];
	unsigned char *pIn[4];
	for (i = 0; i < 4; i++)
	{
		pIn[i] = pInLines[i];
	}
	x = 0;
#ifdef USE_NEON	
	int16x8_t temp;
	int32x4_t vOut0, vOut1;
	if (nChannel == 1)
	{
		int32x4_t vE = vdupq_n_s32(0);
		for (; x < (nWidth / 8) * 8; x += 8)
		{
			uint8x8_t vPix0 = vld1_u8(pIn[0] + x);
			uint8x8_t vPix1 = vld1_u8(pIn[1] + x);
			uint8x8_t vPix2 = vld1_u8(pIn[2] + x);
			uint8x8_t vPix3 = vld1_u8(pIn[3] + x);
			V_FILTER(vOut0, vOut1, vPix0, vPix1, vPix2, vPix3, vE);
			uint8x8_t vOut = vmovn_u16(vreinterpretq_u16_s16(vcombine_s16(vmovn_s32(vOut0), vmovn_s32(vOut1))));
			vst1_u8(pOutLine, vOut);
			pOutLine += 8;
		}
	}
	else if (nChannel == 2)
	{
		int32x4_t vE0 = vdupq_n_s32(0);
		int32x4_t vE1 = vdupq_n_s32(0);
		for (; x < (nWidth / 8) * 8; x += 8)
		{
			uint8x8x2_t vPix0 = vld2_u8(pIn[0] + 2 * x);
			uint8x8x2_t vPix1 = vld2_u8(pIn[1] + 2 * x);
			uint8x8x2_t vPix2 = vld2_u8(pIn[2] + 2 * x);
			uint8x8x2_t vPix3 = vld2_u8(pIn[3] + 2 * x);
			uint8x8x2_t vOut;
			V_FILTER(vOut0, vOut1, vPix0.val[0], vPix1.val[0], vPix2.val[0], vPix3.val[0], vE0);
			vOut.val[0] = vmovn_u16(vreinterpretq_u16_s16(vcombine_s16(vmovn_s32(vOut0), vmovn_s32(vOut1))));
			V_FILTER(vOut0, vOut1, vPix0.val[1], vPix1.val[1], vPix2.val[1], vPix3.val[1], vE1);
			vOut.val[1] = vmovn_u16(vreinterpretq_u16_s16(vcombine_s16(vmovn_s32(vOut0), vmovn_s32(vOut1))));
			vst2_u8(pOutLine, vOut);
			pOutLine += 16;
		}
	}
	else if (nChannel == 3)
	{
		int32x4_t vE0 = vdupq_n_s32(0);
		int32x4_t vE1 = vdupq_n_s32(0);
		int32x4_t vE2 = vdupq_n_s32(0);
		for (; x < (nWidth / 8) * 8; x += 8)
		{
			uint8x8x3_t vPix0 = vld3_u8(pIn[0] + 3 * x);
			uint8x8x3_t vPix1 = vld3_u8(pIn[1] + 3 * x);
			uint8x8x3_t vPix2 = vld3_u8(pIn[2] + 3 * x);
			uint8x8x3_t vPix3 = vld3_u8(pIn[3] + 3 * x);
			uint8x8x3_t vOut;
			V_FILTER(vOut0, vOut1, vPix0.val[0], vPix1.val[0], vPix2.val[0], vPix3.val[0], vE0);
			vOut.val[0] = vmovn_u16(vreinterpretq_u16_s16(vcombine_s16(vmovn_s32(vOut0), vmovn_s32(vOut1))));
			V_FILTER(vOut0, vOut1, vPix0.val[1], vPix1.val[1], vPix2.val[1], vPix3.val[1], vE1);
			vOut.val[1] = vmovn_u16(vreinterpretq_u16_s16(vcombine_s16(vmovn_s32(vOut0), vmovn_s32(vOut1))));
			V_FILTER(vOut0, vOut1, vPix0.val[2], vPix1.val[2], vPix2.val[2], vPix3.val[2], vE2);
			vOut.val[2] = vmovn_u16(vreinterpretq_u16_s16(vcombine_s16(vmovn_s32(vOut0), vmovn_s32(vOut1))));
			vst3_u8(pOutLine, vOut);
			pOutLine += 24;
		}
	}
#endif
	E[0] = E[1] = E[2] = 0;
	for (; x < nWidth; x++)
	{
		for (j = 0; j < nChannel; j++)
		{
			Out[j] = (int)(pIn[0][nChannel*x + j] * f[0] + pIn[1][nChannel*x + j] * f[1] + pIn[2][nChannel*x + j] * f[2] + pIn[3][nChannel*x + j] * f[3]);
			if (Out[j] < 0)Out[j] = 0;
#ifdef __DITHERING_
			Out[j] += E[j];
			E[j] = Out[j] & ((1 << SHIFT_NUM) - 1);
#else
			Out[j] += (1 << (SHIFT_NUM - 1));
#endif
			Out[j] >>= SHIFT_NUM;
			if (Out[j] > 255)Out[j] = 255;
			*(pOutLine++) = (unsigned char)Out[j];
		}
	}
}
extern void HReScaleLine(unsigned char *pInLine, unsigned char *pOutLine, int nChannel, int nInWidth, int nOutWidth,int *pnInXs, short *pCubicCoefX)
{
	int j, nInX, nOutX, Out[3], E[3];
	short *f;
	nOutX = 0;
#ifdef USE_NEON
	int32x4_t vOut0, vOut1, vOut2, vOut3;
	int16x4_t v16Pix0, v16Pix1;
	int32x4x2_t vTemp;
	if (nChannel == 1)
	{
		uint8x8_t vPix0, vPix1, vPix2, vPix3;
		int32x4_t vE = vdupq_n_s32(0);
		for (; nOutX < (nOutWidth / 8) * 8; nOutX += 8)
		{
			f = pCubicCoefX + 4 * nOutX;
			vPix0 = vld1_u8(pInLine + pnInXs[nOutX + 0] - 1);
			vPix1 = vld1_u8(pInLine + pnInXs[nOutX + 1] - 1);
			vPix2 = vld1_u8(pInLine + pnInXs[nOutX + 2] - 1);
			vPix3 = vld1_u8(pInLine + pnInXs[nOutX + 3] - 1);
			H_FILTER(vOut0, vOut1, vE, vPix0, vPix1, vPix2, vPix3, f);
			vPix0 = vld1_u8(pInLine + pnInXs[nOutX + 4] - 1);
			vPix1 = vld1_u8(pInLine + pnInXs[nOutX + 5] - 1);
			vPix2 = vld1_u8(pInLine + pnInXs[nOutX + 6] - 1);
			vPix3 = vld1_u8(pInLine + pnInXs[nOutX + 7] - 1);
			H_FILTER(vOut2, vOut3, vE, vPix0, vPix1, vPix2, vPix3, f + 16);
			vst1_u8(pOutLine, vmovn_u16(vreinterpretq_u16_s16(vcombine_s16(vmovn_s32(vOut0), vmovn_s32(vOut2)))));
			pOutLine += 8;
		}
	}
	else if (nChannel == 2)
	{
		uint8x8x2_t vPix0, vPix1, vPix2, vPix3;
		int32x4_t vE0 = vdupq_n_s32(0);
		int32x4_t vE1 = vdupq_n_s32(0);
		uint8x8x2_t vOut;
		for (; nOutX < (nOutWidth / 8) * 8; nOutX += 8)
		{
			f = pCubicCoefX + 4 * nOutX;
			vPix0 = vld2_u8(pInLine + (pnInXs[nOutX + 0] - 1) * 2);
			vPix1 = vld2_u8(pInLine + (pnInXs[nOutX + 1] - 1) * 2);
			vPix2 = vld2_u8(pInLine + (pnInXs[nOutX + 2] - 1) * 2);
			vPix3 = vld2_u8(pInLine + (pnInXs[nOutX + 3] - 1) * 2);
			H_FILTER(vOut0, vOut1, vE0, vPix0.val[0], vPix1.val[0], vPix2.val[0], vPix3.val[0], f);
			H_FILTER(vOut1, vOut2, vE1, vPix0.val[1], vPix1.val[1], vPix2.val[1], vPix3.val[1], f);
			vPix0 = vld2_u8(pInLine + (pnInXs[nOutX + 4] - 1) * 2);
			vPix1 = vld2_u8(pInLine + (pnInXs[nOutX + 5] - 1) * 2);
			vPix2 = vld2_u8(pInLine + (pnInXs[nOutX + 6] - 1) * 2);
			vPix3 = vld2_u8(pInLine + (pnInXs[nOutX + 7] - 1) * 2);
			H_FILTER(vOut2, vOut3, vE0, vPix0.val[0], vPix1.val[0], vPix2.val[0], vPix3.val[0], f + 16);
			vOut.val[0] = vmovn_u16(vreinterpretq_u16_s16(vcombine_s16(vmovn_s32(vOut0), vmovn_s32(vOut2))));
			H_FILTER(vOut2, vOut3, vE1, vPix0.val[1], vPix1.val[1], vPix2.val[1], vPix3.val[1], f + 16);
			vOut.val[1] = vmovn_u16(vreinterpretq_u16_s16(vcombine_s16(vmovn_s32(vOut1), vmovn_s32(vOut2))));
			vst2_u8(pOutLine, vOut);
			pOutLine += 16;
		}
	}
	else if (nChannel == 3)
	{
		uint8x8x3_t vPix0, vPix1, vPix2, vPix3;
		int32x4_t vE0 = vdupq_n_s32(0);
		int32x4_t vE1 = vdupq_n_s32(0);
		int32x4_t vE2 = vdupq_n_s32(0);
		int32x4_t vOut4;
		uint8x8x3_t vOut;
		for (; nOutX < (nOutWidth / 8) * 8; nOutX += 8)
		{
			f = pCubicCoefX + 4 * nOutX;
			vPix0 = vld3_u8(pInLine + (pnInXs[nOutX + 0] - 1) * 3);
			vPix1 = vld3_u8(pInLine + (pnInXs[nOutX + 1] - 1) * 3);
			vPix2 = vld3_u8(pInLine + (pnInXs[nOutX + 2] - 1) * 3);
			vPix3 = vld3_u8(pInLine + (pnInXs[nOutX + 3] - 1) * 3);
			H_FILTER(vOut0, vOut1, vE0, vPix0.val[0], vPix1.val[0], vPix2.val[0], vPix3.val[0], f);
			H_FILTER(vOut1, vOut2, vE1, vPix0.val[1], vPix1.val[1], vPix2.val[1], vPix3.val[1], f);
			H_FILTER(vOut2, vOut3, vE2, vPix0.val[2], vPix1.val[2], vPix2.val[2], vPix3.val[2], f);
			vPix0 = vld3_u8(pInLine + (pnInXs[nOutX + 4] - 1) * 3);
			vPix1 = vld3_u8(pInLine + (pnInXs[nOutX + 5] - 1) * 3);
			vPix2 = vld3_u8(pInLine + (pnInXs[nOutX + 6] - 1) * 3);
			vPix3 = vld3_u8(pInLine + (pnInXs[nOutX + 7] - 1) * 3);
			H_FILTER(vOut3, vOut4, vE0, vPix0.val[0], vPix1.val[0], vPix2.val[0], vPix3.val[0], f + 16);
			vOut.val[0] = vmovn_u16(vreinterpretq_u16_s16(vcombine_s16(vmovn_s32(vOut0), vmovn_s32(vOut3))));
			H_FILTER(vOut3, vOut4, vE1, vPix0.val[1], vPix1.val[1], vPix2.val[1], vPix3.val[1], f + 16);
			vOut.val[1] = vmovn_u16(vreinterpretq_u16_s16(vcombine_s16(vmovn_s32(vOut1), vmovn_s32(vOut3))));
			H_FILTER(vOut3, vOut4, vE2, vPix0.val[2], vPix1.val[2], vPix2.val[2], vPix3.val[2], f + 16);
			vOut.val[2] = vmovn_u16(vreinterpretq_u16_s16(vcombine_s16(vmovn_s32(vOut2), vmovn_s32(vOut3))));
			vst3_u8(pOutLine, vOut);
			pOutLine += 24;
		}
	}
#endif
	E[0] = E[1] = E[2] = 0;
	for (; nOutX < nOutWidth; nOutX++)
	{
		nInX = pnInXs[nOutX];
		f = pCubicCoefX + 4 * nOutX;
		for (j = 0; j < nChannel; j++)
		{
			Out[j] = (int)(pInLine[(nInX - 1)*nChannel + j] * f[0] + pInLine[(nInX)*nChannel + j] * f[1] + pInLine[(nInX + 1)*nChannel + j] * f[2] + pInLine[(nInX + 2)*nChannel + j] * f[3]);
			if (Out[j] < 0)Out[j] = 0;
#ifdef __DITHERING_
			Out[j] += E[j];
			E[j] = Out[j] & ((1 << SHIFT_NUM) - 1);
#else
			Out[j] += (1 << (SHIFT_NUM - 1));
#endif
			Out[j] >>= SHIFT_NUM;
			if (Out[j] > 255)Out[j] = 255;
			*(pOutLine++) = (unsigned char)Out[j];
		}
	}
}
extern bool RescaleCubicData(unsigned char *pInImage, unsigned char *pOutImage, int nChannel, int nInWidth, int nInHeight, int nOutWidth, int nOutHeight)
{
#define NUM_EXTRA_BYTES 16 // to prevent out of memory
	double fXScale = (double)nInWidth / nOutWidth;
	double fYScale = (double)nInHeight / nOutHeight;
	short  *pCubicCoefX = new short[nOutWidth * 4];
	int *pnInXs = new int[nOutWidth];
	for (int i = 0; i < nOutWidth; i++)
	{
		double f = fXScale / 2.0 - 0.5 + i * fXScale;
		pnInXs[i] = (int)f;
		f = f - (int)f;
		CalCubicCoef(f, pCubicCoefX + 4 * i);
	}
	int nProcs = omp_get_num_procs();
	unsigned char *pBuffer = new unsigned char[((nInWidth + 8) * nChannel + NUM_EXTRA_BYTES)*nProcs];
#pragma omp parallel for schedule(dynamic, 16)
	for (int nOutY = 0; nOutY < nOutHeight; nOutY++)
	{
		short pCubicCoefY[4];
		double f = fYScale / 2.0 - 0.5 + nOutY * fYScale;
		int nInY = (int)f;
		f = f - nInY;
		CalCubicCoef(f, pCubicCoefY);
		unsigned char *pVLines[4];
		for (int i = 0; i < 4; i++)
		{
			int yi = nInY - 1 + i;
			yi = MIN2(nInHeight - 1, MAX2(0, yi));
			pVLines[i] = pInImage + yi * nInWidth * nChannel;
		}
		int nThreadId = omp_get_thread_num();
		unsigned char *pHLine = pBuffer + nThreadId * ((nInWidth + 8) * nChannel + NUM_EXTRA_BYTES);
		VReScaleLine(pVLines, pHLine + 4 * nChannel, nChannel, nInWidth, pCubicCoefY);
		for (int j = 0; j < nChannel; j++)
		{
			pHLine[j] = pHLine[nChannel + j] = pHLine[2 * nChannel + j] = pHLine[3 * nChannel + j] = pHLine[4 * nChannel + j];
			pHLine[(nInWidth + 7)*nChannel + j] = pHLine[(nInWidth + 6)*nChannel + j] = pHLine[(nInWidth + 5)*nChannel + j] = pHLine[(nInWidth + 4)*nChannel + j] = pHLine[(nInWidth + 3)*nChannel + j];
		}
		unsigned char *pOutLine = pOutImage + nOutY * nOutWidth * nChannel;
		HReScaleLine(pHLine + 4 * nChannel, pOutLine, nChannel, nInWidth, nOutWidth, pnInXs, pCubicCoefX);
	}
	delete[] pBuffer;
	delete[] pCubicCoefX;
	delete[] pnInXs;
	return true;
}
bool NonMaximumWindow(float &fX, float &fY, float &max, float fYWin[][7])
{
	int x, y, C;
	max = 0.0;
	for (y = 0; y < 7; y++)
	{
		for (x = 0; x < 7; x++)
		{
			if (max < fYWin[y][x])max = fYWin[y][x];
		}
	}
	if (fYWin[3][3] < max || max == 0)return false;
	fX = fY = 0;
	C = 0;
	for (y = 0; y < 7; y++)
	{
		for (x = 0; x < 7; x++)
		{
			if (fYWin[y][x] == max)
			{
				fX += x;
				fY += y;
				C++;
			}
		}
	}
	if (C > 1)return false;
	fX /= C;
	fY /= C;
	fX -= 3;
	fY -= 3;
	return true;
}
extern void NonMaximumLine(float *pInLines[], int nY, int nWidth, int nPitch, TFeaturePoint *pPtList, int &nPtNum, int nMaxNum)
{
	int i, j, x;
	float *pIn[7];
	float fx, fy, R;
	float fYWin[7][7];
	for (i = 0; i < 7; i++)
	{
		pIn[i] = pInLines[i];
		for (j = 0; j < 6; j++)
		{
			fYWin[i][j] = pIn[i][0];
			pIn[i] += nPitch;
		}
	}
	for (x = 3; x < nWidth - 3; x++)
	{
		for (i = 0; i < 7; i++)
		{
			fYWin[i][6] = pIn[i][0];
			pIn[i] += nPitch;
		}
		if (fYWin[3][3] > 0)
		{
			if (NonMaximumWindow(fx, fy, R, fYWin))
			{
				if (nPtNum < nMaxNum)
				{
					pPtList[nPtNum].fX = (float)x;
					pPtList[nPtNum].fY = (float)nY;
					pPtList[nPtNum].fScore = R;
					pPtList[nPtNum].nFlag = -1;
					pPtList[nPtNum].fX += fx;
					pPtList[nPtNum].fY += fy;
					nPtNum++;
				}
				else
				{
					int mini = 0;
					for (i = 1; i < nMaxNum; i++)
					{
						if (pPtList[i].fScore < pPtList[mini].fScore)
						{
							mini = i;
						}
					}
					if (pPtList[mini].fScore < R)
					{
						pPtList[mini].fX = (float)x;
						pPtList[mini].fY = (float)nY;
						pPtList[mini].fScore = R;
						pPtList[mini].nFlag = -1;

						pPtList[mini].fX += fx;
						pPtList[mini].fY += fy;
					}
				}
			}
		}
		for (i = 0; i < 7; i++)
		{
			for (j = 0; j < 6; j++)
			{
				fYWin[i][j] = fYWin[i][j + 1];
			}
		}
	}
}
bool FastDetectCorner(int nYWin[][7],int nRatio,int nPtThre1,int nPtThre2,int nThre)
{
	int i, k, I0, I[16], T, N, P;
	int nTotalN, nTotalP;
	I0 = nYWin[3][3];
	T = MAX2((((255 - I0)*nRatio + 64) >> 7), nThre);
	I[0] = nYWin[0][3];
	I[1] = nYWin[0][4];
	I[2] = nYWin[1][5];
	I[3] = nYWin[2][6];
	I[4] = nYWin[3][6];
	I[5] = nYWin[4][6];
	I[6] = nYWin[5][5];
	I[7] = nYWin[6][4];
	I[8] = nYWin[6][3];
	I[9] = nYWin[6][2];
	I[10] = nYWin[5][1];
	I[11] = nYWin[4][0];
	I[12] = nYWin[3][0];
	I[13] = nYWin[2][0];
	I[14] = nYWin[1][1];
	I[15] = nYWin[0][2];
	nTotalN = nTotalP = 0;
	for (i = 0; i < 4; i++)
	{
		N = P = 0;
		for (k = i; k <= i + 12; k += 4)
		{
			I[k] -= I0;
			if (I[k] < -T)
			{
				N++;
			}
			else if (I[k] > T)
			{
				P++;
			}
			else
			{
				I[k] = 0;
			}
		}
		if (N < nPtThre1 && P < nPtThre1)return false;
		nTotalN += N;
		nTotalP += P;
	}
	if (nTotalN < nPtThre2 && nTotalP < nPtThre2)return false;
	return true;
}
float ComputeHarrisCorner(int nYWin[][7], float fHarris_K)
{
	int i, x1, y1, gx, gy;
	float sX2, sXY, sY2;
	float det, trace;
	sX2 = 0;
	sXY = 0;
	sY2 = 0;
	for (y1 = 1, i = 0; y1 < 6; y1++)
	{
		for (x1 = 1; x1 < 6; x1++, i++)
		{
			gx = nYWin[y1][x1 + 1] - nYWin[y1][x1 - 1];
			gy = nYWin[y1 + 1][x1] - nYWin[y1 - 1][x1];
			sX2 += gx * gx*nMask[i];
			sXY += gx * gy*nMask[i];
			sY2 += gy * gy*nMask[i];
		}
	}
	sX2 /= 256.0;
	sY2 /= 256.0;
	sXY /= 256.0;
	det = sX2 * sY2 - sXY * sXY;
	trace = sX2 + sY2;
	return det - fHarris_K * (trace*trace);
}
extern void ComputeCornernessLine(unsigned char *pInLines[], float *pOutLine, int nWidth, int nPitch, float fHarris_K,int nHarrisThre, int nRatio, int nPtThre1, int nPtThre2,int nThre)
{
	int i, j, x;
	float R;
	unsigned char *pIn[7];
	int nYWin[7][7];
	for (i = 0; i < 7; i++)
	{
		pIn[i] = pInLines[i];
		for (j = 3; j < 6; j++)
		{
			nYWin[i][j] = pIn[i][0];
			pIn[i] += nPitch;
		}
		for (j = 0; j < 3; j++)
		{
			nYWin[i][j] = nYWin[i][3];
		}
	}
	for (x = 0; x < nWidth - 3; x++)
	{
		for (i = 0; i < 7; i++)
		{
			nYWin[i][6] = pIn[i][0];
			pIn[i] += nPitch;
		}
		if (FastDetectCorner(nYWin, nRatio,nPtThre1, nPtThre2, nThre))
		{
			R = ComputeHarrisCorner(nYWin, fHarris_K);
			if (R < nHarrisThre)R = 0;
			*(pOutLine++) = R;
		}
		else
		{
			*(pOutLine++) = 0;
		}
		for (i = 0; i < 7; i++)
		{
			for (j = 0; j < 6; j++)
			{
				nYWin[i][j] = nYWin[i][j + 1];
			}
		}
	}
	for (; x < nWidth; x++)
	{
		if (FastDetectCorner(nYWin, nRatio, nPtThre1, nPtThre2, nThre))
		{
			R = ComputeHarrisCorner(nYWin, fHarris_K);
			if (R < nHarrisThre)R = 0;
			*(pOutLine++) = R;
		}
		else
		{
			*(pOutLine++) = 0;
		}
		for (i = 0; i < 7; i++)
		{
			for (j = 0; j < 6; j++)
			{
				nYWin[i][j] = nYWin[i][j + 1];
			}
		}
	}
}
extern __inline float ComputeHarrisCorner(unsigned char nYWin[][11], int x, int y,float fHarris_K)
{
	int i, x1, y1, gx, gy, iX2, iXY, iY2;
	float sX2, sXY, sY2;
	float det, trace;
	iX2 = 0;
	iXY = 0;
	iY2 = 0;
#ifdef USE_NEON
	int32x4_t viX2 = vdupq_n_s32(0);
	int32x4_t viXY = vdupq_n_s32(0);
	int32x4_t viY2 = vdupq_n_s32(0);
	int16x4_t vgx, vgy;
	int32x4_t vMask;
	uint8x8_t vDataTbl = vcreate_u8((uint64_t)0x0504030203020100L);
#endif
	for (y1 = 1, i = 0; y1 < 6; y1++)
	{
		x1 = 1;
#ifdef USE_NEON
		//#include <arm_neon.h>
		vMask = vmovl_s16(vld1_s16(nMask + i));
		uint8x8_t vdatIn[4];
		vdatIn[0] = vld1_u8(nYWin[y + y1] + x + x1 - 1);
		vdatIn[1] = vld1_u8(nYWin[y + y1 + 1] + x + x1);
		vdatIn[2] = vld1_u8(nYWin[y + y1 - 1] + x + x1);
		int16x8_t vdat = vreinterpretq_s16_u16(vmovl_u8(vtbl1_u8(vdatIn[0], vDataTbl)));
		vgx = vsub_s16(vget_high_s16(vdat), vget_low_s16(vdat));
		vgy = vreinterpret_s16_u16(vget_low_u16(vsubl_u8(vdatIn[1], vdatIn[2])));

		viX2 = vmlaq_s32(viX2, vmull_s16(vgx, vgx), vMask);
		viXY = vmlaq_s32(viXY, vmull_s16(vgx, vgy), vMask);
		viY2 = vmlaq_s32(viY2, vmull_s16(vgy, vgy), vMask);
		i += 4;
		x1 += 4;
#endif	
		for (; x1 < 6; x1++, i++)
		{
			gx = nYWin[y + y1][x + x1 + 1] - nYWin[y + y1][x + x1 - 1];
			gy = nYWin[y + y1 + 1][x + x1] - nYWin[y + y1 - 1][x + x1];

			iX2 += gx * gx*nMask[i];
			iXY += gx * gy*nMask[i];
			iY2 += gy * gy*nMask[i];
		}
	}
#ifdef USE_NEON
	iX2 += (vgetq_lane_s32(viX2, 0) + vgetq_lane_s32(viX2, 1) + vgetq_lane_s32(viX2, 2) + vgetq_lane_s32(viX2, 3));
	iXY += (vgetq_lane_s32(viXY, 0) + vgetq_lane_s32(viXY, 1) + vgetq_lane_s32(viXY, 2) + vgetq_lane_s32(viXY, 3));
	iY2 += (vgetq_lane_s32(viY2, 0) + vgetq_lane_s32(viY2, 1) + vgetq_lane_s32(viY2, 2) + vgetq_lane_s32(viY2, 3));
#endif
	sX2 = iX2 / 256.0;
	sY2 = iY2 / 256.0;
	sXY = iXY / 256.0;
	det = sX2 * sY2 - sXY * sXY;
	trace = sX2 + sY2;
	return det - fHarris_K * (trace*trace);
}
extern void HAvg3Line(unsigned short *pInLine, unsigned short *pOutLine, int nChannel, int nWidth)
{
	unsigned int S;
	unsigned short *pIn[3];
	const int mul = ((unsigned int)1 << 31) / 3 + 1;
	{
		pIn[0] = pInLine;
		pIn[1] = pInLine;
		pInLine += nChannel;
		int tmpnWidth = nWidth - 1;
		for (int x = 0; x < tmpnWidth; x++)
		{
			pIn[2] = pInLine;
			pInLine += nChannel;
			int i = 0;
			#ifdef USE_NEON
			int tmpnChannel = nChannel / 4 * 4;
			for (; i < tmpnChannel; i += 4)
			{
				uint16x4_t vpIn0 = vld1_u16(&pIn[0][i]);
				uint16x4_t vpIn1 = vld1_u16(&pIn[1][i]);
				uint16x4_t vpIn2 = vld1_u16(&pIn[2][i]);
				uint32x4_t vS = vaddl_u16(vpIn0, vpIn1);
				vS = vaddw_u16(vS, vpIn2);
				int32x4_t v_div = vqdmulhq_n_s32(vreinterpretq_s32_u32(vS), mul);
				uint16x4_t vOut = vmovn_u32(vreinterpretq_u32_s32(v_div));
				vst1_u16(pOutLine, vOut); pOutLine += 4;
			}
			#endif
			for (; i < nChannel; i++)
			{
				S = pIn[0][i] + pIn[1][i] + pIn[2][i];
				*(pOutLine++) = S / 3;
				pIn[0] = pIn[1];
				pIn[1] = pIn[2];
			}
		}
		pIn[2] = pIn[1];
		{
			for (int i = 0; i < nChannel; i++)
			{
				S = pIn[0][i] + pIn[1][i] + pIn[2][i];
				*(pOutLine++) = S / 3;
			}
		}
	}
}
extern void VAvg3Line(unsigned short *pInLines[], unsigned short *pOutLine, int nChannel, int nWidth)
{
	int StrideX = nWidth * nChannel;
	unsigned short *pIn[3];
	for (int i = 0; i < 3; i++)
	{
		pIn[i] = pInLines[i];
	}
	const int mul = ((unsigned int)1 << 31) / 3 + 1;
	int x = 0;
#ifdef USE_NEON
	uint32x4_t vS;
	uint16x4_t vpIn0;
	uint16x4_t vpIn1;
	uint16x4_t vpIn2;
	int tmpnWidthDim = StrideX - 3;
	for (; x < tmpnWidthDim; x += 4)
	{
		vpIn0 = vld1_u16(pIn[0] + x);
		vpIn1 = vld1_u16(pIn[1] + x);
		vpIn2 = vld1_u16(pIn[2] + x);
		vS = vaddl_u16(vpIn0, vpIn1);
		vS = vaddw_u16(vS, vpIn2);
		int32x4_t v_div = vqdmulhq_n_s32(vreinterpretq_s32_u32(vS), mul);
		uint16x4_t vOut = vmovn_u32(vreinterpretq_u32_s32(v_div));
		vst1_u16(pOutLine, vOut); pOutLine += 4;
	}
#endif
	for (; x < StrideX; x++)
	{
		unsigned int S = pInLines[0][x] + 2;
		S += pInLines[1][x];
		S += pInLines[2][x];
		S /= 3;
		*(pOutLine++) = (unsigned short)S;
	}
}
extern void HAvg5Line(unsigned short *pInLine, unsigned int *pOutLine, int nChannel, int nWidth)
{
	int i, x;
	unsigned int S;
	unsigned short *pIn[5];
	pIn[0] = pInLine;
	pIn[1] = pInLine;
	pIn[2] = pInLine;
	pIn[3] = pIn[2] + nChannel;
	pIn[4] = pIn[3] + nChannel;
	int tmpnWidth = nWidth - 2;
	for (x = 0; x < tmpnWidth; x++)
	{
		for (i = 0; i < nChannel; i++)
		{
			S = pIn[0][i] + pIn[1][i] + pIn[2][i] + pIn[3][i] + pIn[4][i];
			*(pOutLine++) = S;
		}
		pIn[0] = pIn[1];
		pIn[1] = pIn[2];
		pIn[2] = pIn[3];
		pIn[3] = pIn[4];
		pIn[4] += nChannel;
	}
	pIn[4] = pIn[3];
	for (; x < nWidth; x++)
	{
		for (i = 0; i < nChannel; i++)
		{
			S = pIn[0][i] + pIn[1][i] + pIn[2][i] + pIn[3][i] + pIn[4][i];
			*(pOutLine++) = S;
		}
		pIn[0] = pIn[1];
		pIn[1] = pIn[2];
		pIn[2] = pIn[3];
		pIn[3] = pIn[4];
	}
}
extern void VAvg5Line(unsigned int *pInLines[], unsigned short *pOutLine, int nChannel, int nWidth)
{
	int i, x;
	unsigned int *pIn[5];
	for (i = 0; i < 5; i++)
	{
		pIn[i] = pInLines[i];
	}
	for (x = 0; x < nWidth; x++)
	{
		for (i = 0; i < nChannel; i++)
		{
			unsigned int S = (pIn[0][i] + pIn[1][i] + pIn[2][i] + pIn[3][i] + pIn[4][i] + 12) / 25;
			*(pOutLine++) = (unsigned short)(S);
		}
		for (i = 0; i < 5; i++)
		{
			pIn[i] += nChannel;
		}
	}
}
extern inline void ftofHSmoothLinex7(float *pInLine, float *pOutLine, int nWidth)//
{
	float k[4] = { 0.026267f,0.100742f,0.225511f,0.29496f };
#ifdef USE_NEON
	float32x4_t k0 = vdupq_n_f32(0.026267f);
	float32x4_t k1 = vdupq_n_f32(0.100742f);
	float32x4_t k2 = vdupq_n_f32(0.225511f);
	float32x4_t k3 = vdupq_n_f32(0.29496f);
	float32x4_t sum = vdupq_n_f32(0.0f);
#endif
	float *pIn[7];
	int  x;
	float Y;
	pIn[0] = pIn[1] = pIn[2] = pIn[3] = pInLine;
	pIn[4] = pIn[3] + 1;
	pIn[5] = pIn[4] + 1;
	pIn[6] = pIn[5] + 1;
	for (x = 0; x < 3; x++)
	{
		Y = (pIn[0][0] + pIn[6][0])*k[0];
		Y += (pIn[1][0] + pIn[5][0]) *k[1];
		Y += (pIn[2][0] + pIn[4][0]) *k[2];
		Y += pIn[3][0] * k[3];
		pOutLine[0] = Y;
		pOutLine += 1;
		pIn[0] = pIn[1];
		pIn[1] = pIn[2];
		pIn[2] = pIn[3];
		pIn[3] = pIn[4];
		pIn[4] = pIn[5];
		pIn[5] = pIn[6];
		pIn[6] += 1;
	}
	int tmpnWidth = (nWidth - 3) / 4 * 4;
	for (x = 3; x < tmpnWidth; x += 4)
	{
#ifdef USE_NEON
		//vfmaq_f32 vmlaq_f32
		sum = vmlaq_f32(vmulq_f32((vld1q_f32(pIn[3])), k3), vaddq_f32(vld1q_f32(pIn[0]), vld1q_f32(pIn[6])), k0);
		sum = vmlaq_f32(sum, (vaddq_f32(vld1q_f32(pIn[1]), vld1q_f32(pIn[5]))), k1);
		sum = vmlaq_f32(sum, (vaddq_f32(vld1q_f32(pIn[2]), vld1q_f32(pIn[4]))), k2);
		vst1q_f32(pOutLine, sum);
		pOutLine += 4;	
#else
		pOutLine[0] = (pIn[0][0] + pIn[6][0])*k[0] + (pIn[1][0] + pIn[5][0]) *k[1] + (pIn[2][0] + pIn[4][0]) *k[2] + pIn[3][0] * k[3];
		pOutLine += 1;//+5\CA\C7?\C1\CB\C8\C3\CF\E0\C1?\C45\D0快\C4\CA\FD\BE?\E5\C8?\C6\E4\D6孝\AC\B8\F8\BA\F3\C3\E6\B5?\B9?\B7\BD\CF\F2?\C3\E8\D7\F6?\B1\B8\CF\E0\C1?\C45\D0\D0
		pOutLine[0] = (pIn[0][1] + pIn[6][1])*k[0] + (pIn[1][1] + pIn[5][1]) *k[1] + (pIn[2][1] + pIn[4][1]) *k[2] + pIn[3][1] * k[3];
		pOutLine += 1;//+5\CA\C7?\C1\CB\C8\C3\CF\E0\C1?\C45\D0快\C4\CA\FD\BE?\E5\C8?\C6\E4\D6孝\AC\B8\F8\BA\F3\C3\E6\B5?\B9?\B7\BD\CF\F2?\C3\E8\D7\F6?\B1\B8\CF\E0\C1?\C45\D0\D0
		pOutLine[0] = (pIn[0][2] + pIn[6][2])*k[0] + (pIn[1][2] + pIn[5][2]) *k[1] + (pIn[2][2] + pIn[4][2]) *k[2] + pIn[3][2] * k[3];
		pOutLine += 1;//+5\CA\C7?\C1\CB\C8\C3\CF\E0\C1?\C45\D0快\C4\CA\FD\BE?\E5\C8?\C6\E4\D6孝\AC\B8\F8\BA\F3\C3\E6\B5?\B9?\B7\BD\CF\F2?\C3\E8\D7\F6?\B1\B8\CF\E0\C1?\C45\D0\D0
		pOutLine[0] = (pIn[0][3] + pIn[6][3])*k[0] + (pIn[1][3] + pIn[5][3]) *k[1] + (pIn[2][3] + pIn[4][3]) *k[2] + pIn[3][3] * k[3];
		pOutLine += 1;//+5\CA\C7?\C1\CB\C8\C3\CF\E0\C1?\C45\D0快\C4\CA\FD\BE?\E5\C8?\C6\E4\D6孝\AC\B8\F8\BA\F3\C3\E6\B5?\B9?\B7\BD\CF\F2?\C3\E8\D7\F6?\B1\B8\CF\E0\C1?\C45\D0\D0
#endif
		pIn[0] += 4;
		pIn[1] += 4;
		pIn[2] += 4;
		pIn[3] += 4;
		pIn[4] += 4;
		pIn[5] += 4;
		pIn[6] += 4;
	}
	for (; x < nWidth - 3; x++)
	{
		Y = (pIn[0][0] + pIn[6][0])*k[0];
		Y += (pIn[1][0] + pIn[5][0]) *k[1];
		Y += (pIn[2][0] + pIn[4][0]) *k[2];
		Y += pIn[3][0] * k[3];
		pOutLine[0] = Y;
		pOutLine += 1;
		pIn[0] += 1;
		pIn[1] += 1;
		pIn[2] += 1;
		pIn[3] += 1;
		pIn[4] += 1;
		pIn[5] += 1;
		pIn[6] += 1;
	}
	pIn[6] = pIn[5];
	for (; x < nWidth; x++)
	{
		Y = (pIn[0][0] + pIn[6][0])*k[0];
		Y += (pIn[1][0] + pIn[5][0]) *k[1];
		Y += (pIn[2][0] + pIn[4][0]) *k[2];
		Y += pIn[3][0] * k[3];
		pOutLine[0] = Y;
		pOutLine += 1;
		pIn[0] = pIn[1];
		pIn[1] = pIn[2];
		pIn[2] = pIn[3];
		pIn[3] = pIn[4];
		pIn[4] = pIn[5];
		pIn[5] = pIn[6];
	}
}
extern inline void ftofVSmoothLinex7(float *pInLines[], float *pOutLine, int nWidth)
{
	//float k[4] = { 0.026267f,0.100742f,0.225511f,0.29496f };
	float *pIn[7];
#ifdef USE_NEON
	float32x4_t sum;
	float32x4_t k0 = vdupq_n_f32(0.026267f);
	float32x4_t k1 = vdupq_n_f32(0.100742f);
	float32x4_t k2 = vdupq_n_f32(0.225511f);
	float32x4_t k3 = vdupq_n_f32(0.29496f);
#endif
	int x = nWidth;
	pIn[0] = pInLines[0];
	pIn[1] = pInLines[1];
	pIn[2] = pInLines[2];
	pIn[3] = pInLines[3];
	pIn[4] = pInLines[4];
	pIn[5] = pInLines[5];
	pIn[6] = pInLines[6];
#ifdef arm32
	asm volatile(
		"vdup.f32 q7,  %18\n"//kk0
		"vdup.f32 q8,  %19\n"//kk1
		"vdup.f32 q9,  %20\n"//kk2
		"vdup.f32 q10, %21\n"//kk3
		"0:                    \n"
		"pld [%0,#128]\n"  //pldl1keep pldl1strm
		"pld [%1,#128]\n"
		"pld [%2,#128]\n"
		"pld [%3,#128]\n"
		"pld [%4,#128]\n"
		"pld [%5,#128]\n"
		"pld [%6,#128]\n"
		//"pst [%8,#128]\n"
		"vld1.f32 {d0-d1}, [%0 :128]!\n"
		"vld1.f32 {d2-d3}, [%1 :128]!\n"
		"vld1.f32 {d4-d5}, [%2 :128]!\n"
		"vld1.f32 {d6-d7}, [%3 :128]!\n"
		"vld1.f32 {d8-d9}, [%4 :128]!\n"
		"vld1.f32 {d10-d11}, [%5 :128]!\n"
		"vld1.f32 {d12-d13}, [%6 :128]!\n"
		"vmul.f32 q3, q3, q10 \n" //vmulq_f32(vld1q_f32(pIn3), k3)
		"vadd.f32 q0, q0, q6\n" //addq_f32(vld1q_f32(pIn0), vld1q_f32(pIn6))
		"vmla.f32 q3, q0, q7\n" //vfmaq_f32(vmulq_f32(vld1q_f32(pIn3), k3), vaddq_f32(vld1q_f32(pIn0), vld1q_f32(pIn6)), k0);
		"vadd.f32 q1, q1, q5\n" //vaddq_f32(vld1q_f32(pIn1), vld1q_f32(pIn5))
		"vmla.f32 q3, q1, q8\n" //vfmaq_f32(sum, vaddq_f32(vld1q_f32(pIn1), vld1q_f32(pIn5)), k1);
		"vadd.f32 q2, q2, q4\n" //vaddq_f32(vld1q_f32(pIn2), vld1q_f32(pIn4))
		"vmla.f32 q3, q2, q9\n" //vfmaq_f32(sum, vaddq_f32(vld1q_f32(pIn2), vld1q_f32(pIn4)), k2);
		"vst1.f32     {d6-d7}, [%17 :128]!\n"
		//"str {d6-d7}, [%17 :128]!\n"
		"subs       %7, %7, #4          \n"
		"bne     0b                    \n"
		: "=r"(pIn0),     // %0
		"=r"(pIn1),
		"=r"(pIn2),
		"=r"(pIn3),     // %3
		"=r"(pIn4),
		"=r"(pIn5),
		"=r"(pIn6),
		"=r"(x),
		"=r"(pOutLine) //%8
		: "0"(pIn0),
		"1"(pIn1),
		"2"(pIn2),
		"3"(pIn3),
		"4"(pIn4),
		"5"(pIn5),
		"6"(pIn6),
		"7"(x),
		"8"(pOutLine), //%8
		"r"(k[0]),   //%17
		"r"(k[1]),
		"r"(k[2]),
		"r"(k[3])
		: "memory", "q0", "q1", "q2", "q3", "q4", "q5", "q6", "q7", "q8", "q9", "q10"
		);
#endif
#ifdef arm64
	asm volatile(
		"dup v7.4s,  %w18\n"//kk0
		"dup v8.4s,  %w19\n"//kk1
		"dup v9.4s,  %w20\n"//kk2
		"dup v10.4s, %w21\n"//kk3
		"0:                    \n"
		//"prfm pldl1keep, [%0,#128]\n"  //pldl1keep pldl1strm
		//"prfm pldl1keep, [%1,#128]\n"
		//"prfm pldl1keep, [%2,#128]\n"
		//"prfm pldl1keep, [%3,#128]\n"
		//"prfm pldl1keep, [%4,#128]\n"
		//"prfm pldl1keep, [%5,#128]\n"
		//"prfm pldl1keep, [%6,#128]\n"
		//"prfm pstl1keep, [%7,#128]\n"
		//"prfm pldl1keep, [%7,#128]\n"
		"ld1 {v0.4s}, [%0] ,#16\n"
		"ld1 {v1.4s}, [%1] ,#16\n"
		"ld1 {v2.4s}, [%2] ,#16\n"
		"ld1 {v3.4s}, [%3] ,#16\n"
		"ld1 {v4.4s}, [%4] ,#16\n"
		"ld1 {v5.4s}, [%5] ,#16\n"
		"ld1 {v6.4s}, [%6] ,#16\n"
		"fadd v0.4s, v0.4s, v6.4s\n" //addq_f32(vld1q_f32(pIn0), vld1q_f32(pIn6))
		"fadd v1.4s, v1.4s, v5.4s\n" //vaddq_f32(vld1q_f32(pIn1), vld1q_f32(pIn5))
		"fadd v2.4s, v2.4s, v4.4s\n" //vaddq_f32(vld1q_f32(pIn2), vld1q_f32(pIn4))

		"fmul v3.4s, v3.4s, v10.4s \n" //vmulq_f32(vld1q_f32(pIn3), k3) k3
		"fmla v3.4s, v0.4s, v7.4s\n" //vfmaq_f32(vmulq_f32(vld1q_f32(pIn3), k3), vaddq_f32(vld1q_f32(pIn0), vld1q_f32(pIn6)), k0); k0
		"fmla v3.4s, v1.4s, v8.4s\n" //vfmaq_f32(sum, vaddq_f32(vld1q_f32(pIn1), vld1q_f32(pIn5)), k1); k1
		"fmla v3.4s, v2.4s, v9.4s\n" //vfmaq_f32(sum, vaddq_f32(vld1q_f32(pIn2), vld1q_f32(pIn4)), k2); k2
		"st1     {v3.4s}, [%7], #16\n"
		//"str q3, [%7], #16\n"
		"subs       %w8, %w8, #4          \n"
		"bne     0b                    \n"
		: "=r"(pIn[0]),     // %0
		"=r"(pIn[1]),
		"=r"(pIn[2]),
		"=r"(pIn[3]),     // %3
		"=r"(pIn[4]),
		"=r"(pIn[5]),
		"=r"(pIn[6]),
		"=r"(pOutLine), //%8
		"=r"(x)
		: "0"(pIn[0]),
		"1"(pIn[1]),
		"2"(pIn[2]),
		"3"(pIn[3]),
		"4"(pIn[4]),
		"5"(pIn[5]),
		"6"(pIn[6]),
		"7"(pOutLine), //%8
		"8"(x),
		"r"(k[0]),   //%17
		"r"(k[1]),
		"r"(k[2]),
		"r"(k[3])
		: "cc", "memory", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10"
		);
#endif
	//600/20=30;

#ifdef arm642
	asm volatile(
		"dup v14.4s,  %w18\n"//kk0
		"dup v15.4s,  %w19\n"//kk1
		"dup v16.4s,  %w20\n"//kk2
		"dup v17.4s,  %w21\n"//kk3
		"0:                    \n"
		//"prfm pldl2keep, [%0,#128]\n"  //pldl1keep pldl1strm
		//"prfm pldl2keep, [%1,#128]\n"
		//"prfm pldl2keep, [%2,#128]\n"
		//"prfm pldl2keep, [%3,#128]\n"
		//"prfm pldl2keep, [%4,#128]\n"
		//"prfm pldl2keep, [%5,#128]\n"
		//"prfm pldl2keep, [%6,#128]\n"
		//"prfm pstl2keep, [%8,#256]\n"
		"ld2 {v0.4s,v1.4s}, [%0] ,#32\n"//pIn0
		"ld2 {v2.4s,v3.4s}, [%1] ,#32\n"//pIn1
		"ld2 {v4.4s,v5.4s}, [%2] ,#32\n"//pIn2
		"ld2 {v6.4s,v7.4s}, [%3] ,#32\n"//pIn3
		"ld2 {v8.4s,v9.4s}, [%4] ,#32\n"//pIn4
		"ld2 {v10.4s,v11.4s}, [%5] ,#32\n"//pIn5
		"ld2 {v12.4s,v13.4s}, [%6] ,#32\n"//pIn6
		"fadd v0.4s, v0.4s, v12.4s\n" //addq_f32(vld1q_f32(pIn0), vld1q_f32(pIn6))
		"fadd v1.4s, v1.4s, v13.4s\n" //addq_f32(vld1q_f32(pIn0), vld1q_f32(pIn6))
		"fadd v2.4s, v2.4s, v10.4s\n" //vaddq_f32(vld1q_f32(pIn1), vld1q_f32(pIn5))
		"fadd v3.4s, v3.4s, v11.4s\n" //vaddq_f32(vld1q_f32(pIn1), vld1q_f32(pIn5))
		"fadd v4.4s, v4.4s, v8.4s\n" //vaddq_f32(vld1q_f32(pIn2), vld1q_f32(pIn4))
		"fadd v5.4s, v5.4s, v9.4s\n" //vaddq_f32(vld1q_f32(pIn2), vld1q_f32(pIn4))

		"fmul v6.4s, v6.4s, v17.4s \n" //vmulq_f32(vld1q_f32(pIn3), k3)
		"fmul v7.4s, v7.4s, v17.4s \n" //vmulq_f32(vld1q_f32(pIn3), k3)
		"fmla v6.4s, v0.4s, v14.4s\n" //vfmaq_f32(vmulq_f32(vld1q_f32(pIn3), k3), vaddq_f32(vld1q_f32(pIn0), vld1q_f32(pIn6)), k0);
		"fmla v7.4s, v1.4s, v14.4s\n" //vfmaq_f32(vmulq_f32(vld1q_f32(pIn3), k3), vaddq_f32(vld1q_f32(pIn0), vld1q_f32(pIn6)), k0);
		"fmla v6.4s, v2.4s, v15.4s\n" //vfmaq_f32(sum, vaddq_f32(vld1q_f32(pIn1), vld1q_f32(pIn5)), k1);
		"fmla v7.4s, v3.4s, v15.4s\n" //vfmaq_f32(sum, vaddq_f32(vld1q_f32(pIn1), vld1q_f32(pIn5)), k1);
		"fmla v6.4s, v4.4s, v16.4s\n" //vfmaq_f32(sum, vaddq_f32(vld1q_f32(pIn2), vld1q_f32(pIn4)), k2);
		"fmla v7.4s, v5.4s, v16.4s\n" //vfmaq_f32(sum, vaddq_f32(vld1q_f32(pIn2), vld1q_f32(pIn4)), k2);
		"subs       %w8, %w8, #8          \n"
		"st2     {v6.4s,v7.4s}, [%7], #32\n"
		"bne     0b                    \n"
		: "=r"(pIn[0]),     // %0
		"=r"(pIn[1]),
		"=r"(pIn[2]),
		"=r"(pIn[3]),     // %3
		"=r"(pIn[4]),
		"=r"(pIn[5]),
		"=r"(pIn[6]),
		"=r"(pOutLine), //%8
		"=r"(x)
		: "0"(pIn[0]),
		"1"(pIn[1]),
		"2"(pIn[2]),
		"3"(pIn[3]),
		"4"(pIn[4]),
		"5"(pIn[5]),
		"6"(pIn[6]),
		"7"(pOutLine), //%8
		"8"(x),
		"r"(k[0]),   //%17
		"r"(k[1]),
		"r"(k[2]),
		"r"(k[3])
		: "cc", "memory", "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "v8", "v9", "v10", "v11", "v12", "v13", "v14", "v15", "v16", "v17", "v18"
		);
#endif

	for (x = nWidth; x > 0; x -= 4)
	{
#ifdef USE_NEON
		sum = vmlaq_f32(vmulq_f32(vld1q_f32(pIn[3]), k3), vaddq_f32(vld1q_f32(pIn[0]), vld1q_f32(pIn[6])), k0);
		sum = vmlaq_f32(sum, vaddq_f32(vld1q_f32(pIn[1]), vld1q_f32(pIn[5])), k1);
		sum = vmlaq_f32(sum, vaddq_f32(vld1q_f32(pIn[2]), vld1q_f32(pIn[4])), k2);
		vst1q_f32(pOutLine, sum);
		pOutLine += 4;
#else
		*(pOutLine++) = (pIn[0][0] + pIn[6][0])*k[0] + (pIn[1][0] + pIn[5][0]) *k[1] + (pIn[2][0] + pIn[4][0]) *k[2] + pIn[3][0] * k[3];
		*(pOutLine++) = (pIn[0][1] + pIn[6][1])*k[0] + (pIn[1][1] + pIn[5][1]) *k[1] + (pIn[2][1] + pIn[4][1]) *k[2] + pIn[3][1] * k[3];
		*(pOutLine++) = (pIn[0][2] + pIn[6][2])*k[0] + (pIn[1][2] + pIn[5][2]) *k[1] + (pIn[2][2] + pIn[4][2]) *k[2] + pIn[3][2] * k[3];
		*(pOutLine++) = (pIn[0][3] + pIn[6][3])*k[0] + (pIn[1][3] + pIn[5][3]) *k[1] + (pIn[2][3] + pIn[4][3]) *k[2] + pIn[3][3] * k[3];
#endif
		pIn[0] += 4;
		pIn[1] += 4;
		pIn[2] += 4;
		pIn[3] += 4;
		pIn[4] += 4;
		pIn[5] += 4;
		pIn[6] += 4;
	}
}
extern  bool FToFSmoothx7(float *pImage, float *pout, int nWidth, int nHeight)
{
	int nThread = omp_get_num_procs();
	float *pBuffer = new float[nWidth * 7 * nThread];
	if (pBuffer == NULL)return false;
	float *pHLines[7];
	float *pOutLine = pout;
	float *pInLine = pImage;
	unsigned short  loop = 0;
#pragma omp parallel for num_threads(nThread) firstprivate(loop) private(pHLines) 
	for (int y = 0; y < nHeight; y++)
	{
		if (loop == 0)
		{
			int nThreadId = omp_get_thread_num();
			pHLines[0] = pBuffer + 7 * nWidth*nThreadId;
			pHLines[1] = pHLines[0] + nWidth;
			pHLines[2] = pHLines[1] + nWidth;
			pHLines[3] = pHLines[2] + nWidth;
			pHLines[4] = pHLines[3] + nWidth;
			pHLines[5] = pHLines[4] + nWidth;
			pHLines[6] = pHLines[5] + nWidth;
			ftofHSmoothLinex7(pInLine + abs(y - 3)*nWidth, pHLines[0], nWidth);
			ftofHSmoothLinex7(pInLine + abs(y - 2)*nWidth, pHLines[1], nWidth);
			ftofHSmoothLinex7(pInLine + abs(y - 1)*nWidth, pHLines[2], nWidth);
			ftofHSmoothLinex7(pInLine + (y + 0)*nWidth, pHLines[3], nWidth);
			ftofHSmoothLinex7(pInLine + (y + 1)*nWidth, pHLines[4], nWidth);
			ftofHSmoothLinex7(pInLine + (y + 2)*nWidth, pHLines[5], nWidth);
			loop++;
		}
		int yy = y + 3;
		if (yy >= nHeight) yy -= 3;
		ftofHSmoothLinex7(pInLine + yy * nWidth, pHLines[6], nWidth);
		ftofVSmoothLinex7(pHLines, pOutLine + y * nWidth, nWidth);
		float  *pTemp0 = pHLines[0];
		pHLines[0] = pHLines[1];
		pHLines[1] = pHLines[2];
		pHLines[2] = pHLines[3];
		pHLines[3] = pHLines[4];
		pHLines[4] = pHLines[5];
		pHLines[5] = pHLines[6];
		pHLines[6] = pTemp0;
	}
	delete[] pBuffer;
	return true;
}
extern inline void stofHSmooth7Line(unsigned short *pInLine, float *pOutLine, int nWidth)//
{
	float k[4] = { 0.026267f,0.100742f,0.225511f,0.29496f };
#ifdef USE_NEON
	float32x4_t k0 = vdupq_n_f32(0.026267f);
	float32x4_t k1 = vdupq_n_f32(0.100742f);
	float32x4_t k2 = vdupq_n_f32(0.225511f);
	float32x4_t k3 = vdupq_n_f32(0.29496f);
	float32x4_t sum = vdupq_n_f32(0.0f);
#endif
	unsigned short *pIn[7];
	int  x;
	float Y;
	pIn[0] = pIn[1] = pIn[2] = pIn[3] = pInLine;
	pIn[4] = pIn[3] + 1;
	pIn[5] = pIn[4] + 1;
	pIn[6] = pIn[5] + 1;
	for (x = 0; x < 3; x++)
	{
		Y = (pIn[0][0] + pIn[6][0])*k[0];
		Y += (pIn[1][0] + pIn[5][0]) *k[1];
		Y += (pIn[2][0] + pIn[4][0]) *k[2];
		Y += pIn[3][0] * k[3];
		pOutLine[0] = Y;
		pOutLine += 1;//+5\CA\C7?\C1\CB\C8\C3\CF\E0\C1?\C45\D0快\C4\CA\FD\BE?\E5\C8?\C6\E4\D6孝\AC\B8\F8\BA\F3\C3\E6\B5?\B9?\B7\BD\CF\F2?\C3\E8\D7\F6?\B1\B8\CF\E0\C1?\C45\D0\D0
		pIn[0] = pIn[1];
		pIn[1] = pIn[2];
		pIn[2] = pIn[3];
		pIn[3] = pIn[4];
		pIn[4] = pIn[5];
		pIn[5] = pIn[6];
		pIn[6] += 1;
	}
	int tmpnWidth = (nWidth - 4) / 4 * 4;
	for (x = 3; x < tmpnWidth; x += 4)
	{
#ifdef USE_NEON
		//vfmaq_f32 vmlaq_f32
		sum = vmlaq_f32(vmulq_f32(vcvtq_f32_u32(vmovl_u16(vld1_u16(pIn[3]))), k3), vcvtq_f32_u32(vaddl_u16(vld1_u16(pIn[0]), vld1_u16(pIn[6]))), k0);
		sum = vmlaq_f32(sum, vcvtq_f32_u32(vaddl_u16(vld1_u16(pIn[1]), vld1_u16(pIn[5]))), k1);
		sum = vmlaq_f32(sum, vcvtq_f32_u32(vaddl_u16(vld1_u16(pIn[2]), vld1_u16(pIn[4]))), k2);
		vst1q_f32(pOutLine, sum);
		pOutLine += 4;	
#else
		pOutLine[0] = (pIn[0][0] + pIn[6][0])*k[0] + (pIn[1][0] + pIn[5][0]) *k[1] + (pIn[2][0] + pIn[4][0]) *k[2] + pIn[3][0] * k[3];
		pOutLine += 1;//+5\CA\C7?\C1\CB\C8\C3\CF\E0\C1?\C45\D0快\C4\CA\FD\BE?\E5\C8?\C6\E4\D6孝\AC\B8\F8\BA\F3\C3\E6\B5?\B9?\B7\BD\CF\F2?\C3\E8\D7\F6?\B1\B8\CF\E0\C1?\C45\D0\D0
		pOutLine[0] = (pIn[0][1] + pIn[6][1])*k[0] + (pIn[1][1] + pIn[5][1]) *k[1] + (pIn[2][1] + pIn[4][1]) *k[2] + pIn[3][1] * k[3];
		pOutLine += 1;//+5\CA\C7?\C1\CB\C8\C3\CF\E0\C1?\C45\D0快\C4\CA\FD\BE?\E5\C8?\C6\E4\D6孝\AC\B8\F8\BA\F3\C3\E6\B5?\B9?\B7\BD\CF\F2?\C3\E8\D7\F6?\B1\B8\CF\E0\C1?\C45\D0\D0
		pOutLine[0] = (pIn[0][2] + pIn[6][2])*k[0] + (pIn[1][2] + pIn[5][2]) *k[1] + (pIn[2][2] + pIn[4][2]) *k[2] + pIn[3][2] * k[3];
		pOutLine += 1;//+5\CA\C7?\C1\CB\C8\C3\CF\E0\C1?\C45\D0快\C4\CA\FD\BE?\E5\C8?\C6\E4\D6孝\AC\B8\F8\BA\F3\C3\E6\B5?\B9?\B7\BD\CF\F2?\C3\E8\D7\F6?\B1\B8\CF\E0\C1?\C45\D0\D0
		pOutLine[0] = (pIn[0][3] + pIn[6][3])*k[0] + (pIn[1][3] + pIn[5][3]) *k[1] + (pIn[2][3] + pIn[4][3]) *k[2] + pIn[3][3] * k[3];
		pOutLine += 1;//+5\CA\C7?\C1\CB\C8\C3\CF\E0\C1?\C45\D0快\C4\CA\FD\BE?\E5\C8?\C6\E4\D6孝\AC\B8\F8\BA\F3\C3\E6\B5?\B9?\B7\BD\CF\F2?\C3\E8\D7\F6?\B1\B8\CF\E0\C1?\C45\D0\D0
#endif
		pIn[0] += 4;
		pIn[1] += 4;
		pIn[2] += 4;
		pIn[3] += 4;
		pIn[4] += 4;
		pIn[5] += 4;
		pIn[6] += 4;
	}
	for (; x < nWidth - 4; x++)
	{
		Y = (pIn[0][0] + pIn[6][0])*k[0];
		Y += (pIn[1][0] + pIn[5][0]) *k[1];
		Y += (pIn[2][0] + pIn[4][0]) *k[2];
		Y += pIn[3][0] * k[3];
		pOutLine[0] = Y;
		pOutLine += 1;
		pIn[0] += 1;
		pIn[1] += 1;
		pIn[2] += 1;
		pIn[3] += 1;
		pIn[4] += 1;
		pIn[5] += 1;
		pIn[6] += 1;
	}
	pIn[6] = pIn[5];
	for (; x < nWidth; x++)
	{
		Y = (pIn[0][0] + pIn[6][0])*k[0];
		Y += (pIn[1][0] + pIn[5][0]) *k[1];
		Y += (pIn[2][0] + pIn[4][0]) *k[2];
		Y += pIn[3][0] * k[3];
		pOutLine[0] = Y;
		pOutLine += 1;
		pIn[0] = pIn[1];
		pIn[1] = pIn[2];
		pIn[2] = pIn[3];
		pIn[3] = pIn[4];
		pIn[4] = pIn[5];
		pIn[5] = pIn[6];
	}
}
extern inline void ftosVSmooth7Line(float *pInLines[], unsigned short *pOutLine, int nWidth)
{
	float k[4] = { 0.026267f,0.100742f,0.225511f,0.29496f };
	float *pIn[7];
	int x = nWidth;
	pIn[0] = pInLines[0];
	pIn[1] = pInLines[1];
	pIn[2] = pInLines[2];
	pIn[3] = pInLines[3];
	pIn[4] = pInLines[4];
	pIn[5] = pInLines[5];
	pIn[6] = pInLines[6];
	for (x = nWidth; x > 0; x -= 4)
	{
		*(pOutLine++) = (pIn[0][0] + pIn[6][0])*k[0] + (pIn[1][0] + pIn[5][0]) *k[1] + (pIn[2][0] + pIn[4][0]) *k[2] + pIn[3][0] * k[3];
		*(pOutLine++) = (pIn[0][1] + pIn[6][1])*k[0] + (pIn[1][1] + pIn[5][1]) *k[1] + (pIn[2][1] + pIn[4][1]) *k[2] + pIn[3][1] * k[3];
		*(pOutLine++) = (pIn[0][2] + pIn[6][2])*k[0] + (pIn[1][2] + pIn[5][2]) *k[1] + (pIn[2][2] + pIn[4][2]) *k[2] + pIn[3][2] * k[3];
		*(pOutLine++) = (pIn[0][3] + pIn[6][3])*k[0] + (pIn[1][3] + pIn[5][3]) *k[1] + (pIn[2][3] + pIn[4][3]) *k[2] + pIn[3][3] * k[3];
		pIn[0] += 4;
		pIn[1] += 4;
		pIn[2] += 4;
		pIn[3] += 4;
		pIn[4] += 4;
		pIn[5] += 4;
		pIn[6] += 4;
	}
}

extern bool StoSSmooth7Image(unsigned short *pImage, unsigned short *pout, int nWidth, int nHeight)
{
	int nThread = omp_get_num_procs();
	float *pBuffer = new float[nWidth * 7 * nThread];
	if (pBuffer == NULL)return false;
	float *pHLines[7];
	unsigned short *pOutLine = pout;
	unsigned short *pInLine = pImage;
	unsigned short  loop = 0;
#pragma omp parallel for num_threads(nThread) firstprivate(loop) private(pHLines) 
	for (int y = 0; y < nHeight; y++)
	{
		if (loop == 0)
		{
			int nThreadId = omp_get_thread_num();
			pHLines[0] = pBuffer + 7 * nWidth*nThreadId;
			pHLines[1] = pHLines[0] + nWidth;
			pHLines[2] = pHLines[1] + nWidth;
			pHLines[3] = pHLines[2] + nWidth;
			pHLines[4] = pHLines[3] + nWidth;
			pHLines[5] = pHLines[4] + nWidth;
			pHLines[6] = pHLines[5] + nWidth;
			stofHSmooth7Line(pInLine + abs(y - 3)*nWidth, pHLines[0], nWidth);
			stofHSmooth7Line(pInLine + abs(y - 2)*nWidth, pHLines[1], nWidth);
			stofHSmooth7Line(pInLine + abs(y - 1)*nWidth, pHLines[2], nWidth);
			stofHSmooth7Line(pInLine + (y + 0)*nWidth, pHLines[3], nWidth);
			stofHSmooth7Line(pInLine + (y + 1)*nWidth, pHLines[4], nWidth);
			stofHSmooth7Line(pInLine + (y + 2)*nWidth, pHLines[5], nWidth);
			loop++;
		}
		int yy = y + 3;
		if (yy >= nHeight) yy -= 3;
		stofHSmooth7Line(pInLine + yy * nWidth, pHLines[6], nWidth);
		ftosVSmooth7Line(pHLines, pOutLine + y * nWidth, nWidth);
		float  *pTemp0 = pHLines[0];
		pHLines[0] = pHLines[1];
		pHLines[1] = pHLines[2];
		pHLines[2] = pHLines[3];
		pHLines[3] = pHLines[4];
		pHLines[4] = pHLines[5];
		pHLines[5] = pHLines[6];
		pHLines[6] = pTemp0;
	}
	delete[] pBuffer;
	return true;
}

extern inline void ftofHSmoothLinex15(float *pInLine, float *pOutLine, int nWidth)//
{
	float k[8] = { 0.004961f, 0.012246f, 0.026304f, 0.049165f, 0.079968f, 0.113193f, 0.139431f, 0.149464f };
	float *pIn[15];
	int  x;
	float Y;
	pIn[0] = pIn[1] = pIn[2] = pIn[3] = pIn[4] = pIn[5] = pIn[6] = pIn[7] =  pInLine;
	pIn[8] = pIn[7] + 1;
	pIn[9] = pIn[8] + 1;
	pIn[10] = pIn[9] + 1;
	pIn[11] = pIn[10] + 1;
	pIn[12] = pIn[11] + 1;
	pIn[13] = pIn[12] + 1;
	pIn[14] = pIn[13] + 1;
	for (x = 0; x < 7; x++)
	{
		Y = (pIn[0][0] + pIn[14][0])*k[0];
		Y += (pIn[1][0] + pIn[13][0]) *k[1];
		Y += (pIn[2][0] + pIn[12][0]) *k[2];
		Y += (pIn[3][0] + pIn[11][0]) *k[3];
		Y += (pIn[4][0] + pIn[10][0]) *k[4];
		Y += (pIn[5][0] + pIn[9][0]) *k[5];
		Y += (pIn[6][0] + pIn[8][0]) *k[6];
		Y += pIn[7][0] * k[7];
		*(pOutLine++) = Y;
		pIn[0] = pIn[1];
		pIn[1] = pIn[2];
		pIn[2] = pIn[3];
		pIn[3] = pIn[4];
		pIn[4] = pIn[5];
		pIn[5] = pIn[6];
		pIn[6] = pIn[7];
		pIn[7] = pIn[8];
		pIn[8] = pIn[9];
		pIn[9] = pIn[10];
		pIn[10] = pIn[11];
		pIn[11] = pIn[12];
		pIn[12] = pIn[13];
		pIn[13] = pIn[14];
		pIn[14] += 1;
	}
	for (; x < nWidth - 7; x++)
	{
		Y = (pIn[0][0] + pIn[14][0])*k[0];
		Y += (pIn[1][0] + pIn[13][0]) *k[1];
		Y += (pIn[2][0] + pIn[12][0]) *k[2];
		Y += (pIn[3][0] + pIn[11][0]) *k[3];
		Y += (pIn[4][0] + pIn[10][0]) *k[4];
		Y += (pIn[5][0] + pIn[9][0]) *k[5];
		Y += (pIn[6][0] + pIn[8][0]) *k[6];
		Y += pIn[7][0] * k[7];
		*(pOutLine++) = Y;
		pIn[0] ++;
		pIn[1] ++;
		pIn[2] ++;
		pIn[3] ++;
		pIn[4] ++;
		pIn[5] ++;
		pIn[6] ++;
		pIn[7] ++;
		pIn[8] ++;
		pIn[9] ++;
		pIn[10] ++;
		pIn[11] ++;
		pIn[12] ++;
		pIn[13] ++;
		pIn[14] ++;
	}
	pIn[6] = pIn[5];
	for (; x < nWidth; x++)
	{
		Y = (pIn[0][0] + pIn[14][0])*k[0];
		Y += (pIn[1][0] + pIn[13][0]) *k[1];
		Y += (pIn[2][0] + pIn[12][0]) *k[2];
		Y += (pIn[3][0] + pIn[11][0]) *k[3];
		Y += (pIn[4][0] + pIn[10][0]) *k[4];
		Y += (pIn[5][0] + pIn[9][0]) *k[5];
		Y += (pIn[6][0] + pIn[8][0]) *k[6];
		Y += pIn[7][0] * k[7];
		*(pOutLine++) = Y;
		pIn[0] = pIn[1];
		pIn[1] = pIn[2];
		pIn[2] = pIn[3];
		pIn[3] = pIn[4];
		pIn[4] = pIn[5];
		pIn[5] = pIn[6];
		pIn[6] = pIn[7];
		pIn[7] = pIn[8];
		pIn[8] = pIn[9];
		pIn[9] = pIn[10];
		pIn[10] = pIn[11];
		pIn[11] = pIn[12];
		pIn[12] = pIn[13];
		pIn[13] = pIn[14];
	}
}
extern inline void ftofVSmoothLinex15(float *pInLines[], float *pOutLine, int nWidth)
{
	float k[8] = { 0.004961f, 0.012246f, 0.026304f, 0.049165f, 0.079968f, 0.113193f, 0.139431f, 0.149464f };
	float *pIn[15];
	float Y;
	int x = 0;
	pIn[0] = pInLines[0];
	pIn[1] = pInLines[1];
	pIn[2] = pInLines[2];
	pIn[3] = pInLines[3];
	pIn[4] = pInLines[4];
	pIn[5] = pInLines[5];
	pIn[6] = pInLines[6];
	pIn[7] = pInLines[7];
	pIn[8] = pInLines[8];
	pIn[9] = pInLines[9];
	pIn[10] = pInLines[10];
	pIn[11] = pInLines[11];
	pIn[12] = pInLines[12];
	pIn[13] = pInLines[13];
	pIn[14] = pInLines[14];
	for (x = 0; x< nWidth; x++)
	{
		Y = (pIn[0][0] + pIn[14][0])*k[0];
		Y += (pIn[1][0] + pIn[13][0]) *k[1];
		Y += (pIn[2][0] + pIn[12][0]) *k[2];
		Y += (pIn[3][0] + pIn[11][0]) *k[3];
		Y += (pIn[4][0] + pIn[10][0]) *k[4];
		Y += (pIn[5][0] + pIn[9][0]) *k[5];
		Y += (pIn[6][0] + pIn[8][0]) *k[6];
		Y += pIn[7][0] * k[7];
		*(pOutLine++) = Y;
		pIn[0] ++;
		pIn[1] ++;
		pIn[2] ++;
		pIn[3] ++;
		pIn[4] ++;
		pIn[5] ++;
		pIn[6] ++;
		pIn[7] ++;
		pIn[8] ++;
		pIn[9] ++;
		pIn[10] ++;
		pIn[11] ++;
		pIn[12] ++;
		pIn[13] ++;
		pIn[14] ++;
	}
}
extern  bool FToFSmoothx15(float *pImage, float *pout, int nWidth, int nHeight)
{
	int nThread = omp_get_num_procs();
	float *pBuffer = new float[nWidth * 15 * nThread];
	if (pBuffer == NULL)return false;
	float *pHLines[15];
	float *pOutLine = pout;
	float *pInLine = pImage;
	unsigned short  loop = 0;
#pragma omp parallel for num_threads(nThread) firstprivate(loop) private(pHLines) 
	for (int y = 0; y < nHeight; y++)
	{
		if (loop == 0)
		{
			int nThreadId = omp_get_thread_num();
			pHLines[0] = pBuffer + 15 * nWidth*nThreadId;
			pHLines[1] = pHLines[0] + nWidth;
			pHLines[2] = pHLines[1] + nWidth;
			pHLines[3] = pHLines[2] + nWidth;
			pHLines[4] = pHLines[3] + nWidth;
			pHLines[5] = pHLines[4] + nWidth;
			pHLines[6] = pHLines[5] + nWidth;
			pHLines[7] = pHLines[6] + nWidth;
			pHLines[8] = pHLines[7] + nWidth;
			pHLines[9] = pHLines[8] + nWidth;
			pHLines[10] = pHLines[9] + nWidth;
			pHLines[11] = pHLines[10] + nWidth;
			pHLines[12] = pHLines[11] + nWidth;
			pHLines[13] = pHLines[12] + nWidth;
			pHLines[14] = pHLines[13] + nWidth;

			ftofHSmoothLinex15(pInLine + abs(y - 7)*nWidth, pHLines[0], nWidth);
			ftofHSmoothLinex15(pInLine + abs(y - 6)*nWidth, pHLines[1], nWidth);
			ftofHSmoothLinex15(pInLine + abs(y - 5)*nWidth, pHLines[2], nWidth);
			ftofHSmoothLinex15(pInLine + abs(y - 4)*nWidth, pHLines[3], nWidth);
			ftofHSmoothLinex15(pInLine + abs(y - 3)*nWidth, pHLines[4], nWidth);
			ftofHSmoothLinex15(pInLine + abs(y - 2)*nWidth, pHLines[5], nWidth);
			ftofHSmoothLinex15(pInLine + abs(y - 1)*nWidth, pHLines[6], nWidth);
			ftofHSmoothLinex15(pInLine + (y + 0)*nWidth, pHLines[7], nWidth);
			ftofHSmoothLinex15(pInLine + (y + 1)*nWidth, pHLines[8], nWidth);
			ftofHSmoothLinex15(pInLine + (y + 2)*nWidth, pHLines[9], nWidth);
			ftofHSmoothLinex15(pInLine + (y + 3)*nWidth, pHLines[10], nWidth);
			ftofHSmoothLinex15(pInLine + (y + 4)*nWidth, pHLines[11], nWidth);
			ftofHSmoothLinex15(pInLine + (y + 5)*nWidth, pHLines[12], nWidth);
			ftofHSmoothLinex15(pInLine + (y + 6)*nWidth, pHLines[13], nWidth);
			loop++;
		}
		int yy = y + 7;
		if (yy >= nHeight) yy -= 7;
		ftofHSmoothLinex15(pInLine + yy * nWidth, pHLines[14], nWidth);
		ftofVSmoothLinex15(pHLines, pOutLine + y * nWidth, nWidth);
		float  *pTemp0 = pHLines[0];
		pHLines[0] = pHLines[1];
		pHLines[1] = pHLines[2];
		pHLines[2] = pHLines[3];
		pHLines[3] = pHLines[4];
		pHLines[4] = pHLines[5];
		pHLines[5] = pHLines[6];
		pHLines[6] = pHLines[7];
		pHLines[7] = pHLines[8];
		pHLines[8] = pHLines[9];
		pHLines[9] = pHLines[10];
		pHLines[10] = pHLines[11];
		pHLines[11] = pHLines[12];
		pHLines[12] = pHLines[13];
		pHLines[13] = pHLines[14];
		pHLines[14] = pTemp0;
	}
	delete[] pBuffer;
	return true;
}
extern inline void stosHSmoothLinex15(unsigned short *pInLine, float *pOutLine, int nWidth)//
{
	float k[8] = { 0.004961f, 0.012246f, 0.026304f, 0.049165f, 0.079968f, 0.113193f, 0.139431f, 0.149464f };
	unsigned short *pIn[15];
	int  x;
	float Y;
	pIn[0] = pIn[1] = pIn[2] = pIn[3] = pIn[4] = pIn[5] = pIn[6] = pIn[7] = pInLine;
	pIn[8] = pIn[7] + 1;
	pIn[9] = pIn[8] + 1;
	pIn[10] = pIn[9] + 1;
	pIn[11] = pIn[10] + 1;
	pIn[12] = pIn[11] + 1;
	pIn[13] = pIn[12] + 1;
	pIn[14] = pIn[13] + 1;
	for (x = 0; x < 7; x++)
	{
		Y = (pIn[0][0] + pIn[14][0])*k[0];
		Y += (pIn[1][0] + pIn[13][0]) *k[1];
		Y += (pIn[2][0] + pIn[12][0]) *k[2];
		Y += (pIn[3][0] + pIn[11][0]) *k[3];
		Y += (pIn[4][0] + pIn[10][0]) *k[4];
		Y += (pIn[5][0] + pIn[9][0]) *k[5];
		Y += (pIn[6][0] + pIn[8][0]) *k[6];
		Y += pIn[7][0] * k[7];
		*(pOutLine++) = Y;
		pIn[0] = pIn[1];
		pIn[1] = pIn[2];
		pIn[2] = pIn[3];
		pIn[3] = pIn[4];
		pIn[4] = pIn[5];
		pIn[5] = pIn[6];
		pIn[6] = pIn[7];
		pIn[7] = pIn[8];
		pIn[8] = pIn[9];
		pIn[9] = pIn[10];
		pIn[10] = pIn[11];
		pIn[11] = pIn[12];
		pIn[12] = pIn[13];
		pIn[13] = pIn[14];
		pIn[14] += 1;
	}
	for (; x < nWidth - 7; x++)
	{
		Y = (pIn[0][0] + pIn[14][0])*k[0];
		Y += (pIn[1][0] + pIn[13][0]) *k[1];
		Y += (pIn[2][0] + pIn[12][0]) *k[2];
		Y += (pIn[3][0] + pIn[11][0]) *k[3];
		Y += (pIn[4][0] + pIn[10][0]) *k[4];
		Y += (pIn[5][0] + pIn[9][0]) *k[5];
		Y += (pIn[6][0] + pIn[8][0]) *k[6];
		Y += pIn[7][0] * k[7];
		*(pOutLine++) = Y;
		pIn[0] ++;
		pIn[1] ++;
		pIn[2] ++;
		pIn[3] ++;
		pIn[4] ++;
		pIn[5] ++;
		pIn[6] ++;
		pIn[7] ++;
		pIn[8] ++;
		pIn[9] ++;
		pIn[10] ++;
		pIn[11] ++;
		pIn[12] ++;
		pIn[13] ++;
		pIn[14] ++;
	}
	pIn[6] = pIn[5];
	for (; x < nWidth; x++)
	{
		Y = (pIn[0][0] + pIn[14][0])*k[0];
		Y += (pIn[1][0] + pIn[13][0]) *k[1];
		Y += (pIn[2][0] + pIn[12][0]) *k[2];
		Y += (pIn[3][0] + pIn[11][0]) *k[3];
		Y += (pIn[4][0] + pIn[10][0]) *k[4];
		Y += (pIn[5][0] + pIn[9][0]) *k[5];
		Y += (pIn[6][0] + pIn[8][0]) *k[6];
		Y += pIn[7][0] * k[7];
		*(pOutLine++) = Y;
		pIn[0] = pIn[1];
		pIn[1] = pIn[2];
		pIn[2] = pIn[3];
		pIn[3] = pIn[4];
		pIn[4] = pIn[5];
		pIn[5] = pIn[6];
		pIn[6] = pIn[7];
		pIn[7] = pIn[8];
		pIn[8] = pIn[9];
		pIn[9] = pIn[10];
		pIn[10] = pIn[11];
		pIn[11] = pIn[12];
		pIn[12] = pIn[13];
		pIn[13] = pIn[14];
	}
}
extern inline void stosVSmoothLinex15(float *pInLines[], unsigned short *pOutLine, int nWidth)
{
	float k[8] = { 0.004961f, 0.012246f, 0.026304f, 0.049165f, 0.079968f, 0.113193f, 0.139431f, 0.149464f };
	float *pIn[15];
	float Y;
	int x = 0;
	pIn[0] = pInLines[0];
	pIn[1] = pInLines[1];
	pIn[2] = pInLines[2];
	pIn[3] = pInLines[3];
	pIn[4] = pInLines[4];
	pIn[5] = pInLines[5];
	pIn[6] = pInLines[6];
	pIn[7] = pInLines[7];
	pIn[8] = pInLines[8];
	pIn[9] = pInLines[9];
	pIn[10] = pInLines[10];
	pIn[11] = pInLines[11];
	pIn[12] = pInLines[12];
	pIn[13] = pInLines[13];
	pIn[14] = pInLines[14];
	for (x = 0; x < nWidth; x++)
	{
		Y = (pIn[0][0] + pIn[14][0])*k[0];
		Y += (pIn[1][0] + pIn[13][0]) *k[1];
		Y += (pIn[2][0] + pIn[12][0]) *k[2];
		Y += (pIn[3][0] + pIn[11][0]) *k[3];
		Y += (pIn[4][0] + pIn[10][0]) *k[4];
		Y += (pIn[5][0] + pIn[9][0]) *k[5];
		Y += (pIn[6][0] + pIn[8][0]) *k[6];
		Y += pIn[7][0] * k[7];
		*(pOutLine++) = Y;
		pIn[0] ++;
		pIn[1] ++;
		pIn[2] ++;
		pIn[3] ++;
		pIn[4] ++;
		pIn[5] ++;
		pIn[6] ++;
		pIn[7] ++;
		pIn[8] ++;
		pIn[9] ++;
		pIn[10] ++;
		pIn[11] ++;
		pIn[12] ++;
		pIn[13] ++;
		pIn[14] ++;
	}
}
extern  bool SToSSmoothx15(unsigned short *pImage, unsigned short *pout, int nWidth, int nHeight)
{
	int nThread = omp_get_num_procs();
	float *pBuffer = new float[nWidth * 15 * nThread];
	if (pBuffer == NULL)return false;
	float *pHLines[15];
	unsigned short *pOutLine = pout;
	unsigned short *pInLine = pImage;
	unsigned short  loop = 0;
#pragma omp parallel for num_threads(nThread) firstprivate(loop) private(pHLines) 
	for (int y = 0; y < nHeight; y++)
	{
		if (loop == 0)
		{
			int nThreadId = omp_get_thread_num();
			pHLines[0] = pBuffer + 15 * nWidth*nThreadId;
			pHLines[1] = pHLines[0] + nWidth;
			pHLines[2] = pHLines[1] + nWidth;
			pHLines[3] = pHLines[2] + nWidth;
			pHLines[4] = pHLines[3] + nWidth;
			pHLines[5] = pHLines[4] + nWidth;
			pHLines[6] = pHLines[5] + nWidth;
			pHLines[7] = pHLines[6] + nWidth;
			pHLines[8] = pHLines[7] + nWidth;
			pHLines[9] = pHLines[8] + nWidth;
			pHLines[10] = pHLines[9] + nWidth;
			pHLines[11] = pHLines[10] + nWidth;
			pHLines[12] = pHLines[11] + nWidth;
			pHLines[13] = pHLines[12] + nWidth;
			pHLines[14] = pHLines[13] + nWidth;

			stosHSmoothLinex15(pInLine + abs(y - 7)*nWidth, pHLines[0], nWidth);
			stosHSmoothLinex15(pInLine + abs(y - 6)*nWidth, pHLines[1], nWidth);
			stosHSmoothLinex15(pInLine + abs(y - 5)*nWidth, pHLines[2], nWidth);
			stosHSmoothLinex15(pInLine + abs(y - 4)*nWidth, pHLines[3], nWidth);
			stosHSmoothLinex15(pInLine + abs(y - 3)*nWidth, pHLines[4], nWidth);
			stosHSmoothLinex15(pInLine + abs(y - 2)*nWidth, pHLines[5], nWidth);
			stosHSmoothLinex15(pInLine + abs(y - 1)*nWidth, pHLines[6], nWidth);
			stosHSmoothLinex15(pInLine + (y + 0)*nWidth, pHLines[7], nWidth);
			stosHSmoothLinex15(pInLine + (y + 1)*nWidth, pHLines[8], nWidth);
			stosHSmoothLinex15(pInLine + (y + 2)*nWidth, pHLines[9], nWidth);
			stosHSmoothLinex15(pInLine + (y + 3)*nWidth, pHLines[10], nWidth);
			stosHSmoothLinex15(pInLine + (y + 4)*nWidth, pHLines[11], nWidth);
			stosHSmoothLinex15(pInLine + (y + 5)*nWidth, pHLines[12], nWidth);
			stosHSmoothLinex15(pInLine + (y + 6)*nWidth, pHLines[13], nWidth);
			loop++;
		}
		int yy = y + 7;
		if (yy >= nHeight) yy -= 7;
		stosHSmoothLinex15(pInLine + yy * nWidth, pHLines[14], nWidth);
		stosVSmoothLinex15(pHLines, pOutLine + y * nWidth, nWidth);
		float  *pTemp0 = pHLines[0];
		pHLines[0] = pHLines[1];
		pHLines[1] = pHLines[2];
		pHLines[2] = pHLines[3];
		pHLines[3] = pHLines[4];
		pHLines[4] = pHLines[5];
		pHLines[5] = pHLines[6];
		pHLines[6] = pHLines[7];
		pHLines[7] = pHLines[8];
		pHLines[8] = pHLines[9];
		pHLines[9] = pHLines[10];
		pHLines[10] = pHLines[11];
		pHLines[11] = pHLines[12];
		pHLines[12] = pHLines[13];
		pHLines[13] = pHLines[14];
		pHLines[14] = pTemp0;
	}
	delete[] pBuffer;
	return true;
}