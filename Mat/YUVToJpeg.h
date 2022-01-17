#ifndef _YUV2JPG_H__
#define _YUV2JPG_H__
#include <malloc.h>
#include <string.h>
#define DC_MAX_QUANTED 2047
#define DC_MIN_QUANTED -2048
typedef struct tagHUFFCODE
{
	unsigned short code;
	unsigned char length;
	unsigned short val;
}HUFFCODE;
typedef struct tagJPEGINFO
{
	float YQT_DCT[64];
	float UVQT_DCT[64];
	unsigned char YQT[64];
	unsigned char UVQT[64];
	unsigned char VLI_TAB[4096];
	unsigned char* pVLITAB;
	HUFFCODE STD_DC_Y_HT[12];
	HUFFCODE STD_DC_UV_HT[12];
	HUFFCODE STD_AC_Y_HT[256];
	HUFFCODE STD_AC_UV_HT[256];
	unsigned char bytenew;
	short bytepos;
}JPEGINFO;	
static unsigned short SOITAG = 0xD8FF;
static unsigned short EOITAG = 0xD9FF;
static unsigned char FZBT[64]={
	0, 1, 5, 6, 14,15,27,28,
	2, 4, 7, 13,16,26,29,42,
	3, 8, 12,17,25,30,41,43,
	9, 11,18,24,31,40,44,53,
	10,19,23,32,39,45,52,54,
	20,22,33,38,46,51,55,60,
	21,34,37,47,50,56,59,61,
	35,36,48,49,57,58,62,63 
};
//亮度量化表
static unsigned char std_Y_QT[64]={
	16, 11, 10, 16, 24, 40, 51, 61,
	12, 12, 14, 19, 26, 58, 60, 55,
	14, 13, 16, 24, 40, 57, 69, 56,
	14, 17, 22, 29, 51, 87, 80, 62,
	18, 22, 37, 56, 68, 109,103,77,
	24, 35, 55, 64, 81, 104,113,92,
	49, 64, 78, 87, 103,121,120,101,
	72, 92, 95, 98, 112,100,103,99
};
//色度量化表
static unsigned char std_UV_QT[64]={
	17, 18, 24, 47, 99, 99, 99, 99,
	18, 21, 26, 66, 99, 99, 99, 99,
	24, 26, 56, 99, 99, 99, 99, 99,
	47, 66, 99 ,99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99,
	99, 99, 99, 99, 99, 99, 99, 99
};
//缩放因子
static double aanScaleFactor[8]={1.0, 1.387039845, 1.306562965, 1.175875602,1.0, 0.785694958, 0.541196100, 0.275899379};
static unsigned char STD_DC_Y_NRCODES[17]={0,0,1,5,1,1,1,1,1,1,0,0,0,0,0,0,0};
static unsigned char STD_DC_Y_VALUES[12]={0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
static unsigned char STD_DC_UV_NRCODES[17]={0,0,3,1,1,1,1,1,1,1,1,1,0,0,0,0,0};
static unsigned char STD_DC_UV_VALUES[12]={0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11};
static unsigned char STD_AC_Y_NRCODES[17]={0,0,2,1,3,3,2,4,3,5,5,4,4,0,0,1,0X7D};
static unsigned char STD_AC_Y_VALUES[162]={
	0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12,
		0x21, 0x31, 0x41, 0x06, 0x13, 0x51, 0x61, 0x07,
		0x22, 0x71, 0x14, 0x32, 0x81, 0x91, 0xa1, 0x08,
		0x23, 0x42, 0xb1, 0xc1, 0x15, 0x52, 0xd1, 0xf0,
		0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0a, 0x16,
		0x17, 0x18, 0x19, 0x1a, 0x25, 0x26, 0x27, 0x28,
		0x29, 0x2a, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
		0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
		0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
		0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
		0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
		0x7a, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
		0x8a, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98,
		0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
		0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6,
		0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3, 0xc4, 0xc5,
		0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2, 0xd3, 0xd4,
		0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda, 0xe1, 0xe2,
		0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9, 0xea,
		0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
		0xf9, 0xfa
};
static unsigned char STD_AC_UV_NRCODES[17]={0,0,2,1,2,4,4,3,4,7,5,4,4,0,1,2,0X77};
static unsigned char STD_AC_UV_VALUES[162]={
	0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21,
			0x31, 0x06, 0x12, 0x41, 0x51, 0x07, 0x61, 0x71,
			0x13, 0x22, 0x32, 0x81, 0x08, 0x14, 0x42, 0x91,
			0xa1, 0xb1, 0xc1, 0x09, 0x23, 0x33, 0x52, 0xf0,
			0x15, 0x62, 0x72, 0xd1, 0x0a, 0x16, 0x24, 0x34,
			0xe1, 0x25, 0xf1, 0x17, 0x18, 0x19, 0x1a, 0x26,
			0x27, 0x28, 0x29, 0x2a, 0x35, 0x36, 0x37, 0x38,
			0x39, 0x3a, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48,
			0x49, 0x4a, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58,
			0x59, 0x5a, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
			0x69, 0x6a, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78,
			0x79, 0x7a, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
			0x88, 0x89, 0x8a, 0x92, 0x93, 0x94, 0x95, 0x96,
			0x97, 0x98, 0x99, 0x9a, 0xa2, 0xa3, 0xa4, 0xa5,
			0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xb2, 0xb3, 0xb4,
			0xb5, 0xb6, 0xb7, 0xb8, 0xb9, 0xba, 0xc2, 0xc3,
			0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9, 0xca, 0xd2,
			0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9, 0xda,
			0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
			0xea, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8,
			0xf9, 0xfa
};
static unsigned short mask[16]={
	1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768
};
typedef struct tagJPEGAPP0{
	unsigned short segmentTag;
	unsigned short length;
	char id[5];
	unsigned short ver;
	unsigned char densityUnit;
	unsigned short densityX;
	unsigned short densityY;
	unsigned char thp;
	unsigned char tvp;
}JPEGAPP0;
typedef struct tagJPEGDQT_8BITS{
	unsigned short segmentTag;
	unsigned short length;
	unsigned char tableInfo;
	unsigned char table[64];
}JPEGDQT_8BITS;
typedef struct tagJPEGSOF0_24BITS{
	unsigned short segmentTag;
	unsigned short length;
	unsigned char precision;
	unsigned short height;
	unsigned short width;
	unsigned char sigNum;
	unsigned char YID;
	unsigned char HVY;
	unsigned char QTY;
	unsigned char UID;
	unsigned char HVU;
	unsigned char QTU;
	unsigned char VID;
	unsigned char HVV;
	unsigned char QTV;
}JPEGSOF0_24BITS;
typedef struct tagJPEGDHT{
	unsigned short segmentTag;
	unsigned short length;
	unsigned char tableInfo;
	unsigned char huffCode[16];
}JPEGDHT;
typedef struct tagJPEGSOS_24BITS{
	unsigned short segmentTag;
	unsigned short length;
	unsigned char sigNum;
	unsigned char YID;
	unsigned char HTY;
	unsigned char UID;
	unsigned char HTU;
	unsigned char VID;
	unsigned char HTV;
	unsigned char Ss;
	unsigned char Se;
	unsigned char Bf;
}JPEGSOS_24BITS;
typedef struct tagACSYM{
	unsigned char zeroLen;
	unsigned char codeLen;
	short amplitude;
}ACSYM;
typedef struct tagSYM2{
	short amplitude;
	unsigned char codeLen;
}SYM2;
typedef struct tagBMBUFINFO{
	unsigned int imgWidth;
	unsigned int imgHeight;
	unsigned int buffWidth;
	unsigned int buffHeight;
	unsigned short BitCount;
	unsigned char padSize;
}BMBUFINFO;
int QualityScaling(int quality);
void DivBuff(unsigned char* pBuf,int width,int height);
void SetQuantTable(unsigned char* std_QT,unsigned char* QT, int Q);
void InitQTForAANDCT(JPEGINFO *pJpgInfo);
void BuildVLITable(JPEGINFO *pJpgInfo);
int WriteSOI(unsigned char* pOut,int nDataLen);
int WriteEOI(unsigned char* pOut,int nDataLen);
int WriteAPP0(unsigned char* pOut,int nDataLen);
int WriteDQT(JPEGINFO *pJpgInfo,unsigned char* pOut,int nDataLen);
int WriteSOF(unsigned char* pOut,int nDataLen,int width,int height);
int WriteDHT(unsigned char* pOut,int nDataLen);
int WriteSOS(unsigned char* pOut,int nDataLen);
void BuildSTDHuffTab(unsigned char* nrcodes,unsigned char* stdTab,HUFFCODE* huffCode);
int ProcessData(JPEGINFO *pJpgInfo,unsigned char* lpYBuf,unsigned char* lpUBuf,unsigned char* lpVBuf,int width,int height,unsigned char* pOut,int nDataLen);
int YUV2Jpg(unsigned char* in_Y,unsigned char* in_U,unsigned char* in_V,int width,int height,int quality,unsigned char* pOut,unsigned long*pnOutSize);
#endif
