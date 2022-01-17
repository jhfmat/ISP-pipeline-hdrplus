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
	bool GetMeanRGBVal(float &Rvalue, float &Gvalue, float &Bvalue);
	bool DivideAWBGain(float Rgain, float Ggain, float Bgain);
	bool ApplyAWBGain(float Rgain, float Ggain, float Bgain);
	bool ClearImageFollowRectOutside(int nLeft, int nTop, int  nRight, int nBottom);
	void YCbCr2BGR();
	void YCbCr2BGROPT();
	void YCbCr2RGB();
	void RGB2YCbCr();
	void BGR2YCbCrOPT();
	void BGR2YCbCr();
	void RGB2YCbCrOPT();
	bool RGB2Y(SingleUcharImage *pOutYImage);
	bool RGBToHue(MultiShortImage *pOutHueImage);
	float GetRGBtoGrayMeanBrightness();
	bool GetHistogram(unsigned int pHistB[],unsigned int pHistG[], unsigned int pHistR[],unsigned char nMaxValue);
	bool GetRectHistogram(unsigned int *nHistR, unsigned int *nHistG, unsigned int *nHistB, unsigned int maxvalue, int nLeft, int nTop, int nRight, int nBottom);
	void HistMapping(int map[3][256], MultiUcharImage * pOutImage);
	float GetMeanHue();
	float GetMeanHSV_V();
	bool GetAverageRGB(double & R, double & G, double & B);
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
	bool ResizeCubicImage(MultiUcharImage *pOutImage, int OutnWidth, int OutnHeight);
	bool SaveBGRToBitmapFile(char *pFileName, bool bVFilp = true);
	bool SaveRGBToBitmapFile(char *pFileName, bool bVFilp = true);
	bool LoadBitmapFileToBGR(char *pFileName);
	bool LoadBitmapFileToRGB(char *pFileName);
	bool SaveRGBToJpegFile(char *pFileName, int Quality = 100);
	bool SaveBGRToJpegFile(char *pFileName, int Quality = 100);
};
#endif

