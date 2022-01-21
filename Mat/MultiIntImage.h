#ifndef __MULTI_INT_IMAGE_H_
#define __MULTI_INT_IMAGE_H_
#include "Basicdef.h"
#include "Mat.h"
#include "MultiUcharImage.h"
#include "SingleUcharImage.h"
#include "MultiUshortImage.h"
class MultiUcharImage;
class MultiUshortImage;
class MultIntImage: public CImageData_INT32
{
public:
	int m_nMAXS;
	int m_nBLC;
	bool Clone(MultIntImage *pInputImage);
	bool CreateImage(int nWidth, int nHeight, int nChannel);
	bool CreateImageWithData(int nWidth, int nHeight, int nChannel, int * pInputData);
	bool CreateImageFillValue(int nWidth, int nHeight, int nValue);
	bool BGRHToBGR(MultiUcharImage *pOutBGRImage);
	bool BGRHToBGR(MultiUshortImage * pOutBGRImage);
	bool GaussianBlur7x7Image(MultIntImage *pOutImage);
	bool SaveBGRHToBitmapFile(char *pFileName, int nB, int nG, int nR, int nH);
	bool SaveSingleChannelToBitmapFile(char *pFileName, int nChannel, int nInScale, int nOutScale, int nOffset);
};
#endif

