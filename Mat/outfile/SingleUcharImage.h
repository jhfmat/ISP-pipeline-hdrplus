#ifndef __SINGLE_UCHAR_IMAGE_H_
#define __SINGLE_UCHAR_IMAGE_H_
#include "Mat.h"
#include "MultiShortImage.h"
#include "MultiIntImage.h"
#define MatTHRESH_BINARY 0
#define MatTHRESH_BINARY_INV 1
#define MatTHRESH_TRUNC 2
#define MatTHRESH_TOZERO 3
#define MatTHRESH_TOZERO_INV 4
class MultiShortImage;
class MultIntImage;
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
	bool GetEachBlockAverageValue(SingleUcharImage *pOutImage, int nRadius);
	bool GetHistogram(int nHist[]);
	bool GetRectHistogram(unsigned int nHist[], unsigned char maxvalue, int nLeft, int nTop, int nRight, int nBottom);
	void HistMapping(int map[256], SingleUcharImage * pOutImage);
	bool GetIntegralHistogram(CImageData_UINT32 * pOutIntegralHist);
	float GetMaskValueRatio(unsigned char cutlabelvalue[],int len);
	bool FillImageRect(int nLeft, int nTop, int nRight, int nBottom, unsigned char nValue);
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
	bool ExtendRightAndBottomImage(int nOutWidth, int nOutHeight);
	bool SubtractEdgeImage(SingleUcharImage * pInImage, MultiShortImage * pOutImage);
	bool DownScaleImagex2(SingleUcharImage * pOutImage, bool bDitheringEnable);
	bool DownScaleImageSamplex2(SingleUcharImage *pOutImage);
	bool UpScaleImagex2(SingleUcharImage * pOutImage, bool bDitheringEnable);
	bool GaussPyramidImage(SingleUcharImage *pOutPyramid, MultiShortImage *pOutEdgePyramid, int &nPyramidLevel);
	bool GetSamplePyramid(SingleUcharImage * pOutPyramid, int nPyramidLevel);
	bool CombineGaussPyramidAndEdgeImage(MultiShortImage *pInputEdgePyramid, int nPyramidLevel);
	bool NonMaximumImage(CImage_FLOAT * pImage, TFeaturePoint * pPtList, int & nPtNum, int nMaxNum);
	bool ComputeCornernessImage(CImage_FLOAT * pOutImage, float fHarris_K, int nHarrisThre, int nRatio, int nPtThre1, int nPtThre2, int nThre);
	bool DetectFeaturePoint(TFeaturePoint * pPtList, int & nPtNum, int nMaxNum, float fHarris_K, int nHarrisThre, int nRatio, int nPtThre1, int nPtThre2, int nThre);
	bool UpdateFeaturePoint(TFeaturePoint * pPtList, int nPtNum, float fHarris_K);
	bool MarkFeaturePoint(char * pFileName, TFeaturePoint pPtList[], int nNum, int nS, int nC);
	bool ResizeCubicImage(SingleUcharImage *pOutImage,int OutnWidth,int OutnHeight);
	bool ResizeNearestNeighborImage(SingleUcharImage * pOutImage, int OutnWidth, int OutnHeight);
	bool AddBackEdgeImage(SingleUcharImage *pInputImage, MultiShortImage *pInputEdgeImage);
	bool SaveGrayToBitmapFile(char *pFileName);
	bool LoadBitmapFileToGray(char *pFileName);
};
#endif

