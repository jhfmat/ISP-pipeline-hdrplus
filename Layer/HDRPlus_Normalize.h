#ifndef __HDRPlus_Normalize_H_
#define __HDRPlus_Normalize_H_
#include "../Mat/WeightConfig.h"
#include "../Mat/MultiUshortImage.h"
class CHDRPlus_Normalize : public CSingleConfigTitleFILE
{
protected:
	virtual void InitConfigParamList()
	{
		m_nConfigParamList.ConfigParamListAddVariable("bDumpFileEnable", &m_bDumpFileEnable, 0, 1);
		m_bDumpFileEnable = 1;
		m_nConfigParamList.ConfigParamListAddVariable("nOutBit", &m_nOutBit, 0, 65535);
		m_nOutBit =8;
	}
	virtual void CreateConfigTitleName()
	{
		strcpy(m_pConfigTitleName, "CHDRPlus_Normalize");
	}
public:
	int m_bDumpFileEnable;
	int m_nOutBit;
	CHDRPlus_Normalize()
	{
		Initialize();
	}
	bool Forward(MultiUshortImage * pInRGBImage, MultiUcharImage * pOutRGBImage);
};

#endif