#ifndef __Common_function_H__
#define __Common_function_H__
#include "Basicdef.h"
#include "SubFunction.h"
const short nMask[25] = {
	1,	4,	6,	4,	1,
	4, 16, 24, 16,	4,
	6, 24, 36, 24,	6,
	4, 16, 24, 16,	4,
	1,	4,	6,	4,	1
};
typedef struct tagFeaturePoint
{
	int fX;
	int fY;
	float fScore;
	int nFlag;
}TFeaturePoint;
extern bool AnalysisHistIsSingleColor(int nHistHue[], int nMaxValue, int nHistRangeNumThre, int nHistRange, unsigned int nRealHistRangeNum);
extern bool AnalysisRGBHistIsSingleColor(unsigned int nHistR[], unsigned int nHistG[], unsigned int nHistB[], int nMaxValue, int nHistRangeNumThre, int nHistRange, unsigned int nRealHistRangeNum[]);
extern void AnalysisRGBHistToMeanY(unsigned int nHistR[], unsigned int nHistG[], unsigned int nHistB[], int nWidth, int nHeight, int nMaxValue, unsigned char &MeanYValue, unsigned char MeanRGBValue[]);
extern void HistIntToHistfloat(int pHist[256], float pfHist[256]);
extern void FindHistSpecification(float SrcHistNum[256], float DstHistNum[256], int map[256]);
extern float HistogramMeanValue(int pHist[256]);
extern void HGaussianLine3(unsigned char *pInLine, unsigned short *pOutLine, int nWidth, int nChannel);
extern void VGaussianLine3(unsigned short *pInLines[3], unsigned char *pOutLine, int nWidth, int nChannel);
extern void HGaussianLine5(unsigned char *pInLine, unsigned short *pOutLine, int nWidth, int nChannel);
extern void VGaussianLine5(unsigned short *pInLines[5], unsigned char *pOutLine, int nWidth, int nChannel);
extern void HDilation3Line(unsigned char *pInLine, unsigned char *pOutLine, int nWidth);
extern void VDilation3Line(unsigned char *pInLine, unsigned char *pOutLine, int nWidth);
extern void Dilation3x3Line(unsigned char *pInLines[], unsigned char *pOutLine, int nWidth);
extern void HErosion3Line(unsigned char *pInLine, unsigned char *pOutLine, int nWidth);
extern void VErosion3Line(unsigned char *pInLine[3], unsigned char *pOutLine, int nWidth);
extern void Erosion3x3Line(unsigned char *pInLines[], unsigned char *pOutLine, int nWidth);
extern void GetImageIntegralData(unsigned char *pInData, unsigned int *Integral, int Width, int Height, int Stride);
extern void GetImageIntegralSData(short *pInData, unsigned int *Integral, int Width, int Height, int Stride);
extern void FillUcharData(unsigned char *pInData, unsigned char *pOutData, int nInWidth, int nInHeight, int nOutWidth, int nOutHeight, int nChannel);
extern void FillWordData(unsigned short *pInData, unsigned short *pOutData, int nInWidth, int nInHeight, int nOutWidth, int nOutHeight, int nChannel);
extern void SubtractUcharEdgeData(unsigned char *pInData0, unsigned char *pInData1, short *pOutData, int nWidth0, int nHeight0, int nWidth1, int nHeight1, int nChannel);
extern void SubtractWordEdgeData(unsigned short *pInImage0, unsigned short *pInImage1, short *pOutImage, int nWidth0, int nHeight0, int nWidth1, int nHeight1, int nChannel);
extern void HDownScaleUcharLinex2(unsigned char *pInLine, unsigned short *pOutLine, int nWidth, int nChannel);
extern void VDownScaleUcharLinex2(unsigned short *pInLines[], unsigned char *pOutLine, int nWidth, int nChannel);
extern bool DownScaleUcharDatax2(unsigned char *pInData, unsigned char *pOutData, int nWidth, int nHeight, int nChannel, bool bDitheringEnable);
extern void HDownScaleWordLinex2(unsigned short *pInLine, unsigned int *pOutLine, int nWidth, int nChannel);
extern void VDownScaleWordLinex2(unsigned int *pInLines[], unsigned short *pOutLine, int nWidth, int nChannel);
extern bool DownScaleWordDatax2(unsigned short *pInData, unsigned short *pOutData, int nWidth, int nHeight, int nChannel, bool bDitheringEnable);

extern void HUpScaleUcharLinex2(unsigned char *pInLine, unsigned short *pOutLine, int nWidth, int nChannel);
extern void VUpScaleUcharLinex2(unsigned short *pInLines[], unsigned char *pOutLine, int nLen);
extern bool UpScaleUcharDatax2(unsigned char *pInData, unsigned char *pOutData, int nWidth, int nHeight, int nChannel, bool bDitheringEnable);
extern void HUpScaleWordLinex2(unsigned short *pInLine, unsigned int *pOutLine, int nWidth, int nChannel);
extern void VUpScaleWordLinex2(unsigned int *pInLines[], unsigned short *pOutLine, int bYFlag, int nWidth, int nChannel);
extern bool UpScaleWordDatax2(unsigned short *pInData, unsigned short *pOutData, int nWidth, int nHeight, int nChannel);
extern void AddBackUcharEdge(unsigned char *pInData, short *pEdgeData, unsigned char *pOutData, int nWidth, int nHeight, int nEdgeWidth, int nEdgeHeight, int nChannel);
extern void AddBackWordEdge(unsigned short *pInImage, short *pEdgeImage, unsigned short *pOutImage, int nWidth, int nHeight, int nEdgeWidth, int nEdgeHeight, int nChannel);
extern inline void CalCubicCoef(double f, short *pCoefs);
extern bool RescaleCubicData(unsigned char *pInImage, unsigned char *pOutImage, int nChannel, int nInWidth, int nInHeight, int nOutWidth, int nOutHeight);
extern void NonMaximumLine(float *pInLines[], int nY, int nWidth, int nPitch, TFeaturePoint *pPtList, int &nPtNum, int nMaxNum);
extern void ComputeCornernessLine(unsigned char *pInLines[], float *pOutLine, int nWidth, int nPitch, float fHarris_K, int nHarrisThre, int nRatio, int nPtThre1, int nPtThre2, int nThre);
extern __inline float ComputeHarrisCorner(unsigned char nYWin[][11], int x, int y, float fHarris_K);
extern void HAvg3Line(unsigned short *pInLine, unsigned short *pOutLine, int nChannel, int nWidth);
extern void VAvg3Line(unsigned short *pInLines[], unsigned short *pOutLine, int nChannel, int nWidth);
extern void HAvg5Line(unsigned short *pInLine, unsigned int *pOutLine, int nChannel, int nWidth);
extern void VAvg5Line(unsigned int *pInLines[], unsigned short *pOutLine, int nChannel, int nWidth);
extern inline void ftofHSmoothlinex7(float *pInLine, float *pOutLine, int nWidth);
extern inline void ftofVSmoothlinex7(float *pInLines[], float *pOutLine, int nWidth);
extern bool FToFSmoothx7(float *pImage, float *pout, int nWidth, int nHeight);
extern bool StoSSmooth7Image(unsigned short *pImage, unsigned short *pout, int nWidth, int nHeight);
extern bool FToFSmoothx15(float *pImage, float *pout, int nWidth, int nHeight);
extern  bool SToSSmoothx15(unsigned short *pImage, unsigned short *pout, int nWidth, int nHeight);
#endif
