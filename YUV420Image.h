#ifndef __YUV420_IMAGE_H_
#define __YUV420_IMAGE_H_
#include "MultiUcharImage.h"
class MultiUcharImage;//相互包含要申明
class Yuv420Image : public CImageData_UINT8
{
protected:
	int m_nWidth;
	int m_nHeight;
	unsigned char *m_pYImage;
	unsigned char *m_pUVImage;
	void HIntUVLine(unsigned char *pInLine, unsigned short *pOutLine, int nWidth);
public:
	inline unsigned char *GetYLine(int nY) { return m_pYImage + nY * m_nWidth; }
	inline unsigned char *GetUVLine(int nY) { return m_pUVImage + (nY >> 1)*m_nWidth; }
	unsigned char *GetYData() { return m_pYImage; }
	unsigned char *GetUVData() { return m_pUVImage; }
	bool CreateImage(int nWidth, int nHeight);
	bool CreateImageWithData(int nWidth, int nHeight, unsigned char *pInputData);
	bool Clone(Yuv420Image *pInImage);
	int GetImageWidth(){return m_nWidth;}
	int GetImageHeight(){return m_nHeight;}
	bool YUV444ToYUV420(MultiUcharImage *pInputYUV444Image);
	bool YUV444ToYUV420OPT(MultiUcharImage *pInputYUV444Image);
	bool YUV420ToYUV444(MultiUcharImage *pOutYUV444Image, bool bSimple=false);
	bool GetYImage(SingleUcharImage *pOutYImage);
	bool GetUImage(SingleUcharImage *pOutUImage);
	bool GetVImage(SingleUcharImage *pOutVImage);
	bool CombineYCbCr(SingleUcharImage *pInputYImage, MultiUcharImage *pOutYCbCr);
	bool SaveYuvToBitmapFile(char *pFileName);
	bool SaveYToBitmapFile(char *pFileName);
	bool SaveYuvToJpegFile(char *pFileName, int Quality=100);
};
#endif