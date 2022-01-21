#ifndef __HDRPlus_Tonemapping_H_
#define __HDRPlus_Tonemapping_H_
#include "../Mat/WeightConfig.h"
#include "../Mat/MultiUshortImage.h"
class CHDRPlus_Tonemapping : public CSingleConfigTitleFILE
{
protected:
	virtual void InitConfigParamList()
	{
		m_nConfigParamList.ConfigParamListAddVariable("bDumpFileEnable", &m_bDumpFileEnable, 0, 1);
		m_bDumpFileEnable = 1;
		m_nConfigParamList.ConfigParamListAddVariable("nManualAmount", &m_nManualAmount, 1, 100000, 16);
		m_nManualAmount = 16;
		m_nConfigParamList.ConfigParamListAddVariable("bAutoAmount", &m_bAutoAmount, 0, 1);
		m_bAutoAmount = 1;
		m_nConfigParamList.ConfigParamListAddVariable("nGainOption", &m_nGainOption, 0, 2);
		m_nGainOption = 0;
		m_nConfigParamList.ConfigParamListAddArray("nGainList", m_nGainList, 3, 16, 8192, 128);
		m_nGainList[0] = 256;
		m_nGainList[1] = 512;
		m_nGainList[2] = 2048;
		m_nConfigParamList.ConfigParamListAddArray("nAmountGainList", m_nAmountGainList, 3, 0, 4096,16);
		m_nAmountGainList[0] = 16;
		m_nAmountGainList[1] = 32;
		m_nAmountGainList[2] = 48;
		m_nConfigParamList.ConfigParamListAddVariable("nTargetY", &m_nTargetY, 0, 65535);
		m_nTargetY = 16384;
		m_nConfigParamList.ConfigParamListAddVariable("bAutoDigiGainEnable", &m_bAutoDigiGainEnable, 0, 1);
		m_bAutoDigiGainEnable = 1;
		m_nConfigParamList.ConfigParamListAddVariable("bKeepHighLightEnable", &m_bKeepHighLightEnable, 0, 1);
		m_bKeepHighLightEnable = 1;
		m_nConfigParamList.ConfigParamListAddVariable("nMaxDigiGain", &m_nMaxDigiGain, 0, 65535,128);
		m_nMaxDigiGain = 16 * 128;
		m_nConfigParamList.ConfigParamListAddVariable("nMaxTotalGain", &m_nMaxTotalGain, 1, 65536,128);
		m_nMaxTotalGain = 2048;
		m_nConfigParamList.ConfigParamListAddMatrixRow("nBlockWeightMap", m_nBlockWeightMap[0], 0, 4, 0, 16);
		m_nConfigParamList.ConfigParamListAddMatrixRow("nBlockWeightMap", m_nBlockWeightMap[1], 1, 4, 0, 16);
		m_nConfigParamList.ConfigParamListAddMatrixRow("nBlockWeightMap", m_nBlockWeightMap[2], 2, 4, 0, 16);
		m_nConfigParamList.ConfigParamListAddMatrixRow("nBlockWeightMap", m_nBlockWeightMap[3], 3, 4, 0, 16);
		for (int i = 0; i < 4; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				m_nBlockWeightMap[i][j] = 1;
			}
		}
		m_nConfigParamList.ConfigParamListAddArray("nHighLevelPtsPercent", m_nHighLevelPtsPercent, 2, 0, 65536);
		m_nHighLevelPtsPercent[0] = 16;
		m_nHighLevelPtsPercent[1] = 16;
		m_nConfigParamList.ConfigParamListAddVariable("nVirtualExposureNum", &m_nVirtualExposureNum, 0, 65535);
		m_nVirtualExposureNum = 3;
		m_nConfigParamList.ConfigParamListAddVariable("nDynamicCompression", &m_nDynamicCompression, 0, 65535,128);
		m_nDynamicCompression = 5.8*128;
		m_nConfigParamList.ConfigParamListAddArray("nSmoothYThreP", m_nSmoothYThreP, 13, 1, 4096);
		m_nConfigParamList.ConfigParamListAddArray("nSmoothYThreM", m_nSmoothYThreM, 13, 1, 4096);
		for (int i = 0; i < 13; i++)
		{
			m_nSmoothYThreP[i] = 2048;
			m_nSmoothYThreM[i] = 2048;
		}
	}
	virtual void CreateConfigTitleName()
	{
		strcpy(m_pConfigTitleName, "CHDRPlus_Tonemapping");
	}
public:
	int m_bDumpFileEnable;
	int m_nMin;
	int m_nMax;
	int m_nManualAmount;
	int m_bAutoAmount;
	int m_nGainOption;
	int m_nGainList[3];
	int m_nAmountGainList[3];
	int m_nTargetY;
	int m_bAutoDigiGainEnable;
	int m_bKeepHighLightEnable;
	int m_nMaxDigiGain;
	int m_nMaxTotalGain;
	int m_nBlockWeightMap[4][4];
	int m_nHighLevelPtsPercent[2];
	int m_nVirtualExposureNum;
	int m_nDynamicCompression;
	int m_nSmoothYThreP[13];
	int m_nSmoothYThreM[13];
	CHDRPlus_Tonemapping()
	{
		Initialize();
	}
	bool ConvertoGray(MultiUshortImage * pRGBImage, MultiUshortImage * pGrayImage);
	bool Brighten(MultiUshortImage * pInDarkImage, float gain, MultiUshortImage * pOutBrightImage);
	bool GrayGammaCorrect(MultiUshortImage * pInGrayImage, MultiUshortImage * pOutGammaImage);
	bool GammaInverse(MultiUshortImage * pInGrayImage, MultiUshortImage * pOutInverseImage);
	bool BuildWeight(MultiUshortImage * pDarkGammaImage, MultiUshortImage * pBrightGammaImage, MultiUshortImage * DarkWeightImage, MultiUshortImage * BrightWeightImage, int ScaleBit);
	bool CombineDarkAndBrightImage(MultiUshortImage * pDarkGammaImage, MultiUshortImage * pBrightGammaImage, MultiUshortImage * pOutCombineImage);
	void GammaCombinRGB(MultiUshortImage * pRGBImage, MultiUshortImage * pGrayImage, MultiUshortImage * pDarkImage);
	bool BilateralSmoothYImagenew(MultiUshortImage * pInImage, MultiUshortImage * pOutImage, int nThreP, int nThreM, int nMaskThreP, int nMaskThreM);
	bool SmoothGammaYImage(MultiUshortImage * pInImage, MultiUshortImage * pOutImage);
	bool EstimateDigiGain(MultiUshortImage * pInImage, TGlobalControl * pControl);
	bool Forward(MultiUshortImage * pInRGBImage,TGlobalControl *pControl);
};

#endif