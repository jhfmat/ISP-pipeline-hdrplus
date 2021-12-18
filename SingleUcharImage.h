#ifndef __SINGLE_UCHAR_IMAGE_H_
#define __SINGLE_UCHAR_IMAGE_H_
#include "Mat.h"
#include "MultiShortImage.h"
#define MatTHRESH_BINARY 0
#define MatTHRESH_BINARY_INV 1
#define MatTHRESH_TRUNC 2
#define MatTHRESH_TOZERO 3
#define MatTHRESH_TOZERO_INV 4
class MultiShortImage;
class SingleUcharImage: public CImageData_UINT8
{
public:
	unsigned char *operator[](int y) { return m_pImgData + y * m_nWidth; }//可以读取可以修改
	const unsigned char *operator[](int y) const { return m_pImgData + y * m_nWidth; }//可以读取不可修改
	//int *  operator [](int m){	return &a[m *col];}
	bool Clone(SingleUcharImage *pInImage);
	bool CreateImage(int nWidth, int nHeight);
	bool CreateImageWithData(int nWidth, int nHeight, unsigned char *pInputData);
	bool CreateImageFillValue(int nWidth, int nHeight,int nValue);
	bool ClearImageFollowRectOutside(int nLeft, int nTop, int nRight, int nBottom);
	bool GetEachBlockAverageValue(SingleUcharImage *pInputImage, SingleUcharImage *pOutImage, int nRadius);
	bool GetHistogram(int nHist[]);
	bool GetHistogram(int nHist[], int nLeft, int nTop, int nRight, int nBottom);
	void HistMapping(int map[256], SingleUcharImage * pOutImage);
	float GetMaskValueRatio(unsigned char cutlabelvalue[],int len);
	bool SobelImage(SingleUcharImage *pOutImage);
	bool PrewittImage(SingleUcharImage *pOutImage);
	bool LaplacianImage(SingleUcharImage *pOutImage);
	bool ThresholdImage(SingleUcharImage *pOutImage, int thresh ,int maxval,int type);
	bool Dilation3x3SquareImage(SingleUcharImage *pOutImage);
	bool Dilation3x3DiamondImage(SingleUcharImage *pOutImage);
	bool Dilation5x5Image(SingleUcharImage *pOutImage);
	bool Dilation7x7Image(SingleUcharImage *pOutImage);
	bool Dilation9x9Image(SingleUcharImage *pOutImage);
	bool Erosion3x3Square(SingleUcharImage *pOutImage);
	bool Erosion3x3Diamond(SingleUcharImage *pOutImage);
	bool Erosion5x5Image(SingleUcharImage *pOutImage);
	bool Erosion7x7Image(SingleUcharImage *pOutImage);
	bool Erosion9x9Image(SingleUcharImage *pOutImage);
	bool BoxFilterImage(SingleUcharImage *pOutImage, int nRadius);
	bool GaussianBlur3x3Image(SingleUcharImage *pOutImage);
	bool GaussianBlur5x5Image(SingleUcharImage *pOutImage);
	bool Bilateral5x5Image(SingleUcharImage * pOutImage, int nThre);
	bool Extend2Image(int nS);
	bool SubtractEdgeImage(SingleUcharImage * pInImage, MultiShortImage * pOutImage);
	bool DownScaleImagex2(SingleUcharImage * pOutImage, bool bDitheringEnable);
	bool UpScaleImagex2(SingleUcharImage * pOutImage, bool bDitheringEnable);
	bool GaussPyramidImage(SingleUcharImage *pOutPyramid, MultiShortImage *pOutEdgePyramid, int &nPyramidLevel);
	bool CombineGaussPyramidAndEdgeImage(MultiShortImage *pInputEdgePyramid, int nPyramidLevel);
	bool ResizeImage(SingleUcharImage *pOutImage,int OutnWidth,int OutnHeight);
	bool AddBackEdgeImage(SingleUcharImage *pInputImage, MultiShortImage *pInputEdgeImage);
	bool SaveGrayToBitmapFile(char *pFileName);
	bool LoadBitmapFileToGray(char *pFileName);
};
#endif

