#ifndef __HDRPlus_Sharpen_H_
#define __HDRPlus_Sharpen_H_
#include "../Mat/WeightConfig.h"
#include "../Mat/MultiUshortImage.h"
#include "HDRPlus_ChromaDenoise.h"
class CHDRPlus_Sharpen : public CSingleConfigTitleFILE
{
protected:
	virtual void InitConfigParamList()
	{
		m_nConfigParamList.ConfigParamListAddVariable("bDumpFileEnable", &m_bDumpFileEnable, 0, 1);
		m_bDumpFileEnable = 0;
		m_nConfigParamList.ConfigParamListAddVariable("nSharpenStrength", &m_nSharpenStrength, 0, 65535,16);
		m_nSharpenStrength = 16*2;
	}
	virtual void CreateConfigTitleName()
	{
		strcpy(m_pConfigTitleName, "CHDRPlus_Sharpen");
	}
public:
	int m_bDumpFileEnable;
	int m_nSharpenStrength;
	CHDRPlus_Sharpen()
	{
		Initialize();
	}
	bool RGBToYUV(MultiUshortImage * pRGBImage, MultiUshortImage * YImage, MultiUshortImage * UImage, MultiUshortImage * VImage);
	void Forward(MultiUshortImage * pRGBImage);
};

#endif