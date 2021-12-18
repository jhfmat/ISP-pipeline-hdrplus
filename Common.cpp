#include "Common.h"
#include "Basicdef.h"
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
extern void FillLine(unsigned char *pOutLine, unsigned char nVal, int nWidth, int nPitch)
{
	int x;
	for (x = 0; x < nWidth; x++)
	{
		*pOutLine = nVal;
		pOutLine += nPitch;
	}
}
extern void RotateLine(unsigned char *pInLine, unsigned char *pOutLine, int nWidth, int nPitch)
{
	int x;
	for (x = 0; x < nWidth; x++)
	{
		*pOutLine = *(pInLine++);
		pOutLine += nPitch;
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
			vst1q_u16((unsigned short *)pOut, vout);
			pOut += 8;
			vout = vsubl_u8(vIn0[1], vIn1[1]);
			vst1q_u16((unsigned short *)pOut, vout);
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
	int i, x, Y;
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
extern bool RescaleData(unsigned char *pInImage, unsigned char *pOutImage, int nChannel, int nInWidth, int nInHeight, int nOutWidth, int nOutHeight)
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