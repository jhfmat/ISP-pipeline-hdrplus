#ifndef __CHDRPlus_ChromaDenoise_H_
#define __CHDRPlus_ChromaDenoise_H_
#include "../Mat/WeightConfig.h"
#include "../Mat/MultiUshortImage.h"
class CHDRPlus_ChromaDenoise : public CSingleConfigTitleFILE
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
		m_nConfigParamList.ConfigParamListAddArray("nGainList", m_nGainList, 3, 16, 8192, 16);
		m_nGainList[0] = 32;
		m_nGainList[1] = 64;
		m_nGainList[2] = 128;
		m_nConfigParamList.ConfigParamListAddArray("nAmountGainList", m_nAmountGainList, 3, 0, 4096,16);
		m_nAmountGainList[0] = 16;
		m_nAmountGainList[1] = 32;
		m_nAmountGainList[2] = 48;
		m_nConfigParamList.ConfigParamListAddVariable("nYnoiseBilateralThre", &m_nYnoiseBilateralThre, 0, 65535);
		m_nYnoiseBilateralThre = 256;
		m_nConfigParamList.ConfigParamListAddVariable("nDenoiseTimes", &m_nDenoiseTimes, 0, 65535);
		m_nDenoiseTimes = 3;
		m_nConfigParamList.ConfigParamListAddVariable("nRatioThre", &m_nRatioThre, 0, 65535,16);
		m_nRatioThre = 16*1.4;
		m_nConfigParamList.ConfigParamListAddVariable("nThreshold", &m_nThreshold, 0, 65535);
		m_nThreshold = 25000;
		m_nConfigParamList.ConfigParamListAddVariable("nAddSaturation", &m_nAddSaturation, 0, 65535,16);
		m_nAddSaturation = 16*1.1;
	}
	virtual void CreateConfigTitleName()
	{
		strcpy(m_pConfigTitleName, "CHDRPlus_ChromaDenoise");
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
	int m_nYnoiseBilateralThre;
	int m_nDenoiseTimes;
	int m_nRatioThre;
	int m_nThreshold;
	int m_nAddSaturation;
	CHDRPlus_ChromaDenoise()
	{
		Initialize();
	}
	bool RGBToYUV(MultiUshortImage * pRGBImage, MultiUshortImage * YImage, MultiUshortImage * UImage, MultiUshortImage * VImage);
	bool YUVToRGB(MultiUshortImage * YImage, MultiUshortImage * UImage, MultiUshortImage * VImage, MultiUshortImage * pRGBImage);
	bool DesaturateNoise(MultiUshortImage * pUVImage);
	void IncreaseSaturation(MultiUshortImage * pUVImage, float len);
	bool Forward(MultiUshortImage * pRGBImage,TGlobalControl *pControl);
};

#endif