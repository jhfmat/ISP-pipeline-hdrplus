#ifndef __MULTI_INT_IMAGE_H_
#define __MULTI_INT_IMAGE_H_
#include "Basicdef.h"
#include "Mat.h"
#include "MultiUcharImage.h"
#include "SingleUcharImage.h"
class MultIntImage: public CImageData_INT32
{
public:
	int m_nMAXS;
	int m_nBLC;
	bool Clone(MultIntImage *pInputImage);
	bool CreateImage(int nWidth, int nHeight, int nChannel);
	bool BGRHToBGR(MultiUcharImage *pOutBGRImage);
	bool SaveBGRHToBitmapFile(char *pFileName, int nB, int nG, int nR, int nH);
	bool SaveSingleChannelToBitmapFile(char *pFileName, int nChannel, int nInScale, int nOutScale, int nOffset);
};
#endif

