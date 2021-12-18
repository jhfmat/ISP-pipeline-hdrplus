#ifndef __MULTI_UCHAR_IMAGE_H_
#define __MULTI_UCHAR_IMAGE_H_
#include "Mat.h"
#include "SingleUcharImage.h"
#include "MultiShortImage.h"
class SingleUcharImage;
class MultiShortImage;
class MultiUcharImage : public CImageData_UINT8
{
public:
	bool CreateImage(int nWidth, int nHeight);
	bool CreateImageWithData(int nWidth, int nHeight, unsigned char *pInputData);
	bool CreateImageFillValue(int nWidth, int nHeight, int nValue);
	bool Clone(MultiUcharImage *pInputImage);
	bool RepImageWithRect(MultiUcharImage *Rectimg, int nLeft, int nTop, int nRight, int nBottom);
	bool CopyExtendRect(MultiUcharImage *pInputImage, int nLeft, int nTop, int nRight, int nBottom, int nOutWidth, int nOutHeight);
	bool ClearImageFollowMask(unsigned char *pInMaskData, int nx,int ny);
	bool ClearImageFollowMaskThreshold(unsigned char *pInMaskData, int nx, int ny, int thre);
	bool ClearImageFollowMaskUpDownThre(unsigned char *pInMaskData, int nx, int ny, int up, int down, int thre);
	bool GetMeanYVal(float &value);
	bool ClearImageFollowRectOutside(int nLeft, int nTop, int  nRight, int nBottom);
	void YCbCr2BGR();
	void YCbCr2BGROPT();
	void YCbCr2RGB();
	void RGB2YCbCr();
	void BGR2YCbCrOPT();
	void BGR2YCbCr();
	void RGB2YCbCrOPT();
	bool RGB2Y(SingleUcharImage *pOutY);
	float GetRGBtoGrayMeanBrightness();
	bool GetHistogram(int pHistB[], int pHistG[], int pHistR[]);
	void HistMapping(int map[3][256], MultiUcharImage * pOutImage);
	bool GetSingleChannelImage(SingleUcharImage *pOutImage);
	bool GetAllChannelImage(SingleUcharImage *pOutImage);
	bool ReplaceSingleChannelImage(SingleUcharImage *pInputImage);
	bool ReplaceAllChannelImage(SingleUcharImage *pInputImage);
	bool BoxFilterImage(MultiUcharImage *pOutImage, int nRadius);
	bool GaussianBlur3x3Image(MultiUcharImage *pOutImage);
	bool GaussianBlur5x5Image(MultiUcharImage *pOutImage);
	bool Bilateral5x5Image(MultiUcharImage * pOutImage, int nThre);
	bool Extend2Image(int nS);
	bool SubtractEdgeImage(MultiUcharImage * pInImage, MultiShortImage * pOutImage);
	bool DownScaleImagex2(MultiUcharImage * pOutImage, bool bDitheringEnable);
	bool UpScaleImagex2(MultiUcharImage * pOutImage, bool bDitheringEnable);
	bool AddBackEdgeImage(MultiUcharImage * pInputImage, MultiShortImage * pInputEdgeImage);
	bool GaussPyramidImage(MultiUcharImage * pOutPyramid, MultiShortImage * pOutEdgePyramid, int & nPyramidLevel);
	bool ResizeImage(MultiUcharImage *pOutImage, int OutnWidth, int OutnHeight);
	bool SaveBGRToBitmapFile(char *pFileName, bool bVFilp = true);
	bool SaveRGBToBitmapFile(char *pFileName, bool bVFilp = true);
	bool LoadBitmapFileToBGR(char *pFileName);
	bool LoadBitmapFileToRGB(char *pFileName);
	bool SaveRGBToJpegFile(char *pFileName, int Quality = 100);
	bool SaveBGRToJpegFile(char *pFileName, int Quality = 100);
};
#endif

