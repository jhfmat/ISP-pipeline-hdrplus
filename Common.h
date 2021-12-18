#ifndef __Common_function_H__
#define __Common_function_H__
#include "Basicdef.h"
#include "SubFunction.h"
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
extern void FillLine(unsigned char *pOutLine, unsigned char nVal, int nWidth, int nPitch);
extern void RotateLine(unsigned char *pInLine, unsigned char *pOutLine, int nWidth, int nPitch);
extern void GetImageIntegralData(unsigned char *pInData, unsigned int *Integral, int Width, int Height, int Stride);
extern void FillUcharData(unsigned char *pInData, unsigned char *pOutData, int nInWidth, int nInHeight, int nOutWidth, int nOutHeight, int nChannel);
extern void SubtractUcharEdgeData(unsigned char *pInData0, unsigned char *pInData1, short *pOutData, int nWidth0, int nHeight0, int nWidth1, int nHeight1, int nChannel);
extern void HDownScaleUcharLinex2(unsigned char *pInLine, unsigned short *pOutLine, int nWidth, int nChannel);
extern void VDownScaleUcharLinex2(unsigned short *pInLines[], unsigned char *pOutLine, int nWidth, int nChannel);
extern bool DownScaleUcharDatax2(unsigned char *pInData, unsigned char *pOutData, int nWidth, int nHeight, int nChannel, bool bDitheringEnable);
extern void HUpScaleUcharLinex2(unsigned char *pInLine, unsigned short *pOutLine, int nWidth, int nChannel);
extern void VUpScaleUcharLinex2(unsigned short *pInLines[], unsigned char *pOutLine, int nLen);
extern bool UpScaleUcharDatax2(unsigned char *pInData, unsigned char *pOutData, int nWidth, int nHeight, int nChannel, bool bDitheringEnable);
extern void AddBackUcharEdge(unsigned char *pInData, short *pEdgeData, unsigned char *pOutData, int nWidth, int nHeight, int nEdgeWidth, int nEdgeHeight, int nChannel);
extern inline void CalCubicCoef(double f, short *pCoefs);
extern bool RescaleData(unsigned char *pInImage, unsigned char *pOutImage, int nChannel, int nInWidth, int nInHeight, int nOutWidth, int nOutHeight);
#endif
