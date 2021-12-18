#ifndef __MULTI_SHORT_IMAGE_H_
#define __MULTI_SHORT_IMAGE_H_
#include "Mat.h"
#include "MultiUcharImage.h"
#include "SingleUcharImage.h"
class MultiUcharImage;
class MultiShortImage : public CImageData_INT16
{
protected:
public:
	int m_nMAXS; 
	int m_nBit;
	int m_nBLC;
	bool CreateImage(int nWidth, int nHeight, int nCh, int nBit);
	bool CreateImage(int width, int height, int channel);
	void CopyParameter(MultiShortImage *pInputImage);
	bool Clone(MultiShortImage *pInputImage);
	bool BGRHToBGR(MultiUcharImage *pOutBGRImage, bool bAddH=true);
	bool BGRHToBGR(MultiShortImage *pOutBGRImage);
	bool SaveSingleChannelToBitmapFile(char *pFileName, int nChannel, int nInScale, int nOutScale, int nOffset);
	bool SaveBGRHToBitmapFile(char *pFileName, int nB, int nG, int nR, int nH);
};
#endif

