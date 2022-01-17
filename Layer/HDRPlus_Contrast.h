#ifndef __HDRPlus_Contrast_H_
#define __HDRPlus_Contrast_H_
#include "../Mat/WeightConfig.h"
#include "../Mat/MultiUshortImage.h"
class CHDRPlus_Contrast : public CSingleConfigTitleFILE
{
protected:
	virtual void InitConfigParamList()
	{
		m_nConfigParamList.ConfigParamListAddVariable("bDumpFileEnable", &m_bDumpFileEnable, 0, 1);
		m_bDumpFileEnable = 0;
		m_nConfigParamList.ConfigParamListAddVariable("nBlacklevel", &m_nBlacklevel, 0, 65535);
		m_nBlacklevel = 2000;
		m_nConfigParamList.ConfigParamListAddVariable("ContrastStrength", &m_ContrastStrength, 0, 65535,16);
		m_ContrastStrength = 5*16;
	}
	virtual void CreateConfigTitleName()
	{
		strcpy(m_pConfigTitleName, "CHDRPlus_Contrast");
	}
public:
	int m_bDumpFileEnable;
	int m_nBlacklevel;
	int m_ContrastStrength;
	CHDRPlus_Contrast()
	{
		Initialize();
	}
	void Forward(MultiUshortImage * pRGBImage);
};

#endif