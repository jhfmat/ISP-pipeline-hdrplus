#ifndef __MULTI_SHORT_IMAGE_H_
#define __MULTI_SHORT_IMAGE_H_
#include "Mat.h"
#include "MultiUcharImage.h"
#include "SingleUcharImage.h"
#include "MultiIntImage.h"
#include "MultiUshortImage.h"
class MultiUcharImage;
class MultIntImage;
class MultiUshortImage;
class MultiShortImage : public CImageData_INT16
{
protected:
public:
	int m_nMAXS; 
	int m_nBit;
	int m_nBLC;
	bool CreateImage(int nWidth, int nHeight, int nCh, int nBit);
	bool CreateImageFillValue(int nWidth, int nHeight, int nDim,int nValue);
	bool CreateImageWithData(int nWidth, int nHeight, int nDim, short *pInputData);
	bool CreateImage(int width, int height, int channel);
	void CopyParameter(MultiShortImage *pInputImage);
	bool Clone(MultiShortImage *pInputImage);
	bool GetRectHistogram(int nHist[], int maxvalue, int nLeft, int nTop, int nRight, int nBottom);
	bool GetEachBlockAverageValue(MultiShortImage *pOutImage, int nRadius);
	bool BGRHToBGR(MultiUcharImage *pOutBGRImage, bool bAddH=true);
	bool BGRHToBGR(MultiShortImage *pOutBGRImage);
	bool BGRHToRGB(MultiUshortImage * pOutRGBImage);
	bool ApplyWeight(MultiUshortImage * pWeightImage, int ScaleBit);
	bool AddImage(MultiShortImage * pRefImage);
	bool SaveSingleChannelToBitmapFile(char *pFileName, int nChannel, int nInScale, int nOutScale, int nOffset);
	bool SaveBGRHToBitmapFile(char *pFileName, int nB, int nG, int nR, int nH);
};
#endif

