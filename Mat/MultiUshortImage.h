#ifndef __MULTI_USHORT_IMAGE_H_
#define __MULTI_USHORT_IMAGE_H_
#include "Basicdef.h"
#include "Mat.h"
#include "MultiUcharImage.h"
#include "SingleUcharImage.h"
#include "MultiShortImage.h"
class MultiShortImage;
class MultiUshortImage: public CImageData_UINT16
{
public:
	int m_nRawBits;
	int m_nRawMAXS;
	int m_nRawBLC;
	int m_nRawISOGain;
	int m_nRawAwbGain[3];
	int m_nRawCCM[3][3];
	unsigned short m_nRawCFA;
	MultiUshortImage();
	bool Clone(MultiUshortImage *pInputImage);
	bool CreateImageWithData(int nWidth, int nHeight, int nDim, unsigned short * pInputData);
	bool CreateImage(int width, int height, int bits);
	bool CreateImage(int width, int height, int dim, int bits);
	void CopyParameters(MultiUshortImage *pInputImage);
	bool BGRHToBGR(MultiUshortImage *pOutBGRImage);
	bool GetBGGRMean(double fMean[]);
	bool Extend2Image(MultiUshortImage *pInImage, MultiUshortImage *pOutImage, int nS);
	bool DownScaleImagex2(MultiUshortImage * pOutImage, bool bDitheringEnable);
	bool UpScaleImagex2(MultiUshortImage * pOutImage, bool bDitheringEnable);
	int  GetRAWStride(int width, int nMIPIRAW = 0);
	bool SubtractEdgeImage(MultiUshortImage * pInImage, MultiShortImage * pOutImage);
	bool AddBackEdgeImage(MultiUshortImage * pInputImage, MultiShortImage * pInputEdgeImage);
	bool GaussPyramidImage(MultiUshortImage * pOutPyramid, MultiShortImage * pOutEdgePyramid, int & nPyramidLevel, bool SaveEdge);
	bool ApplyWeight(MultiUshortImage *pWeightImage, int ScaleBit);
	bool FuseDiffImageWeight(MultiUshortImage * pRefImage, MultiUshortImage * pWeightImage, int ScaleBit);
	bool AddImage(MultiUshortImage * pRefImage);
	bool AddImage(MultiShortImage *pRefImage);
	bool NormalizeByOutBlcAndBit(MultiUshortImage *pInputImage, int m_nOutputBLC, int m_nOutputBits);
	void NormalizeBit(int nNewBit);
	bool ConverterRawDataToISPRawData(char *pInputRawData, int nWidth, int nHeight, int nBits, bool bHighBit, bool bByteOrder, int nMIPIRAW = 0);
	bool ConverterISPRawDataToRawData(unsigned char **pOutData, int nBits, bool bHighBit, bool bByteOrder, int &nLen, int nMIPIRAW = 0);
	bool Load16BitRawDataFromBinFile(char *pFileName, int nWidth, int nHeight, int nBits, bool bHighBit, bool bByteOrder, int nMIPIRAW=0);
	bool Load16BitRawDataFromISPRawData(unsigned short *pInputData, int nWidth, int nHeight, int nBits, bool bHighBit, bool bByteOrder, int nMIPIRAW);	
	bool Save16BitRawToBinFile(char *pFileName, int nBits, bool bHighBit, bool bByteOrder, int nMIPIRAW = 0);
	bool Save16BitRawToBitmapFile(char *pFileName, bool bBlackWhite = true);
	bool MeanImage5x5();
	bool MeanImage3x3();
	bool GetSingleChannelImage(MultiUshortImage * pOutImage,int ch);
	bool ReplaceSingleChannelImage(MultiUshortImage * pInputImage,int ch);
	bool Bilateral5x5SingleImage(int nThre);
	void GetMultiImageIntegralUSData(CImageData_UINT32 * Integral, int Width, int Height, int dim);
	bool GetEachBlockAverageValue(int nRadius);
	bool SaveSingleChannelToBitmapFile(char * pFileName, int nChannel, int nInScale, int nOutScale, int nOffset);
};
#endif