#include "YUVToJpeg.h"
#include <stdio.h>
#include "math.h"
#include <stdlib.h>

#define CONST_BITS  13
#define PASS1_BITS  2
#define DCTSIZE 8

#define FIX_0_298631336  ( 2446)	/* FIX(0.298631336) */
#define FIX_0_390180644  ( 3196)	/* FIX(0.390180644) */
#define FIX_0_541196100  ( 4433)	/* FIX(0.541196100) */
#define FIX_0_765366865  ( 6270)	/* FIX(0.765366865) */
#define FIX_0_899976223  ( 7373)	/* FIX(0.899976223) */
#define FIX_1_175875602  ( 9633)	/* FIX(1.175875602) */
#define FIX_1_501321110  ( 12299)	/* FIX(1.501321110) */
#define FIX_1_847759065  ( 15137)	/* FIX(1.847759065) */
#define FIX_1_961570560  ( 16069)	/* FIX(1.961570560) */
#define FIX_2_053119869  ( 16819)	/* FIX(2.053119869) */
#define FIX_2_562915447  ( 20995)	/* FIX(2.562915447) */
#define FIX_3_072711026  ( 25172)	/* FIX(3.072711026) */

/*leave the results scaled up
   * by an overall factor of 8.*/

void fDCT8x8(int *pIn, int *pOut)
{
	int tmp0, tmp1, tmp2, tmp3;
	int tmp10, tmp11, tmp12, tmp13;
	int z1;
	int ctr;
	int workspace[64];

	/* Pass 1: process rows. */
	int half = (1 << (CONST_BITS - PASS1_BITS - 1));
	for (ctr = 0; ctr < DCTSIZE; ctr++)
	{
		int *pData = pIn + ctr * 8;
		int *pCoef = workspace + ctr * 8;

		tmp0 = pData[0] + pData[7];
		tmp1 = pData[1] + pData[6];
		tmp2 = pData[2] + pData[5];
		tmp3 = pData[3] + pData[4];

		tmp10 = tmp0 + tmp3;
		tmp12 = tmp0 - tmp3;
		tmp11 = tmp1 + tmp2;
		tmp13 = tmp1 - tmp2;

		tmp0 = pData[0] - pData[7];
		tmp1 = pData[1] - pData[6];
		tmp2 = pData[2] - pData[5];
		tmp3 = pData[3] - pData[4];

		pCoef[0] = (tmp10 + tmp11) << PASS1_BITS;	//DC
		pCoef[4] = (tmp10 - tmp11) << PASS1_BITS;

		z1 = (tmp12 + tmp13)*FIX_0_541196100;
		z1 += half;
		pCoef[2] = (z1 + tmp12 * FIX_0_765366865) >> (CONST_BITS - PASS1_BITS);
		pCoef[6] = (z1 - tmp13 * FIX_1_847759065) >> (CONST_BITS - PASS1_BITS);


		tmp10 = tmp0 + tmp3;
		tmp11 = tmp1 + tmp2;
		tmp12 = tmp0 + tmp2;
		tmp13 = tmp1 + tmp3;
		z1 = (tmp12 + tmp13)*FIX_1_175875602; /*  c3 */
		z1 += half;

		tmp0 = (tmp0*FIX_1_501321110);    /*  c1+c3-c5-c7 */
		tmp1 = (tmp1*FIX_3_072711026);    /*  c1+c3+c5-c7 */
		tmp2 = (tmp2*FIX_2_053119869);    /*  c1+c3-c5+c7 */
		tmp3 = (tmp3*FIX_0_298631336);    /* -c1+c3+c5-c7 */
		tmp10 = (tmp10*(-FIX_0_899976223));    /*  c7-c3 */
		tmp11 = (tmp11*(-FIX_2_562915447));    /* -c1-c3 */
		tmp12 = (tmp12*(-FIX_0_390180644));    /*  c5-c3 */
		tmp13 = (tmp13*(-FIX_1_961570560));    /* -c3-c5 */

		tmp12 += z1;
		tmp13 += z1;

		pCoef[1] = (tmp0 + tmp10 + tmp12) >> (CONST_BITS - PASS1_BITS);
		pCoef[3] = (tmp1 + tmp11 + tmp13) >> (CONST_BITS - PASS1_BITS);
		pCoef[5] = (tmp2 + tmp11 + tmp12) >> (CONST_BITS - PASS1_BITS);
		pCoef[7] = (tmp3 + tmp10 + tmp13) >> (CONST_BITS - PASS1_BITS);
	}

	half = (1 << (CONST_BITS + PASS1_BITS - 1));
	for (ctr = 0; ctr < DCTSIZE; ctr++)
	{
		int *pCoef = workspace + ctr;
		int *dataptr = pOut + ctr;

		tmp0 = pCoef[DCTSIZE * 0] + pCoef[DCTSIZE * 7];
		tmp1 = pCoef[DCTSIZE * 1] + pCoef[DCTSIZE * 6];
		tmp2 = pCoef[DCTSIZE * 2] + pCoef[DCTSIZE * 5];
		tmp3 = pCoef[DCTSIZE * 3] + pCoef[DCTSIZE * 4];

		tmp10 = tmp0 + tmp3 + (1 << (PASS1_BITS - 1));
		tmp12 = tmp0 - tmp3;
		tmp11 = tmp1 + tmp2;
		tmp13 = tmp1 - tmp2;

		tmp0 = pCoef[DCTSIZE * 0] - pCoef[DCTSIZE * 7];
		tmp1 = pCoef[DCTSIZE * 1] - pCoef[DCTSIZE * 6];
		tmp2 = pCoef[DCTSIZE * 2] - pCoef[DCTSIZE * 5];
		tmp3 = pCoef[DCTSIZE * 3] - pCoef[DCTSIZE * 4];

		dataptr[DCTSIZE * 0] = (tmp10 + tmp11) >> PASS1_BITS;
		dataptr[DCTSIZE * 4] = (tmp10 - tmp11) >> PASS1_BITS;

		z1 = (tmp12 + tmp13)*FIX_0_541196100;
		z1 += half;

		dataptr[DCTSIZE * 2] = (z1 + tmp12 * FIX_0_765366865) >> (CONST_BITS + PASS1_BITS);
		dataptr[DCTSIZE * 6] = (z1 - tmp13 * FIX_1_847759065) >> (CONST_BITS + PASS1_BITS);

		tmp10 = tmp0 + tmp3;
		tmp11 = tmp1 + tmp2;
		tmp12 = tmp0 + tmp2;
		tmp13 = tmp1 + tmp3;

		z1 = (tmp12 + tmp13)*FIX_1_175875602; /*  c3 */
		z1 += half;

		tmp0 = (tmp0*FIX_1_501321110);    /*  c1+c3-c5-c7 */
		tmp1 = (tmp1*FIX_3_072711026);    /*  c1+c3+c5-c7 */
		tmp2 = (tmp2*FIX_2_053119869);    /*  c1+c3-c5+c7 */
		tmp3 = (tmp3*FIX_0_298631336);    /* -c1+c3+c5-c7 */
		tmp10 = (tmp10*(-FIX_0_899976223));    /*  c7-c3 */
		tmp11 = (tmp11*(-FIX_2_562915447));    /* -c1-c3 */
		tmp12 = (tmp12*(-FIX_0_390180644));    /*  c5-c3 */
		tmp13 = (tmp13*(-FIX_1_961570560));    /* -c3-c5 */

		tmp12 += z1;
		tmp13 += z1;

		dataptr[DCTSIZE * 1] = (tmp0 + tmp10 + tmp12) >> (CONST_BITS + PASS1_BITS);
		dataptr[DCTSIZE * 3] = (tmp1 + tmp11 + tmp13) >> (CONST_BITS + PASS1_BITS);
		dataptr[DCTSIZE * 5] = (tmp2 + tmp11 + tmp12) >> (CONST_BITS + PASS1_BITS);
		dataptr[DCTSIZE * 7] = (tmp3 + tmp10 + tmp13) >> (CONST_BITS + PASS1_BITS);
	}
}

void upscale(unsigned char *pInLine, unsigned char *pOut, int nx1, int ny1)
{
	int snx = nx1 * 2;
	for (int  j = 0; j < ny1; j++)
	{
		int tmpy = snx*(2 * j);
		for (int  i = 0; i < nx1; i++)
		{
			int tmp = i + j*nx1;
			int tmp1 = 2 * i + tmpy;
			pOut[tmp1] = pInLine[tmp];
			pOut[tmp1 + 1] = pInLine[tmp];
			pOut[tmp1 + snx] = pInLine[tmp];
			pOut[tmp1 + snx + 1] = pInLine[tmp];
		}
	}
}
void DivBuff(unsigned char* pBuf,int width,int height)
{
	int xlen = width/ 8;
	int	ylen = height/ 8;
	int line8 = width * 8;
	unsigned char line8sram[48000];//6000*8;
	for (int y = 0; y < ylen; ++y)
	{
		int n = 0;  
		for (int x = 0; x < xlen; ++x)
		{   
			int bufOffset = y * width*8 + x * 8;//y和x都是8个每次移动
			for (int k = 0; k < 8; ++k)
			{
				memcpy(line8sram+n,pBuf+bufOffset, 8);
				n += 8;
				bufOffset += width;
			}
		}
		//处理完了8行数据后更新下原来的buf
		memcpy(pBuf +y*line8, line8sram, line8);
	} 
}
int QualityScaling(int quality)
{
	if (quality <= 0) quality = 1;
	if (quality > 100) quality = 100;
	if (quality < 50)
		quality = 5000 / quality;
	else
		quality = 200 - quality * 2;
	return quality;
}
void SetQuantTable(unsigned char* std_QT,unsigned char* QT, int Q)
{
	for (int i = 0; i < 64; ++i)
	{
		int tmpVal = (std_QT[i] * Q + 50L) / 100L;//乘以的Q越大则生成的QT值就越大则量化就越大，在将量化表zig字形排列
		if (tmpVal < 1)           
		{
			tmpVal = 1L;
		}
		if (tmpVal > 255)
		{
			tmpVal = 255L;
		}
		QT[FZBT[i]] = (unsigned char)tmpVal;
	}
}
void InitQTForAANDCT(JPEGINFO *pJpgInfo)
{
	//dct表由y uv 量化表来产生
	unsigned int k = 0; 
	for (int i = 0; i < 8; i++)  
	{
		for (int j = 0; j < 8; j++)
		{
			pJpgInfo->YQT_DCT[k] = (float) (1.0 / ((double) pJpgInfo->YQT[FZBT[k]] *aanScaleFactor[i] * aanScaleFactor[j] * 8.0));       
			++k;
		}
	} 
	k = 0;
	for (int i = 0; i < 8; i++)  
	{
		for (int j = 0; j < 8; j++)
		{
			pJpgInfo->UVQT_DCT[k] = (float) (1.0 / ((double) pJpgInfo->UVQT[FZBT[k]] *aanScaleFactor[i] * aanScaleFactor[j] * 8.0));       
			++k;
		}
	} 
}
unsigned char ComputeVLI(short val)
{ 
	unsigned char binStrLen = 0;
	val = abs(val); 
	if(val == 1)
	{
		binStrLen = 1;  
	}
	else if(val >= 2 && val <= 3)
	{
		binStrLen = 2;
	}
	else if(val >= 4 && val <= 7)
	{
		binStrLen = 3;
	}
	else if(val >= 8 && val <= 15)
	{
		binStrLen = 4;
	}
	else if(val >= 16 && val <= 31)
	{
		binStrLen = 5;
	}
	else if(val >= 32 && val <= 63)
	{
		binStrLen = 6;
	}
	else if(val >= 64 && val <= 127)
	{
		binStrLen = 7;
	}
	else if(val >= 128 && val <= 255)
	{
		binStrLen = 8;
	}
	else if(val >= 256 && val <= 511)
	{
		binStrLen = 9;
	}
	else if(val >= 512 && val <= 1023)
	{
		binStrLen = 10;
	}
	else if(val >= 1024 && val <= 2047)
	{
		binStrLen = 11;
	}
	return binStrLen;
}
void BuildVLITable(JPEGINFO *pJpgInfo)
{
	short i   = 0;
	for (i = 0; i < DC_MAX_QUANTED; ++i)
	{
		pJpgInfo->pVLITAB[i] = ComputeVLI(i);
	}
	for (i = DC_MIN_QUANTED; i < 0; ++i)
	{
		pJpgInfo->pVLITAB[i] = ComputeVLI(i);
	}
}
int WriteSOI(unsigned char* pOut,int nDataLen)
{ 
	memcpy(pOut+nDataLen,&SOITAG,sizeof(SOITAG));
	return nDataLen+sizeof(SOITAG);
}
int WriteEOI(unsigned char* pOut,int nDataLen)
{
	memcpy(pOut+nDataLen,&EOITAG,sizeof(EOITAG));
	return nDataLen + sizeof(EOITAG);
}
int WriteAPP0(unsigned char* pOut,int nDataLen)
{
	JPEGAPP0 APP0;
	APP0.segmentTag  = 0xE0FF;
	APP0.length    = 0x1000;
	APP0.id[0]    = 'J';
	APP0.id[1]    = 'F';
	APP0.id[2]    = 'I';
	APP0.id[3]    = 'F';
	APP0.id[4]    = 0;
	APP0.ver     = 0x0101;
	APP0.densityUnit = 0x00;
	APP0.densityX   = 0x0100;
	APP0.densityY   = 0x0100;
	APP0.thp     = 0x00;
	APP0.tvp     = 0x00;
	memcpy(pOut+nDataLen,&APP0.segmentTag,2);
	memcpy(pOut+nDataLen+2,&APP0.length,2);
	memcpy(pOut+nDataLen+4,APP0.id,5);
	memcpy(pOut+nDataLen+9,&APP0.ver,2);
	*(pOut+nDataLen+11) = APP0.densityUnit;
	memcpy(pOut+nDataLen+12,&APP0.densityX,2);
	memcpy(pOut+nDataLen+14,&APP0.densityY,2);
	*(pOut+nDataLen+16) = APP0.thp;
	*(pOut+nDataLen+17) = APP0.tvp;
	return nDataLen + sizeof(APP0)-2;
}
int WriteDQT(JPEGINFO *pJpgInfo,unsigned char* pOut,int nDataLen)
{
	JPEGDQT_8BITS DQT_Y;
	DQT_Y.segmentTag = 0xDBFF;
	DQT_Y.length   = 0x4300;
	DQT_Y.tableInfo  = 0x00;
	for (int i = 0; i < 64; i++)
	{
		DQT_Y.table[i] = pJpgInfo->YQT[i];
	}    
	memcpy(pOut+nDataLen,&DQT_Y.segmentTag,2);
	memcpy(pOut+nDataLen+2,&DQT_Y.length,2);
	*(pOut+nDataLen+4) = DQT_Y.tableInfo;
	memcpy(pOut+nDataLen+5,DQT_Y.table,64);
	nDataLen += sizeof(DQT_Y)-1;
	DQT_Y.tableInfo  = 0x01;
	for (int i = 0; i < 64; i++)
	{
		DQT_Y.table[i] = pJpgInfo->UVQT[i];
	}
	memcpy(pOut+nDataLen,&DQT_Y.segmentTag,2);
	memcpy(pOut+nDataLen+2,&DQT_Y.length,2);
	*(pOut+nDataLen+4) = DQT_Y.tableInfo;
	memcpy(pOut+nDataLen+5,DQT_Y.table,64);
	nDataLen += sizeof(DQT_Y)-1;
	return nDataLen;
}
unsigned short Intel2Moto(unsigned short val)
{
	unsigned char highBits = (unsigned char)(val / 256);
	unsigned char lowBits = (unsigned char)(val % 256);
	return lowBits * 256 + highBits;
}
int WriteSOF(unsigned char* pOut,int nDataLen,int width,int height)
{
	JPEGSOF0_24BITS SOF;
	SOF.segmentTag = 0xC0FF;
	SOF.length   = 0x1100;
	SOF.precision  = 0x08;
	SOF.height   = Intel2Moto((unsigned short)height);
	SOF.width    = Intel2Moto((unsigned short)width); 
	SOF.sigNum   = 0x03;
	SOF.YID     = 0x01; 
	SOF.QTY     = 0x00;
	SOF.UID     = 0x02;
	SOF.QTU     = 0x01;
	SOF.VID     = 0x03;
	SOF.QTV     = 0x01;
	SOF.HVU     = 0x11;
	SOF.HVV     = 0x11;
	SOF.HVY   = 0x11;
	memcpy(pOut+nDataLen,&SOF.segmentTag,2);
	memcpy(pOut+nDataLen+2,&SOF.length,2);
	*(pOut+nDataLen+4) = SOF.precision;
	memcpy(pOut+nDataLen+5,&SOF.height,2);
	memcpy(pOut+nDataLen+7,&SOF.width,2);
	*(pOut+nDataLen+9) = SOF.sigNum;
	*(pOut+nDataLen+10) = SOF.YID;
	*(pOut+nDataLen+11) = SOF.HVY;
	*(pOut+nDataLen+12) = SOF.QTY;
	*(pOut+nDataLen+13) = SOF.UID;
	*(pOut+nDataLen+14) = SOF.HVU;
	*(pOut+nDataLen+15) = SOF.QTU;
	*(pOut+nDataLen+16) = SOF.VID;
	*(pOut+nDataLen+17) = SOF.HVV;
	*(pOut+nDataLen+18) = SOF.QTV;
	return nDataLen + sizeof(SOF)-1;
}
int WriteByte(unsigned char val,unsigned char* pOut,int nDataLen)
{   
	pOut[nDataLen] = val;
	return nDataLen + 1;
}
int WriteDHT(unsigned char* pOut,int nDataLen)
{
	JPEGDHT DHT;
	DHT.segmentTag = 0xC4FF;
	DHT.length   = Intel2Moto(19 + 12);
	DHT.tableInfo  = 0x00;
	for (int i = 0; i < 16; i++)
	{
		DHT.huffCode[i] = STD_DC_Y_NRCODES[i + 1];
	} 
	memcpy(pOut+nDataLen,&DHT.segmentTag,2);
	memcpy(pOut+nDataLen+2,&DHT.length,2);
	*(pOut+nDataLen+4) = DHT.tableInfo;
	memcpy(pOut+nDataLen+5,DHT.huffCode,16);
	nDataLen += sizeof(DHT)-1;
	for (int i = 0; i <= 11; i++)
	{
		nDataLen = WriteByte(STD_DC_Y_VALUES[i],pOut,nDataLen);  
	}  
	DHT.tableInfo  = 0x01;
	for (int i = 0; i < 16; i++)
	{
		DHT.huffCode[i] = STD_DC_UV_NRCODES[i + 1];
	}
	memcpy(pOut+nDataLen,&DHT.segmentTag,2);
	memcpy(pOut+nDataLen+2,&DHT.length,2);
	*(pOut+nDataLen+4) = DHT.tableInfo;
	memcpy(pOut+nDataLen+5,DHT.huffCode,16);
	nDataLen += sizeof(DHT)-1;
	for (int i = 0; i <= 11; i++)
	{
		nDataLen = WriteByte(STD_DC_UV_VALUES[i],pOut,nDataLen);  
	} 
	DHT.length   = Intel2Moto(19 + 162);
	DHT.tableInfo  = 0x10;
	for (int i = 0; i < 16; i++)
	{
		DHT.huffCode[i] = STD_AC_Y_NRCODES[i + 1];
	}
	memcpy(pOut+nDataLen,&DHT.segmentTag,2);
	memcpy(pOut+nDataLen+2,&DHT.length,2);
	*(pOut+nDataLen+4) = DHT.tableInfo;
	memcpy(pOut+nDataLen+5,DHT.huffCode,16);
	nDataLen += sizeof(DHT)-1;
	for (int i = 0; i <= 161; i++)
	{
		nDataLen = WriteByte(STD_AC_Y_VALUES[i],pOut,nDataLen);  
	}  
	DHT.tableInfo  = 0x11;
	for (int i = 0; i < 16; i++)
	{
		DHT.huffCode[i] = STD_AC_UV_NRCODES[i + 1];
	}
	memcpy(pOut+nDataLen,&DHT.segmentTag,2);
	memcpy(pOut+nDataLen+2,&DHT.length,2);
	*(pOut+nDataLen+4) = DHT.tableInfo;
	memcpy(pOut+nDataLen+5,DHT.huffCode,16);
	nDataLen += sizeof(DHT)-1;
	for (int i = 0; i <= 161; i++)
	{
		nDataLen = WriteByte(STD_AC_UV_VALUES[i],pOut,nDataLen);  
	}
	return nDataLen;
}
int WriteSOS(unsigned char* pOut,int nDataLen)
{
	JPEGSOS_24BITS SOS;
	SOS.segmentTag   = 0xDAFF;
	SOS.length    = 0x0C00;
	SOS.sigNum    = 0x03;
	SOS.YID     = 0x01;
	SOS.HTY     = 0x00;
	SOS.UID     = 0x02;
	SOS.HTU     = 0x11;
	SOS.VID     = 0x03;
	SOS.HTV     = 0x11;
	SOS.Se     = 0x3F;
	SOS.Ss     = 0x00;
	SOS.Bf     = 0x00;
	memcpy(pOut+nDataLen,&SOS,sizeof(SOS)); 
	return nDataLen+sizeof(SOS);
}
void BuildSTDHuffTab(unsigned char* nrcodes,unsigned char* stdTab,HUFFCODE* huffCode)
{
	unsigned char k     = 0;
	unsigned short code   = 0; 
	for (int i = 1; i <= 16; i++)
	{ 
		for (int j = 1; j <= nrcodes[i]; j++)
		{   
			huffCode[stdTab[k]].code = code;//code值从0开始
			huffCode[stdTab[k]].length = i;//len长度从1开始
			++k;
			++code;
		}
		code*=2;
	} 
	for (int i = 0; i < k; i++)
	{
		huffCode[i].val = stdTab[i];  
	}
}
void FDCT(float* lpBuff)
{
	float tmp0, tmp1, tmp2, tmp3, tmp4, tmp5, tmp6, tmp7;
	float tmp10, tmp11, tmp12, tmp13;
	float z1, z2, z3, z4, z5, z11, z13;
	float* dataptr;
	int ctr;
	dataptr = lpBuff;
	for (ctr = 8-1; ctr >= 0; ctr--)
	{
		tmp0 = dataptr[0] + dataptr[7];
		tmp7 = dataptr[0] - dataptr[7];
		tmp1 = dataptr[1] + dataptr[6];
		tmp6 = dataptr[1] - dataptr[6];
		tmp2 = dataptr[2] + dataptr[5];
		tmp5 = dataptr[2] - dataptr[5];
		tmp3 = dataptr[3] + dataptr[4];
		tmp4 = dataptr[3] - dataptr[4];
		
		tmp10 = tmp0 + tmp3; 
		tmp13 = tmp0 - tmp3;
		tmp11 = tmp1 + tmp2;
		tmp12 = tmp1 - tmp2;	
		dataptr[0] = tmp10 + tmp11; /* phase 3 */
		dataptr[4] = tmp10 - tmp11;
		z1 = (float)((tmp12 + tmp13) * (0.707106781)); /* c4 */
		dataptr[2] = tmp13 + z1; /* phase 5 */
		dataptr[6] = tmp13 - z1;
		

		tmp10 = tmp4 + tmp5; /* phase 2 */
		tmp11 = tmp5 + tmp6;
		tmp12 = tmp6 + tmp7;
		z5 = (float)((tmp10 - tmp12) * ( 0.382683433)); /* c6 */
		z2 = (float)((0.541196100) * tmp10 + z5); /* c2-c6 */
		z4 = (float)((1.306562965) * tmp12 + z5); /* c2+c6 */
		z3 = (float)(tmp11 * (0.707106781)); /* c4 */	
		z11 = tmp7 + z3; 
		z13 = tmp7 - z3;	
		dataptr[5] = z13 + z2; /* phase 6 */
		dataptr[3] = z13 - z2;
		dataptr[1] = z11 + z4;
		dataptr[7] = z11 - z4;	
		dataptr += 8; 
	}	
	dataptr = lpBuff;
	for (ctr = 8-1; ctr >= 0; ctr--)
	{
		tmp0 = dataptr[8*0] + dataptr[8*7];
		tmp7 = dataptr[8*0] - dataptr[8*7];
		tmp1 = dataptr[8*1] + dataptr[8*6];
		tmp6 = dataptr[8*1] - dataptr[8*6];
		tmp2 = dataptr[8*2] + dataptr[8*5];
		tmp5 = dataptr[8*2] - dataptr[8*5];
		tmp3 = dataptr[8*3] + dataptr[8*4];
		tmp4 = dataptr[8*3] - dataptr[8*4];
		

		tmp10 = tmp0 + tmp3;
		tmp13 = tmp0 - tmp3;
		tmp11 = tmp1 + tmp2;
		tmp12 = tmp1 - tmp2;	
		dataptr[8*0] = tmp10 + tmp11; /* phase 3 */
		dataptr[8*4] = tmp10 - tmp11;
		
		z1 = (float)((tmp12 + tmp13) * (0.707106781)); /* c4 */
		dataptr[8*2] = tmp13 + z1; /* phase 5 */
		dataptr[8*6] = tmp13 - z1;
		
		tmp10 = tmp4 + tmp5; /* phase 2 */
		tmp11 = tmp5 + tmp6;
		tmp12 = tmp6 + tmp7;
		
		z5 = (float)((tmp10 - tmp12) * (0.382683433)); /* c6 */
		z2 = (float)((0.541196100) * tmp10 + z5); /* c2-c6 */
		z4 = (float)((1.306562965) * tmp12 + z5); /* c2+c6 */
		z3 = (float)(tmp11 * (0.707106781)); /* c4 */
		
		z11 = tmp7 + z3;  /* phase 5 */
		z13 = tmp7 - z3;
		
		dataptr[8*5] = z13 + z2; /* phase 6 */
		dataptr[8*3] = z13 - z2;
		dataptr[8*1] = z11 + z4;
		dataptr[8*7] = z11 - z4;
		++dataptr;  
	}
}
int WriteBitsStream(JPEGINFO *pJpgInfo,unsigned short value,unsigned char codeLen,unsigned char* pOut,int nDataLen)
{ 
	short posval;//bit position in the bitstring we read, should be<=15 and >=0 
	posval=(short)(codeLen-1);
	while (posval>=0)
	{
		if (value & mask[posval])
		{
			pJpgInfo->bytenew|=mask[pJpgInfo->bytepos];
		}
		posval--;
		pJpgInfo->bytepos--;
		if (pJpgInfo->bytepos<0) 
		{ 
			if (pJpgInfo->bytenew==0xFF)
			{
				nDataLen = WriteByte(0xFF,pOut,nDataLen);
				nDataLen = WriteByte(0,pOut,nDataLen);
			}
			else
			{
				nDataLen = WriteByte(pJpgInfo->bytenew,pOut,nDataLen);
			}
			pJpgInfo->bytepos=7;
			pJpgInfo->bytenew=0;
		}
	}
	return nDataLen;
}
int WriteBits(JPEGINFO *pJpgInfo,HUFFCODE huffCode,unsigned char* pOut,int nDataLen)
{  
	return WriteBitsStream(pJpgInfo,huffCode.code,huffCode.length,pOut,nDataLen); 
}
int WriteBits2(JPEGINFO *pJpgInfo,SYM2 sym,unsigned char* pOut,int nDataLen)
{
	return WriteBitsStream(pJpgInfo,sym.amplitude,sym.codeLen,pOut,nDataLen); 
}
double mypow(double x,double y)
{
	int i=0;
	double sum=1;
	for(i=1;i<=(int)y;i++)
		sum *=x;
	return sum;
}
SYM2 BuildSym2(short value)
{
	SYM2 Symbol;  	
	Symbol.codeLen = ComputeVLI(value);           
	Symbol.amplitude = 0;
	if (value >= 0)
	{
		Symbol.amplitude = value;
	}
	else
	{
		double tmp = mypow(2,Symbol.codeLen);
		Symbol.amplitude = (short)(tmp-1) + value; 
	}
	return Symbol;
}
void RLEComp(short* lpbuf,ACSYM* lpOutBuf,unsigned char *resultLen)
{  
	unsigned char zeroNum     = 0;
	unsigned int EOBPos      = 0;
	unsigned char MAXZEROLEN = 15;
	unsigned int i        = 0;    
	unsigned int j        = 0;
	EOBPos = 64 - 1;
	for (i = EOBPos; i > 0; i--) //记录最后一个非零的位子
	{
		if (lpbuf[i] == 0) 
		{
			--EOBPos; 
		}
		else     
		{
			break;                   
		}
	}
	//RLE编码 如0,0,0,-6 RLE=(3,-6) BIT=(3,3, 001)
	for (i = 1; i <= EOBPos; i++) //统计去掉dc后的1个数到ac中最后一个非0位子来进行编码
	{
		if (lpbuf[i] == 0 && zeroNum < MAXZEROLEN)//遇到0就统计个数，遇到非零就编码就到一个对于的
		{
			++zeroNum;   
		}
		else
		{   
			lpOutBuf[j].zeroLen = zeroNum; //0的个数
			lpOutBuf[j].codeLen = ComputeVLI(lpbuf[i]);//码长
			lpOutBuf[j].amplitude = lpbuf[i];//ac振幅 
			zeroNum = 0;     
			++(*resultLen);//jpeg码长增加一个
			++j;
		}
	} 
}
int ProcessDU(JPEGINFO *pJpgInfo, float* lpBuf,float* quantTab,HUFFCODE* dcHuffTab,HUFFCODE* acHuffTab,short* DC,unsigned char* pOut,int nDataLen)
{
	unsigned char i = 0;        
	unsigned int j = 0;
	short diffVal = 0;              
	unsigned char acLen  = 0;               
	short sigBuf[64];
	ACSYM acSym[64];
	FDCT(lpBuf);//先做dct离散变换在做量化
	for (i = 0; i < 64; i++)
	{          
		sigBuf[FZBT[i]] = (short)((lpBuf[i] * quantTab[i] + 16384.5) - 16384);//将离散化的块乘以量dct量化表
	}
	diffVal = sigBuf[0] - *DC;//每个8x8方块（第一个值低频）后面和前面一个的差=====减去dc直流信号获取交流信号 将交流信号进行编码
	*DC = sigBuf[0];//更新dc直流
	//两个块直流的差 编码直流
	if (diffVal == 0)//
	{  
		nDataLen = WriteBits(pJpgInfo,dcHuffTab[0],pOut,nDataLen); //记录pOut里面的nDataLen长度
	}
	else
	{   
		nDataLen = WriteBits(pJpgInfo,dcHuffTab[pJpgInfo->pVLITAB[diffVal]],pOut,nDataLen);  
		nDataLen = WriteBits2(pJpgInfo,BuildSym2(diffVal),pOut,nDataLen);    
	}
	//编码交流
	for (i = 63; (i > 0) && (sigBuf[i] == 0); i--)//最后一个非0的位子
	{		
	}
	if (i == 0)//全部为0的编码
	{
		nDataLen = WriteBits(pJpgInfo,acHuffTab[0x00],pOut,nDataLen);       
	}
	else
	{ 
		RLEComp(sigBuf,&acSym[0],&acLen); //RLE编码 如0,0,0,-6 RLE=(3,-6) （3表示0个个数，-6表示0后面的数值）     BIT=(3,3, 001)
		//将RLE码转换BIT码 BIT=(3,3, 001)（第一个3表示0个的个数，第二个3表示001的位数）编码后变成了(0x33,001)
		//在进行哈夫曼编码查表 
		for (j = 0; j < acLen; j++)          
		{   
			if (acSym[j].codeLen == 0)  
			{   
				nDataLen = WriteBits(pJpgInfo,acHuffTab[0xF0],pOut,nDataLen);//WriteBits 哈夫曼编码
			}
			else
			{
				nDataLen = WriteBits(pJpgInfo,acHuffTab[acSym[j].zeroLen * 16 + acSym[j].codeLen],pOut,nDataLen);
				nDataLen = WriteBits2(pJpgInfo,BuildSym2(acSym[j].amplitude),pOut,nDataLen);    
			}   
		}
		if (i != 63)         
		{
			nDataLen = WriteBits(pJpgInfo,acHuffTab[0x00],pOut,nDataLen);
		}
	}
	return nDataLen;
}
int ProcessData(JPEGINFO *pJpgInfo,unsigned char* lpYBuf,unsigned char* lpUBuf,unsigned char* lpVBuf,int width,int height,unsigned char* pOut,int nDataLen)//out为jpeg码流，nDataLen为码流长度
{ 
	unsigned int  BufLen = width* height;
	float dctYBuf[64];
	float dctUBuf[64];
	float dctVBuf[64];
	short yDC   = 0;          
	short uDC   = 0;        
	short vDC   = 0;       
	unsigned char yCounter  = 0;     
	unsigned char uCounter  = 0;
	unsigned char vCounter  = 0;
	unsigned int y    = 0;                      
	unsigned int u    = 0;
	unsigned int v    = 0; 
	unsigned int blocknum = (height * width)/(64);
	for (int p = 0;p < blocknum; p++)
	{
		yCounter = 1;
		uCounter = 1;
		vCounter = 1;
		for (; y < BufLen; y += 64)
		{
			for (int j = 0; j < 64; j++)
			{
				dctYBuf[j] = (float)(lpYBuf[y + j] - 128);//减去128是应为余弦离散函数是在正负128变化
			}   
			if (yCounter > 0)
			{    
				--yCounter;
				nDataLen = ProcessDU(pJpgInfo,dctYBuf,pJpgInfo->YQT_DCT,pJpgInfo->STD_DC_Y_HT,pJpgInfo->STD_AC_Y_HT,&yDC,pOut,nDataLen);
			}
			else
			{
				break;
			}
		}  
		for (; u < BufLen; u += 64)
		{
			for (int n = 0; n < 64; n++)
			{
				dctUBuf[n] = (float)(lpUBuf[u + n] - 128);
			}    
			if (uCounter > 0)
			{    
				--uCounter;
				nDataLen = ProcessDU(pJpgInfo,dctUBuf,pJpgInfo->UVQT_DCT,pJpgInfo->STD_DC_UV_HT,pJpgInfo->STD_AC_UV_HT,&uDC,pOut,nDataLen);
			}
			else
			{
				break;
			}
		}  
		for (; v < BufLen; v += 64)
		{
			for (int k = 0; k < 64; k++)
			{
				dctVBuf[k] = (float)(lpVBuf[v + k] - 128);
			}
			if (vCounter > 0)
			{
				--vCounter;
				nDataLen = ProcessDU(pJpgInfo,dctVBuf,pJpgInfo->UVQT_DCT,pJpgInfo->STD_DC_UV_HT,pJpgInfo->STD_AC_UV_HT,&vDC,pOut,nDataLen);
			}
			else
			{
				break;
			}
		}  
	} 
	return nDataLen;
}
int YUV2Jpg(unsigned char* in_Y,unsigned char* in_U,unsigned char* in_V,int width,int height,int quality,unsigned char* pOut,unsigned long *pnOutSize)
{
	unsigned char *pYBuf = (unsigned char*)malloc(width * height);
	unsigned char *pUBuf = (unsigned char*)malloc(width * height);
	unsigned char *pVBuf = (unsigned char*)malloc(width * height);
	memcpy(pYBuf,in_Y, width * height);
	upscale(in_U, pUBuf, width/2, height/2);
	upscale(in_V, pVBuf, width/2, height/2);
	DivBuff(pYBuf,width,height);
	DivBuff(pUBuf,width,height);
	DivBuff(pVBuf,width,height);
	quality = QualityScaling(quality);
	JPEGINFO JpgInfo;
	memset(&JpgInfo, 0, sizeof(JPEGINFO));
	JpgInfo.bytenew = 0;
	JpgInfo.bytepos = 7;
	SetQuantTable(std_Y_QT,JpgInfo.YQT, quality);//根据质量参数生成亮度量化表
	SetQuantTable(std_UV_QT,JpgInfo.UVQT,quality);//根据质量参数生成色度量化表
	InitQTForAANDCT(&JpgInfo);//根据上面的生成的量化表在生成dct余弦离散质量表
	JpgInfo.pVLITAB=JpgInfo.VLI_TAB +2048;  
	BuildVLITable(&JpgInfo);//创建pVLITAB 数字对于的bit需要的位数
	int nDataLen = 0;
	nDataLen = WriteSOI(pOut,nDataLen); 
	nDataLen = WriteAPP0(pOut,nDataLen);
	nDataLen = WriteDQT(&JpgInfo,pOut,nDataLen);
	nDataLen = WriteSOF(pOut,nDataLen,width,height);
	nDataLen = WriteDHT(pOut,nDataLen);
	nDataLen = WriteSOS(pOut,nDataLen);
	BuildSTDHuffTab(STD_DC_Y_NRCODES,STD_DC_Y_VALUES,JpgInfo.STD_DC_Y_HT);
	BuildSTDHuffTab(STD_AC_Y_NRCODES,STD_AC_Y_VALUES,JpgInfo.STD_AC_Y_HT);
	BuildSTDHuffTab(STD_DC_UV_NRCODES,STD_DC_UV_VALUES,JpgInfo.STD_DC_UV_HT);
	BuildSTDHuffTab(STD_AC_UV_NRCODES,STD_AC_UV_VALUES,JpgInfo.STD_AC_UV_HT);
	nDataLen = ProcessData(&JpgInfo,pYBuf,pUBuf,pVBuf,width,height,pOut,nDataLen);  
	nDataLen = WriteEOI(pOut,nDataLen);	
	free(pYBuf);
	free(pUBuf);
	free(pVBuf);
	*pnOutSize = nDataLen;
	return 0;
}
